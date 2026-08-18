
# Serializer round-trip: remaining gaps

Status of the `serializer_pcap` / `serializer_pcap_known_failures` / `serializer_fields` /
`serializer_chunk_roundtrip` work on `topic/bz/serializertest`.

The round-trip tests exercise the **real `serialize()`** path: a `FieldsChunk` built by
deserialization used to cache the original wire bytes and the serializer replayed that
cache instead of re-encoding from the parsed fields, which hid every asymmetry. With the
cache cleared before re-serialization, the pcap corpus went from **1219 differing frames
to 30** (plus 41 error frames). This document lists what is left, why, the fix, and its
size.

A third test, `serializer_fields`, checks something the other two structurally cannot:
both of them reproduce the bytes of a frame, so a serializer that reads and writes a field
in the same wrong place passes them — a swap of two same-width neighbours survives a
round-trip untouched. That test dissects chosen frames and compares fields against values
read from tshark, an independent decoder. It was proved on a deliberately introduced swap
of the receiver and transmitter addresses of an 802.11 header: all three round-trip tests
stayed green, this one named both fields.

The pcap corpus is split in two, each half listed by the test that replays it — so a
regression in a working capture cannot hide among the documented gaps:

| test | captures | frames | verdict |
|---|---|---|---|
| `serializer_pcap` | the 41 that round-trip + 4 derived + 7 generated | 4177 total, **4177 same** | byte-exactness is gated |
| `serializer_pcap_known_failures` | the 6 with gaps | 341 total, 304 same, **22 differ, 15 error** | the counts are pinned |

Four of the six failing captures fail on a few frames only; their round-tripping frames
are cut out into `pcap/derived/` (see the README there) and replayed by the strict test as
well, so SCTP, the 802.11 block-ack / A-MSDU headers and most MIPv6 mobility headers are
held to byte-exact reproduction even though their captures cannot be admitted whole.

Fixing a gap means moving its capture line from `serializer_pcap_known_failures.test` to
`serializer_pcap.test` and dropping the derived file; the pinned counts of the former —
and this document — then have to follow.

Together they exercise **148 of 217** registered serializers (146 from the strict test, 37
from the other; only `DhcpMessage` and `BindingAcknowledgement` are exercised by failing
captures alone; 57 are not pcap-testable and 16 have no capture at all). The
[remaining differs/errors](#summary-table) are below; the serializers that **no capture
exercises** — and why — are in
[Untested serializers](#untested-serializers-registered-but-no-real-capture-coverage).

Legend for **Effort**: **S** = a contained change in one serializer or filler (under an
hour). **M** = a small model addition or a bounded serializer rework (a few hours). **L** =
a new list/TLV model or a message-family rework (a day+). **—** = impossible (encrypted
payload).

---

## Summary table

| Protocol | Frames | Kind | Root cause | Fix | Effort |
|---|---|---|---|---|---|
| MIPv6 (BU/BA) | 9 diff | gap | trailing mobility-**options** not modelled | generic option-TLV list | **L** |
| SCTP | 9 diff | gap | on-wire parameter **order** not preserved (fixed-field model, not a TLV list) | ordered-parameter list | **L** |
| DHCP | 4 diff | gap | `DhcpOptions` is a fixed struct, not a TLV list | option TLV-list rewrite | **L** |
| IEEE 802.11 | 9 err | gap | control subtypes **not modelled** (VHT/HE NDP Announcement, Trigger) | new frame types + serializer | **M** |
| IPsec ESP | 6 err | not fixable | ESP **trailer is inside the ciphertext** | (needs decryption) | **—** |

The 22 differing frames are all in the MIPv6 / SCTP / DHCP captures; the 15 error frames
break down as **6 ESP + 9 IEEE 802.11** control frames of unmodelled subtypes.

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

## IEEE 802.11 — unmodelled control subtypes (9 error)

What is left here is a *model* gap, not a serializer asymmetry. The frames are control
frames whose subtype INET does not model, so the MAC header deserializer marks the chunk
incorrect and the round-trip cannot even start (`Returning an incorrect chunk is not
allowed`, at byte offset 0):

- **8 × VHT/HE NDP Announcement** (type 1, subtype 5) in `wlan-blockack-reassoc.pcap`:
  19-octet frames `54 00 64 00 <RA> <TA> <sounding dialog token> <STA info>`.
- **1 × Trigger** (type 1, subtype 2, 802.11ax) in `wlan-aggregation.pcap`:
  `24 00 92 00 <RA> <TA> <common info> <user info>`.

**Fix:** add the frame types to `Ieee80211Frame.msg` and the matching branches to
`Ieee80211MacHeaderSerializer` (plus round-trip fillers). **Effort M.**

### Fixed in this arc

Three defects that used to account for 14 of the 802.11 errors and 4 length mismatches:

- **QoS Control bit order** (`Ieee80211MacHeaderSerializer`). QoS Control is a 16-bit
  little-endian field: within its first octet the TID is in B0–B3, then EOSP (B4), Ack
  Policy (B5–B6) and A-MSDU Present (B7). The serializer wrote/read it MSB-first
  (`writeUint4(tid) … writeBit(aMsduPresent)`), i.e. bit-reversed inside the octet.
  INET-to-INET traffic round-tripped (both directions used the same wrong order), so only
  a real capture could expose it: a QoS-Data frame with TID 7 (`07 00`) was parsed as
  TID 0 + **A-MSDU present**, sending the dissector into the A-MSDU walk and off the end
  of the frame (`offset is out of range`). 4 EAPOL frames in `wlan-addba-delba.pcap`.
- **Protected (encrypted) data frames** (`Ieee80211MacProtocolDissector`). With the
  Protected bit set the body is a CCMP header followed by ciphertext; the dissector
  descended into it as if it were LLC/SNAP. It is now left as opaque bytes — the only
  correct treatment without the keys, and the same one an ESP payload needs. 10 frames in
  `wlan-blockack-reassoc.pcap` / `wlan-aggregation.pcap`.
- **A-MSDU subframe header and padding dropped** (`Ieee80211MacProtocolDissector`). The
  A-MSDU walk popped each 14-octet `Ieee80211MsduSubframeHeader` without visiting it, and
  skipped the inter-subframe padding by moving the front offset, so the dissected content
  was short by 14 B per subframe plus the padding (2132 B frame → 2102 B rebuilt).
  Both are now handed to the callback. 4 frames in `wlan-aggregation.pcap`.

---

## PPP — a flag octet where the link type expects a direction

Not a round-trip failure, and not a gap in coverage either: the RTP capture is recorded on
a PPP link, so `PppHeader` and `PppTrailer` are exercised on real frames and reproduce
byte-exactly. What is questionable is the framing itself.

`PppHeader` carries the HDLC flag (0x7e) as a field of the header, and the recorder writes
the capture as link type 204 (PPP with direction), whose first octet is a direction
indicator rather than a flag. The two happen to line up -- Wireshark reads the flag as a
direction of 0x7e and finds the address and control octets where it expects them -- so the
frames decode, but for the wrong reason. `PppTrailer` carries a matching oddity in a
comment of its own: the trailer is two octets where the closing flag would make it three.

**Fix.** The flag delimits a frame, it is not a field of it: it belongs to the framing the
link type describes, not to the header chunk. Removing it changes the length of every PPP
frame, so the fingerprints of every simulation with a PPP link move with it -- which is
why this is written down rather than done. **Effort M.**

---

## CFM — the MEG ID is a MAID, and the RTP payload header is not RFC 2250

Two entries of the untested list, closed differently.

**CFM: fixed.** The 48-octet MEG ID of a Continuity Check Message was modelled as a single
name written in one fixed layout, so an 802.1ag frame -- which fills the field with a MAID:
a maintenance domain name and a short association name, each preceded by the format it is
written in -- could not be read back. The length octet of the second name landed in the
middle of the first, and the message was rejected as unrepresentable. The MAID is modelled
now, the End TLV moved out of the header into a chunk of its own -- it is a TLV, and the
fixed part of the message is 74 octets, not 75 -- and `cfm.pcap` moved to the strict test,
with a field reference that pins the names against what tshark reads. The optional TLVs a
frame may carry before the end -- Sender ID, Port Status and the rest -- are still not
A TLV of a CFM message is a chunk in its own right now, `CfmTlvBase`, and deserializing one
yields the TLV its type octet names: the one that ends the list, or a raw TLV holding the
type, the length and the octets of the ones this model does not read into fields -- Sender
ID, Port Status, Interface Status and the organization-specific ones. With that, the TLV
list of a captured frame is chunks all the way to its end rather than one opaque block,
and both TLV classes are exercised by a real frame. Two smaller things came out with it: the first
octet carries the level *and* the version, which are separate fields now, and the frames
INET sends are byte-identical in length but not in content, so the fingerprints of the
link-check configurations moved in `~tND` alone.

**RTP: not worth doing.** The RTP dissector leaves the media opaque, and the obvious next
step -- dispatching on the payload type to reach `RtpMpegHeader` -- would pin an
interpretation nothing else agrees with. INET's MPEG payload header is not the one RFC 2250
describes: the serializer writes a payload length and a picture type in four octets, with
the standard layout sitting commented out beside it. Wireshark reads the very same four
octets of the capture as MBZ, T, temporal reference and the rest. Dispatching would make
the round-trip exercise the serializer, and it would tell us nothing about whether the
frame is right. **Effort M** to model the real header, and it would change what INET's own
RTP model puts on the wire.

---

## MRP — the source address was not on the wire at all

Found by checking the recorded frames field by field rather than only structurally: every
MRP frame carried `MRP_SA = 00:00:00:00:00:00` while the Ethernet source was correct.
`Mrp::initialize` read the bridge address from the relay in `INITSTAGE_LINK_LAYER`, which
is the stage the relay assigns it in, so which of the two ran first was a matter of the
order the NED file declares them in -- and `mrp` comes before `bridging`.

Not only the bytes were affected: the module compares that address to decide which test
frame is its own and which manager wins an auto-manager election, and all-zero addresses
make both comparisons degenerate. Fixed by reading it in `start()`, after initialization
is over, which is what `StpBase` does with the same value.

---

## MRP — the auto-manager option TLV, fixed against the dissector

The frames an auto-manager sends were refused by the check that admits a generated
capture: Wireshark called every one of them `[Malformed Packet]`, and they carry the only
sub-TLVs INET has. Three things were wrong, and the last one was found by asking what
Wireshark *can* decode rather than what it rejected.

1. **The length of the option TLV stopped before the sub-TLV that follows it.** A standard
   dissector therefore read the sub-TLV as the next top-level TLV -- and so did INET, whose
   receive path steps over a TLV by that same length and then read the sub-TLV as the
   common TLV: `Cannot convert chunk from type inet::MrpSubTlvTest to type inet::MrpCommon`.
   That crash had been waiting for the manager election to run for the first time, which is
   why fixing the source address (see above) uncovered it.
2. **The TLV was padded to four octets and the padding was inside its length.** Wireshark
   aligns the TLV *after* its length, which is also the only reading in which the sub-TLV
   is where the length says it is. The alignment never affected any other MRP TLV -- all of
   them are a multiple of four octets long already -- so it is gone.
3. **The organization id was the IEC one.** Wireshark decodes the inner structure of an
   option TLV for exactly one id, `08:00:06` (Siemens), and there it reads an `ed1Type`
   octet followed by the sub-TLV -- precisely the structure INET models, down to the
   manufacturer-data lengths its `Ed1DataLength` enum lists. Replacing the three octets in
   a recorded frame made the same bytes decode entirely, which is what identified the id as
   the odd one out. INET writes the Siemens id now.

With those, `SmallNetworkMRA` produces frames Wireshark decodes to the last field --
`MRP_Option (SIEMENS)`, `MRP_Ed1Type: 0xff`, `MRP_SubOption2 { MRP_AutoMgr }` -- and all
three sub-TLV types (`MRP_AutoMgr`, `MRP_TestMgrNAck`, `MRP_TestPropagate`) are in the
corpus. The MRP dissector also had to learn that a sub-TLV follows the option TLV; without
it the round-trip read the sub-TLV as a topology-change TLV.

`MrpManufacturerFkt` is the one MRP serializer no capture reaches: INET never sends the
manufacturer sub-TLV, only parses it. It stays constructively tested.

The value that was left in that section is fixed too: a TestMgrNack named the manager it
rejects but gave its priority as zero, because the frame was filled with a constant while
the caller passed the priority in. Its sibling, the propagate message, had it right all
along -- and the field reference of the auto-manager capture now pins both.

---

## What the field reference does not name, and why

`serializer_fields.test` prints, after its result, every field of the chunks the corpus
dissects that no expectation names. That list is the work list, and this section is what is
left on it once the mapping has been worked through: 26 fields of 13 classes, in four
groups. None of them is an oversight -- what would make any of them checkable is written
next to it.

**Not on the wire.** No decoder can have an opinion, because nothing about them is
transmitted; the serializers never touch them.

  * `Ieee80211DataHeader.AID`, `Ieee80211DataHeader.MACArrive` -- the association id is a
    reading of the duration field the model keeps separately, and the arrival time is
    technical data the MAC passes to itself. Neither appears in the 802.11 serializer.
  * `SctpHeader.checksumOk`, `SctpHeader.headerLength` -- the SCTP common header has no
    length field, and whether a checksum verified is the result of a check, not a value read.

**A list inside the chunk.** The reference addresses a chunk by class and occurrence, so it
can name a field of a chunk but not a field of a field. Reaching these needs a path in the
second and third columns and a lookup in the test to match -- the same extension the SCTP
chunks have been waiting for.

  * `Ipv4Header.options`, `tcp::TcpHeader.headerOption`, `sctp::SctpHeader.sctpChunks`,
    `Mldv2Query.sourceList`, `Mldv2Report.multicastAddressRecord`.
  * `BytesChunk.byte`, `CfmTlvRaw.bytes` -- octets the dissection did not take apart at all.
    Wireshark does decode what is inside a CFM sender-id TLV, but this model keeps the body
    as bytes, so there is no field on this side to compare it against.

**The corpus carries no frame with it.** A capture that has one would make them checkable;
this is a note about the corpus, not about the mapping.

  * `BindingUpdate`'s eight proxy-MIPv6 option fields (`homeNetworkPrefix`,
    `mobileNodeIdentifier`, `timestampValue`, `accessTechnologyType`, `handoffIndicator`,
    `bindingAuthorizationData`, `homeAddressMN`, `homeNetworkPrefixLength`) -- the mobility
    options, which the frames here do not carry and which are a documented round-trip gap of
    their own (see the MIPv6 section above).
  * `BpduCfg.agreementFlag`, `BpduCfg.proposalFlag` -- RSTP bits; `stp-tcn.pcap` is legacy
    STP, and Wireshark reads only the topology-change bits out of that flags octet.
  * `Ieee80211DataHeader.address4` -- present only in a frame with both DS bits set.

**No independent value exists.** The field is on the wire, but nothing in the capture can
say what it should be.

  * `Ieee80211MacTrailer.fcs` -- the 802.11 capture carries no FCS (the radiotap header says
    so), and the test appends a computed one itself, so the value would be INET's own
    arithmetic confirmed against itself.
  * `PppHeader.flag`, `PppTrailer.flag`, `PppTrailer.fcs` -- the PPP framing gap documented
    above: the model puts the HDLC flag octet inside the header while link type 204 begins
    with a direction indicator, and Wireshark names neither those octets nor a trailing FCS.

---

## SCTP — a checksum that came out right only on a little endian machine

The field reference could not name `SctpHeader.checksum` while the value it held depended on
the host: the serializer builds the common header as a struct over a byte buffer and wrote
`ch->checksum = crc32c(...)`, a plain assignment, so the octets that reached the wire were
whatever order this machine stores an `uint32_t` in.

The order they have to be in is the one RFC 4960 appendix B produces, and it is worth being
precise about what that is. The checksum field is network byte order like every other field
of the header -- the standard writes it with `htonl` -- but what goes *into* it is not the
CRC32c: the reference routine ends with a byte swap, so the field carries the CRC least
significant octet first. Swapping and then writing big endian is writing little endian, so
one `htole32` says it, and says it on either kind of host. What the plain assignment did was
right on x86 by coincidence and wrong on a big endian machine. The read side had the same
shape: it compared the field as the host reads it against the CRC it computed, which agrees
with itself on either machine but only matches the wire on one of them.

That the wire order is little endian is not taken on trust: the CRC32c of the three frames
of `sctp-www-clean.cap`, computed over each packet with its checksum field zeroed, equals
the little endian reading of the octets in the capture and not the big endian one. The
number INET keeps in the field is therefore the CRC itself, and Wireshark's `sctp.checksum`
is that number byte-swapped -- which is why the mapping reads the field's octets rather than
its printed value.

`htole32`/`le32toh` now say it explicitly, from `INETEndians.h`, a header INET already had
for exactly this and nothing used. On a little endian host both are the identity, so no byte
and no fingerprint moves; what changes is that a big endian one now writes the same frame.

The checksum is in the field reference since, so the convention is pinned.

---

## MRP — a link-change TLV two octets short of the standard one

Found by the field reference, and only after its mapping was extended to the fields every
MRP TLV carries: on six frames of `mrp-ring.pcap` the reference held an `MRP_End` where
INET's dissection had an `MRP_Common`, which is what a decoder sees when a TLV before it
is the wrong length.

`MrpLinkChange` -- the class behind both `MRP_LinkUp` and `MRP_LinkDown` -- was four octets
short. The TLV carries `MRP_SA (6)`, `MRP_PortRole (2)`, `MRP_Interval (2)`,
`MRP_Blocked (2)` and two reserved octets that put its end on a four octet boundary, a
length of 14; INET wrote the address, the interval and the blocked flag, a length of 10. Its
own interconnection variant, `MrpInLinkChange`, had the port role all along -- the ring
variant simply left it out.

Adding the port role alone was not enough, and the same reference said so: with a length of
12 Wireshark still read the next TLV two octets late. What settles the size is that it
advances a *fixed* fourteen octets past this TLV whatever the length field says -- it did so
with the original ten as well -- while on the neighbouring `MRP_TopologyChange` it advances
by exactly the declared length. So the two octets are part of the TLV, not alignment
Wireshark applies on its own, and the TLV is sixteen octets long like every other one.

What that cost is more than the missing field. A standard dissector reads `Interval` and
`Blocked` two octets late, so `Blocked` lands on the *next* TLV's header (`01 12`, the
common TLV and its length), and the dissector then continues inside that TLV's payload,
finds a zero octet and calls it the end of the frame. The common TLV -- sequence id and
domain uuid, the fields that identify the ring -- was never decoded at all. Nothing marked
the frame malformed, which is why the check that admitted these captures let it through:
the frame decodes, it just decodes into the wrong fields.

Neither round-trip test could see it either. INET wrote and read the same short TLV, so the
bytes came back identical; only a second opinion about what those bytes mean shows it, which
is the whole argument for `serializer_fields.test`.

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

The tests cover different things. `serializer_chunk_roundtrip` **constructs** an
instance of every registered chunk type — from hand-written typed fillers in
`tests/serializer/lib/fillers/`, one or more per type, a variant per branch the serializer takes
on the content — and checks `serialize → deserialize → serialize`. It currently runs **260
cases** with two gates at `0 unexpected`: every registered serializer class is exercised or
allow-listed, and every wire field of every filled chunk carries a value or is listed as
knowingly left at its default. The two pcap tests instead replay a corpus of real
Wireshark captures, so they only exercise serializers a capture actually contains.

Of the **217 registered serializers**, the two pcap runs together reach **148**, and
report **57 skipped** (not pcap-testable, see below) and **16 registered types that no
capture invokes**. Cross-referencing those 42 against the constructed test:

* **13 are still tested constructively** by `serializer_chunk_roundtrip` — they merely
  lack a *real-wire* exercise (no capture in the corpus carries them);
* **3 are exercised by neither** as a standalone chunk (next subsection).

"Untested" below therefore means **"no real capture exercises it"**, not "no test at all"
(except the 3 explicitly flagged).

### No real capture, but round-tripped constructively (13)

| Family | Count | Serializers | Why no capture exercises it | To light up (effort) |
|---|---|---|---|---|
| **MRP manufacturer sub-TLV** | 1 | MrpManufacturerFkt | INET parses the manufacturer sub-TLV but never sends one, so no recording can carry it | — |
| **RTP payload formats** | 1 | RtpMpegHeader | A capture with an MPEG payload is in the corpus, but the header INET writes there is not the one RFC 2250 describes (see above), so reaching it would pin an interpretation nothing agrees with | model the real header, then dispatch on the payload type (M) |
| **IPv6 extension headers** | 3 | Ipv6AuthenticationHeader (AH), Ipv6DestinationOptionsHeader, Ipv6EncapsulatingSecurityPayloadHeader (ESP) | The AH / DestOpts captures were dropped from the corpus (malformed / model mismatch); the ESP header chunk round-trips, but a real ESP-protected packet cannot (payload+trailer encrypted — see the ESP section) | add clean AH/DestOpts captures (S–M); ESP is impossible |
| **IEEE 802.11 A-MPDU** | 1 | Ieee80211MpduSubframeHeader | No A-MPDU-subframe capture is in the corpus (only A-MSDU) | add an A-MPDU capture (S) |
| **Ethernet sub-fields** | 3 | EthernetMacAddressFields, EthernetTypeOrLengthField, EthernetFragmentFcs | The address / type-or-length / fragment-FCS field serializers are **internal to EthernetMacHeader encoding**, not standalone top-level chunks a capture surfaces | covered indirectly via Ethernet frames |
| **802.1 tag EPD headers** | 2 | Ieee8021aeTagEpdHeader (MACsec), Ieee802EpdHeader | The R-TAG is covered by a generated capture now. MACsec has no example to record from, and the generic 802 EPD header is only popped in the 5.9 GHz 802.11 band | a MACsec capture (M) |
| **PIM** | 1 | PimStateRefresh | No public capture, and the shipped PIM-DM example never originates a State Refresh either, so there was nothing to record | a scenario that originates one, then record (M) |
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

## Measuring serializer coverage

What the two tests cover in *breadth* is gated (every registered serializer class is
exercised, every wire field of every filled chunk is set). Their *depth* -- how much of
each serializer's code actually runs -- is measured with llvm-cov instead of guessed.

OMNeT++ ships no `_coverage`-suffixed libraries, so an instrumented INET has to be linked
against the ordinary debug ones. Only INET needs the instrumentation; the kernel does not.

```
# instrumented INET (note: src/makefrag adds the OSG libraries with +=, which a
# command-line override of OMNETPP_LIBS would drop, hence they are repeated here)
make MODE=coverage KERNEL_LIBS=-loppsim_dbg \
     OMNETPP_LIBS='-loppenvir_dbg $(KERNEL_LIBS) $(SYS_LIBS) -losg -losgText -losgDB -losgGA -losgViewer -losgUtil -lOpenThreads' -j8
cd tests/serializer/lib && make MODE=coverage KERNEL_LIBS=-loppsim_dbg \
     OMNETPP_LIBS='-loppenvir_dbg $(KERNEL_LIBS) $(SYS_LIBS)' -j8

# the two test programs
for t in serializer_chunk_roundtrip serializer_pcap serializer_pcap_known_failures; do
  (cd tests/serializer/work/$t && opp_makemake -f --deep -o $t -lINET_coverage -L../../../../src \
        -ltest_coverage -L../../lib -I../../../../src -I../../lib &&
   make MODE=coverage KERNEL_LIBS=-loppsim_dbg OPPMAIN_LIB=-loppmain_dbg \
        USERIF_LIBS='-loppcmdenv_dbg -loppenvir_dbg' \
        OMNETPP_LIBS='$(OPPMAIN_LIB) $(USERIF_LIBS) $(KERNEL_LIBS) $(SYS_LIBS)' -j8)
done

# run both under one profile each, then report
for t in serializer_chunk_roundtrip serializer_pcap serializer_pcap_known_failures; do
  (cd tests/serializer/work/$t && LLVM_PROFILE_FILE=/tmp/cov-$t.profraw \
      ./${t}_coverage -u Cmdenv -f _defaults.ini --check-signals=false -n ../../../../src:.:../../lib)
done
llvm-profdata merge -sparse /tmp/cov-*.profraw -o /tmp/merged.profdata
llvm-cov report src/libINET_coverage.so -instr-profile=/tmp/merged.profdata $(find src/inet -name '*Serializer.cc')
```

Read the result per **function**, not per file: a `*Serializer.cc` also holds pcap-recorder
helpers and error paths that neither test should reach. Restricted to the `serialize()` and
`deserialize()` bodies, and discounting the regions whose only statement is a `throw`, the
current run leaves **2997 of 7079 regions unexercised (58% covered)**. Where they sit:

| Uncovered regions | Serializer | What is not reached |
|---|---|---|
| 1561 | SctpHeaderSerializer | `deserialize()` alone is 991 of 1432: the INIT/INIT-ACK parameter loops, the stream-reset parameter types, the error causes, ASCONF and AUTH. The fillers build one chunk per SCTP chunk type, not one per parameter type |
| 118 | Ieee80211MgmtFrameSerializer | the per-information-element branches of the management bodies |
| 117 | DhcpMessageSerializer | the per-option branches, mostly on the read side |
| 117 | Icmpv6HeaderSerializer | ND option types and the MLD/error message kinds a filler does not build |
| 116 | Ipv4HeaderSerializer | option types and their read-side validation |
| 101 | TcpHeaderSerializer | the per-option read branches |
| 79 | BgpHeaderSerializer | path-attribute types beyond the ones the fillers build |
| 77 | MobilityHeaderSerializer | the mobility-option branches (the same gap the MIPv6 section describes) |
| 63 | Ieee80211MacHeaderSerializer | frame subtypes and the address/QoS combinations without a variant |
| 56 / 48 | Ospfv3 / Ospfv2 PacketSerializer | LSA types and their inner list shapes |
| 46 / 39 / 34 | Pim / Mrp / Igmp | option and TLV types without a variant |

Each row is a **filler-variant to-do list**, in priority order: an uncovered branch is either
a variant nobody wrote, dead code, or a path only a malformed frame reaches -- and the three
are worth telling apart.

---

## Gaps the constructed test surfaces (no capture involved)

These are reachable from a hand-built chunk, so the corpus has nothing to do with them.
Each is a model decision rather than a missing branch, which is why they are documented
instead of fixed.

| Item | What is wrong | Fix | Effort |
|---|---|---|---|
| **SCTP COOKIE_ECHO** | The chunk can carry the state cookie in two encodings — raw `cookie[]` octets or a structured `stateCookie` — but the deserializer always rebuilds the structured one, so a chunk written from the raw octets comes back in the other encoding and re-serializes differently | decide which encoding is canonical; let the deserializer pick from the chunk length | **M** |
| **SCTP ERROR causes** | serialize knows two cause codes, deserialize recognizes a third (and without its value), and it forces the reconstructed chunk to a fixed 4-octet length, so no ERROR chunk with parameters round-trips | model the cause list as a TLV list, the same pattern as the INIT parameters | **M** |
| **SCTP PKTDROP** | The chunk's payload is handled as an encapsulated `cPacket`, but an `SctpHeader` is a `Chunk`, so a payload the cast accepts cannot be constructed at all; the deserialize case is commented out | model the dropped packet as a chunk (or remove the type) | **M** |
| **SCTP INIT/INIT-ACK AUTH parameters** | `random`, `hmacTypes`, `sctpChunkTypes`, `msg_rwnd` back the RANDOM / HMAC_ALGO / CHUNKS parameters; the fillers do not build them, so those serializer branches are unexercised | a filler variant per parameter | **S** |
| **Modelled but never serialized** | `PimHelloOptionType::AddressList` has no serializer case, `Ieee8021ae*::sci` and `EthernetFragmentFcs::mFcs` are never written or read, the AH/ESP extension-header content is a zero-fill TODO | either serialize the field or drop it from the model; each currently costs an entry on the test's knownUnfilled list | **S** each |
| **IEEE 802.15.4 header length** | The MAC module sizes the header from its `headerLength` parameter (72 b) while the serializer writes 23 octets, so a serialized 802.15.4 frame fails the length check | model the real wire format (the same item as in the table above) | **L** |

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

The 802.11 BlockAckReq layout (1 error, plus the two Basic/Compressed BAR serializers that
no capture could reach) was closed by putting the frame on the wire in its standard 20-octet
format — a 2-octet BAR Control field and a 2-octet Block Ack Starting Sequence Control —
instead of the 38-octet layout that wrote the fragment number as a 32-bit value and the
sequence number as a 64-bit one. The BlockAck frame's Starting Sequence Control had the same
bit-order defect and now goes through the same helper as the Sequence Control field. The
frames a simulation sends shrink from 38 to 20 octets, so the seven wireless fingerprints
that exercise block ack were re-recorded; `wlan-blockack-basic.pcap` now round-trips
completely and the exercised count went 114 → 116.

The wpa-Induction "malformed frames" (10 error) were **not** an INET gap: ten of the
capture's 1093 frames are corrupt (nine with an invalid 802.11 Frame Control protocol
version, one truncated Probe Request) and Wireshark cannot parse them either. They were
removed with `editcap`, so the remaining 1083 frames round-trip cleanly and the file is
now a passing positive test (pcap errors 41 → 31).
