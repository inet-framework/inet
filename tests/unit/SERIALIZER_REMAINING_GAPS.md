# Serializer round-trip: remaining gaps

Status of the `serializer_pcap` / `serializer_chunk_roundtrip` work on
`topic/bz/serializertest`.

Both tests exercise the **real `serialize()`** path: a `FieldsChunk` built by
deserialization used to cache the original wire bytes and the serializer replayed that
cache instead of re-encoding from the parsed fields, which hid every asymmetry. With the
cache cleared before re-serialization, the pcap corpus went from **1219 differing frames
to 30** (plus 41 error frames). This document lists what is left, why, the fix, and its
size.

Current `serializer_pcap`: **4107 same / 22 differ / 31 error** (4160 frames), exercising
**114 of 214** registered serializers (56 not pcap-testable, 44 with no capture). The
[remaining differs/errors](#summary-table) are below; the serializers that **no capture
exercises** — and why — are in
[Untested serializers](#untested-serializers-registered-but-no-real-capture-coverage).

Legend for **Effort**: **M** = a small model addition or a bounded serializer rework (a
few hours). **L** = a new list/TLV model or a message-family rework (a day+). **—** =
impossible (encrypted payload).

---

## Summary table

| Protocol | Frames | Kind | Root cause | Fix | Effort |
|---|---|---|---|---|---|
| MIPv6 (BU/BA) | 9 diff | gap | trailing mobility-**options** not modelled | generic option-TLV list | **L** |
| SCTP | 9 diff | gap | on-wire parameter **order** not preserved (fixed-field model, not a TLV list) | ordered-parameter list | **L** |
| DHCP | 4 diff | gap | `DhcpOptions` is a fixed struct, not a TLV list | option TLV-list rewrite | **L** |
| IEEE 802.11 | 24 err | gap | QoS-Data-carried EAPOL + A-MPDU subframe dissection + non-standard BlockAckReq layout | dissector/parse fixes | **M–L** |
| IPsec ESP | 6 err | not fixable | ESP **trailer is inside the ciphertext** | (needs decryption) | **—** |

The 22 differing frames are all in the MIPv6 / SCTP / DHCP captures; the 31 error frames
break down as **6 ESP + 24 IEEE 802.11 + 1 MIPv6** (the MIPv6 capture also throws one
"incorrect chunk" on a Binding Update alongside its nine content differs — same
mobility-options gap).

---

## MIPv6 — Binding Update / Binding Acknowledgement mobility options (9 frames, differ)

**Cause.** A Mobility Header ends with a list of *mobility options* (TLVs: Pad1, PadN,
Binding Refresh Advice, Alternate Care-of Address, Nonce Indices, Binding Authorization
Data, …). `MobilityHeaderSerializer` writes only the fixed part of each message and does
not emit the trailing options, so a BU/BA that carries any option loses the option bytes.

**Fix.** Add a generic option list to the Mobility Header model — an `option[]` with a
base `type/length/value` and either concrete subclasses for the modelled options or a raw
catch-all (the pattern already used for `BgpUpdatePathAttributesUnknown` and the SCTP
unrecognized parameters). Deserialize walks the options to the end of the header; serialize
re-emits them.

**Effort: L.** New message classes + option loop in both directions. Fingerprint-touching
(the MIPv6 sims carry BUs), so a re-record of the recurring MIPv6 `~tNlb` rows.

---

## SCTP — INIT parameter order (9 frames, differ)

The serializer already **preserves every parameter** (an earlier fix stopped it dropping
the ECN-Capable / unrecognized parameters and fixed the chunk-length accounting). What
remains is order: INET models the INIT parameters as flags/fields (`ipv4Supported`,
`forwardTsn`, `addresses[]`, `sepChunks[]`, `unrecognizedParameters[]`, …) and re-emits
them in a **fixed order** (supported-address, forward-TSN, addresses, …, then the preserved
unknowns). On the wire the parameters are an **ordered TLV list**, so a capture with a
different order (and, e.g., a Supported-Address-Types parameter listing one address type
instead of INET's two) re-serializes to the same bytes in a different arrangement — which
also changes the SCTP checksum, so the frames differ starting at the checksum octet.

**Fix.** Model the INIT parameters as an ordered TLV list (a `parameter[]` with typed
subclasses + a raw catch-all), the same pattern as the MIPv6 mobility options and BGP path
attributes. The SCTP module reads the convenience fields, so those must stay — hence the
list is additional state, which makes this a real rework.

**Effort: L.**

---

## DHCP — options (4 frames, differ)

**Cause.** On the wire, DHCP options are an ordered list of arbitrary `(code, len, value)`
TLVs. INET's `DhcpOptions` is a **fixed struct of known options**, so it cannot preserve
option order, cannot represent unknown option codes (e.g. Vendor Class 60), and cannot
hold non-type-1 Client Identifiers. Round-tripping a real capture therefore reorders /
drops options. Already documented as a TODO on `DhcpMessage`/`DhcpOptions`.

**Fix.** Replace the fixed `DhcpOptions` struct with an ordered TLV list and rewrite the
serializer to walk it. Ripples into the DHCP client/server modules that read the struct
fields.

**Effort: L.** Fingerprint impact on the `/examples/dhcp/` sims (they build options).

---

## IEEE 802.11 — QoS-Data EAPOL / A-MPDU / block-ack (24 error)

Three separate 802.11 dissection gaps, none in the management/data-header serializers
(those round-trip — see the mgmt and data-subtype arcs) but in the layers *above/around*
the MAC header:

**QoS-Data-carried EAPOL — 14 frames (`wlan-blockack-reassoc.pcap`).** These are QoS-Data
frames whose payload is an 802.1X/EAPOL exchange in an LLC/SNAP header
(`aa aa 03 00 00 00 88 8e …`). The MAC header round-trips, but dissecting the LLC/EAPOL
body reads past the frame — `ieee80211mac … offset is out of range` at byte 26. **Fix:**
correct the QoS-Data → LLC/SNAP → EAPOL descent (the payload length / QoS-control offset
accounting). **Effort M.**

**A-MPDU aggregation — 9 frames (`wlan-aggregation.pcap`).** Aggregated-MPDU subframes
deserialize to an *incorrect / incomplete* chunk: the A-MPDU subframe delimiter/padding
walk is not modelled (only A-MSDU is, via `Ieee80211MsduSubframeHeader`). `Ieee80211Mpdu`​`SubframeHeader`
is registered but no A-MPDU capture exercised it before this corpus. **Fix:** model the
A-MPDU subframe framing in the MAC dissector. **Effort M–L.**

**Block-ack / BlockAckReq — 1 frame (`wlan-addba-delba.pcap` / `wlan-blockack-basic.pcap`).**
An incomplete chunk on a BlockAckReq: INET models the BAR in a non-standard wide layout
(38 B: uint32 fragment + uint64 SSN) instead of the 20-byte wire format, so a conformant
BAR under-runs — the same root cause listed for `Ieee80211BasicBlockAckReq` /
`Ieee80211CompressedBlockAckReq` in the [untested table](#no-real-capture-but-round-tripped-constructively-41).
**Effort M** (the 20-byte BAR rewrite).

---

## IPsec ESP — not round-trippable from a capture (6 error)

**Cause.** `esp.pcap` frames reach `inet::ipsec::IPsecEspHeader`, for which no serializer
is registered, so the IPv4 dissector throws "Cannot find serializer" — but that is only
the first symptom. The real blocker is deeper: an ESP packet is
`SPI + Sequence + [ Payload | Padding | Pad Length | Next Header ]_encrypted + ICV`
(RFC 4303), so the **trailer (Pad Length, Next Header) and padding are inside the encrypted
region**. `IpSecEspProtocolDissector` pops that trailer to find the next protocol and the
padding length — which only works when the content is *decrypted* (INET-internal packets).
For a capture the content is ciphertext, so the "pad length" is a random encrypted byte and
`popAtBack(padLength)` fails with "length is invalid".

**Investigated (reverted):** writing `IPsecEspHeaderSerializer`, `IPsecEspTrailerSerializer`
and making `EncryptedChunk` peekable/serializable from raw bytes (a `convertChunk` +
`EncryptedChunkSerializer`, mirroring `SliceChunk`) got past the header, but the dissector
still parses the trailer out of the ciphertext, so ~4 of 6 frames throw and the rest only
"pass" by accident (a small random pad length). Making it work would require the ESP
dissector to treat the whole encrypted region as opaque when it cannot decrypt — a
dissector redesign with semantic implications, for 6 frames whose payload is unrecoverable
anyway.

**Effort: —.** Not achievable without the decryption keys.

---

## Untested serializers (registered, but no real-capture coverage)

The two tests cover different things. `serializer_chunk_roundtrip` **constructs** an
instance of every registered chunk type and checks `serialize → deserialize → serialize`;
its serializer-class gate is `0 unexpected`, i.e. **every registered serializer class is
either exercised there or explicitly allow-listed**. `serializer_pcap` instead replays a
corpus of real Wireshark captures, so it only exercises serializers a capture actually
contains.

Of the **214 registered serializers**, the pcap run reports **114 exercised**, **56
skipped** (not pcap-testable, see below) and **44 registered types that no capture
invokes**. Cross-referencing those 44 against the constructed test:

* **41 are still tested constructively** by `serializer_chunk_roundtrip` — they merely
  lack a *real-wire* exercise (no capture in the corpus carries them);
* **3 are exercised by neither** as a standalone chunk (next subsection).

"Untested" below therefore means **"no real capture exercises it"**, not "no test at all"
(except the 3 explicitly flagged).

### No real capture, but round-tripped constructively (41)

| Family | Count | Serializers | Why no capture exercises it | To light up (effort) |
|---|---|---|---|---|
| **MRP** (Media Redundancy, 0x88E3) | 14 | MrpTest, MrpTopologyChange, MrpLinkChange, MrpInTest/InTopologyChange/InLinkChange/InLinkStatusPoll, MrpCommon, MrpEnd, MrpOption, MrpManufacturerFkt, MrpVersion, MrpSubTlvHeader, MrpSubTlvTest | No public MRP capture exists, and the 0x88E3 EtherType is not dissected in the corpus path | obtain/synthesize an MRP capture + wire the 0x88E3 dissector (M) |
| **RTP / RTCP** | 6 | RtpHeader, RtpMpegHeader, RtcpSenderReportPacket, RtcpReceiverReportPacket, RtcpSdesPacket, RtcpByePacket | No RTP/RTCP dissector, and RTP uses **dynamically negotiated** (SIP/SDP-signalled) UDP ports, so the sip_rtp_rtcp media parses as raw UDP payload | add RTP+RTCP dissectors + dynamic-port routing (M) |
| **IPv6 extension headers** | 3 | Ipv6AuthenticationHeader (AH), Ipv6DestinationOptionsHeader, Ipv6EncapsulatingSecurityPayloadHeader (ESP) | The AH / DestOpts captures were dropped from the corpus (malformed / model mismatch); the ESP header chunk round-trips, but a real ESP-protected packet cannot (payload+trailer encrypted — see the ESP section) | add clean AH/DestOpts captures (S–M); ESP is impossible |
| **MLD queries** | 3 | MldQuery, Mldv2Query, Mldv2Report | mld.pcap carries only MLDv1 Report + Done; no capture holds an MLD(v1/v2) Query or MLDv2 Report | a capture containing MLD queries (S) |
| **IEEE 802.11 BA / A-MPDU** | 3 | Ieee80211BasicBlockAckReq, Ieee80211CompressedBlockAckReq, Ieee80211MpduSubframeHeader | INET models the BlockAckReq in a **non-standard wide layout** (38 B: uint32 fragment + uint64 SSN), so a conformant 20-byte on-wire BAR never deserializes into it; and no A-MPDU-subframe capture is in the corpus (only A-MSDU) | rewrite Basic/Compressed BAR to the 20-byte wire format (M); add an A-MPDU capture |
| **Ethernet control / sub-fields** | 4 | EthernetPauseFrame, EthernetMacAddressFields, EthernetTypeOrLengthField, EthernetFragmentFcs | No 802.3x PAUSE-frame capture; the address / type-or-length / fragment-FCS field serializers are **internal to EthernetMacHeader encoding**, not standalone top-level chunks a capture surfaces | a MAC-control (PAUSE) capture (S); the sub-fields are covered indirectly via Ethernet frames |
| **802.1 tag EPD headers** | 3 | Ieee8021aeTagEpdHeader (MACsec), Ieee8021rTagEpdHeader (802.1CB R-TAG), Ieee802EpdHeader | Sub-headers of MACsec / FRER / generic 802 tagging; no capture in the corpus carries those tags at the point they would be popped | captures with MACsec / 802.1CB R-TAG framing (M) |
| **PPP** | 2 | PppHeader, PppTrailer | The corpus is all Ethernet / 802.11 / raw-IP; no PPP-link-layer capture was added (the sims use PPP, but no PPP wire capture) | a PPP capture (S) |
| **CFM** (802.1ag CCM) | 1 | CfmContinuityCheckMessage | cfm.pcap parses as raw — EtherType 0x8902 has no dissector wired | wire a CFM dissector for 0x8902 (S–M) |
| **PIM** | 1 | PimStateRefresh | PIM-DM State-Refresh (type 9) is a rare, Cisco-specific message; no public capture | a PIM-DM State-Refresh capture (S) |
| **IEEE 802.15.4** | 1 | Ieee802154MacHeader | `Ieee802154MacHeader` is a 3-field placeholder (hard-coded FCF `0xCC01`) that cannot represent real 802.15.4 frames, so the capture was removed as untestable (documented TODO on the `.msg`) | model the real 802.15.4 wire format (L) |

### Exercised by neither test (3)

* **`Ieee80211MultiTidBlockAck`, `Ieee80211MultiTidBlockAckReq`** — **not modelled** (empty
  `.msg` body). The serializer deliberately throws *"unimplemented"* (a `CHK` guard now
  replaces the former null-deref crash), so there is no format to round-trip and no
  capture to replay. Genuinely untested **because it is untestable until the multi-TID
  BlockAck is modelled** (L).
* **`EthernetPadding`** — structural trailing zero-padding; a standalone `serialize()` is
  an *"Invalid operation"*. It **is** exercised as part of a padded Ethernet frame, but is
  never round-tripped as a standalone chunk. Correctly excluded (allow-listed in both
  tests).

### Deliberately skipped by pcap (56, not gaps)

These have no place in a captured-frame corpus and are excluded on purpose (most are still
round-tripped constructively where they have a wire form):

* **generic data chunks** — BytesChunk / ByteCountChunk / BitCountChunk / BitsChunk (no
  protocol layer; round-tripped by the constructed test);
* **12 PHY headers** — `physicallayer::*` (radio preambles/OFDM/HT/VHT/…), absent from a
  byte capture;
* **INET-internal simulation MACs** — AckingMac, ShortcutMac, B-MAC, X-MAC, CSMA-CA (no
  real-wire equivalent);
* **payload placeholders** — ApplicationPacket, EtherAppReq/Resp, VoipStreamPacket,
  EchoPacket, DsdvHello;
* **pseudo / abstract / dispatch bases** — TransportPseudoHeader, the `*Base` dispatch
  headers, the TPID tag sub-headers.

---

## Recommendation

The clean, single-field wins are done. Everything left is one of:

* a **multi-field TLV/list model** (MIPv6 options, SCTP parameter order, DHCP options) —
  real work plus fingerprint churn, best scoped as its own arc; **MIPv6 options** is the
  highest-value single item (9 frames) and reuses the "unknown-attribute" pattern already
  applied to BGP;
* **impossible** (encrypted ESP payload+trailer).

The OSPFv3 LS Update LSA-ordering differ (2 frames) — previously scoped as "L, single
ordered list" — was closed by replacing the five per-type LSA arrays with one ordered list
(`Ospfv3Lsa *lsas[]`, as OSPFv2 already models it), so a captured update round-trips in its
original wire order. The routing module still processes and emits LSAs grouped by type, so
locally originated updates are byte-identical and the change is fully fingerprint-neutral.

The gPTP Pdelay_Req reserved-octet differ (6 frames) — previously left as "INET is
spec-correct" — was closed by preserving the reserved octets verbatim on round-trip
(the serializer must not drop wire bytes, the same principle applied to the SCTP and BGP
unknown-parameter fixes); the simulation still emits zeros there, so it is
fingerprint-neutral.

The wpa-Induction "malformed frames" (10 error) were **not** an INET gap: ten of the
capture's 1093 frames are corrupt (nine with an invalid 802.11 Frame Control protocol
version, one truncated Probe Request) and Wireshark cannot parse them either. They were
removed with `editcap`, so the remaining 1083 frames round-trip cleanly and the file is
now a passing positive test (pcap errors 41 → 31).
