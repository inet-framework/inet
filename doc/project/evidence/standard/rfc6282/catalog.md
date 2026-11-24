# RFC 6282 (6LoWPAN) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC6282-*` · **Stands on:** [standards.md](../../protocol/6lowpan/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This is the statement-catalog artifact for RFC 6282 in the selected 6LoWPAN family.
Source: [rfc6282.txt](rfc6282.txt), downloaded unmodified on 2026-09-18 from
<https://www.rfc-editor.org/rfc/rfc6282.txt>. The publication version and family
relationships are pinned in the linked standards map.

The catalog is a scoped selection, not an exhaustive inventory or a claim of test
coverage. Its scope exclusions are listed at the end. No test-depth level is claimed.
Statements and check ideas are independent of any simulation implementation.

Quotes retain the wording and line breaks of the cached text; leading indentation
is omitted; excerpts may start or end within a line. Line references count physical
newline-delimited lines, including page
headers. Strength records the source keyword, including qualified permissions;
`description` denotes a format or procedure stated without a requirement keyword.
Each check idea is a proposed observation, not a test result. Conditional rules apply
only to the roles, modes and prerequisites named in their source paragraphs.

## Index

| ID | Statement |
| --- | --- |
| [RFC6282-LEGACY-1](#rfc6282-legacy-1) | New implementations should not transmit RFC 4944 HC1-compressed packets. |
| [RFC6282-FRAG-1](#rfc6282-frag-1) | A header that does not fit in the first RFC 4944 fragment is not compressed. |
| [RFC6282-IPHC-1](#rfc6282-iphc-1) | HLIM selects an inline value or the constants 1, 64 and 255. |
| [RFC6282-IPHC-2](#rfc6282-iphc-2) | CID zero omits the extension and uses context zero for context-based compression. |
| [RFC6282-IPHC-3](#rfc6282-iphc-3) | CID one places an extension octet immediately after DAM. |
| [RFC6282-IPHC-4](#rfc6282-iphc-4) | SAC one and SAM zero encode the unspecified source address. |
| [RFC6282-IPHC-5](#rfc6282-iphc-5) | IPv6 Payload Length is elided and reconstructed from lower-layer information. |
| [RFC6282-ADDR-1](#rfc6282-addr-1) | IID bits that do not match the link-layer mapping cannot be elided. |
| [RFC6282-MCAST-1](#rfc6282-mcast-1) | Stateless eight-bit multicast compression reconstructs ff02::00XX. |
| [RFC6282-NHC-1](#rfc6282-nhc-1) | Decompressed extension headers are padded to a multiple of eight octets. |
| [RFC6282-NHC-2](#rfc6282-nhc-2) | Extension headers exceeding 255 octets after the compressed Length field cannot use LOWPAN_NHC. |
| [RFC6282-NHC-3](#rfc6282-nhc-3) | An encapsulated IPv6 header uses EID seven, NH zero and LOWPAN_IPHC. |
| [RFC6282-UDP-1](#rfc6282-udp-1) | Checksum elision requires upper-layer authorization under an allowed integrity case. |
| [RFC6282-UDP-2](#rfc6282-udp-2) | The compressor verifies the UDP checksum with additional integrity protection in place and drops failures. |
| [RFC6282-UDP-3](#rfc6282-udp-3) | The decompressor restores an elided checksum and rejects missing or failed additional integrity protection. |
| [RFC6282-UDP-4](#rfc6282-udp-4) | UDP port mode zero carries both ports in full. |
| [RFC6282-UDP-5](#rfc6282-udp-5) | UDP port mode one elides the destination port high byte 0xf0. |
| [RFC6282-UDP-6](#rfc6282-udp-6) | UDP port mode two elides the source port high byte 0xf0. |
| [RFC6282-UDP-7](#rfc6282-udp-7) | UDP port mode three elides the common 0xf0b prefix of both ports. |
| [RFC6282-UDP-8](#rfc6282-udp-8) | UDP Length is elided and inferred from lower layers. |
| [RFC6282-DISPATCH-1](#rfc6282-dispatch-1) | LOWPAN_IPHC preempts the old ESC value and reserves 0x40 as its replacement. |

## Checkable statements

### RFC6282-LEGACY-1

**New implementations should not transmit RFC 4944 HC1-compressed packets.**

> This document specifies a header compression format that is intended
> to replace that defined in Section 10 of [RFC4944].  Implementation
> of Section 10 of [RFC4944] is now NOT RECOMMENDED.  New
> implementations MAY implement decompression according to Section 10
> of [RFC4944] but SHOULD NOT send packets compressed according to
> Section 10 of [RFC4944].

— §2, `rfc6282.txt:212-217`.

- Strength: should not; may. Class: wire.
- Check idea: Exercise normal compression output and verify IPHC rather than HC1; treat legacy decompression as optional.

### RFC6282-FRAG-1

**A header that does not fit in the first RFC 4944 fragment is not compressed.**

> Section 5.3 of [RFC4944] also defines how to fragment compressed IPv6
> datagrams that do not fit within a single link frame.  Section 5.3 of
> [RFC4944] defines the fragment header's datagram_size and
> datagram_offset values as the size and offset of the IPv6 datagram
> before compression.  As a result, all fragment payload outside the
> first fragment must carry their respective portions of the IPv6
> datagram before compression.  This document does not change that
> requirement.  When using the fragmentation mechanism described in
> Section 5.3 of [RFC4944], any header that cannot fit within the first
> fragment MUST NOT be compressed.

— §2, `rfc6282.txt:235-244`.

- Strength: must not. Class: encoding.
- Check idea: Use an extension-header chain that crosses first-fragment capacity and check that the crossing header remains uncompressed.

### RFC6282-IPHC-1

**HLIM selects an inline value or the constants 1, 64 and 255.**

> 00:  The Hop Limit field is carried in-line.
>
> 01:  The Hop Limit field is compressed and the hop limit is 1.
>
> 10:  The Hop Limit field is compressed and the hop limit is 64.
>
> 11:  The Hop Limit field is compressed and the hop limit is 255.

— §3.1.1, `rfc6282.txt:352-358`.

- Strength: description. Class: encoding.
- Check idea: Round-trip hop limits 1, 64, 255 and a different value, checking each encoding against the adjacent source definitions.

### RFC6282-IPHC-2

**CID zero omits the extension and uses context zero for context-based compression.**

> 0: No additional 8-bit Context Identifier Extension is used.  If
> context-based compression is specified in either Source Address
> Compression (SAC) or Destination Address Compression (DAC),
> context 0 is used.

— §3.1.1, `rfc6282.txt:362-365`.

- Strength: description. Class: encoding.
- Check idea: Compress using context zero with CID clear and check both the absence of an extension octet and reconstruction.

### RFC6282-IPHC-3

**CID one places an extension octet immediately after DAM.**

> 1: An additional 8-bit Context Identifier Extension field
> immediately follows the Destination Address Mode (DAM) field.

— §3.1.1, `rfc6282.txt:367-368`.

- Strength: description. Class: encoding.
- Check idea: Select nonzero context IDs and verify extension placement and both context selectors.

### RFC6282-IPHC-4

**SAC one and SAM zero encode the unspecified source address.**

> 00:  The UNSPECIFIED address, ::

— §3.1.1, `rfc6282.txt:412-412`.

- Strength: description. Class: encoding.
- Check idea: Round-trip the unspecified source address and distinguish it from fully elided link-local addressing.

### RFC6282-IPHC-5

**IPv6 Payload Length is elided and reconstructed from lower-layer information.**

> Fields carried in-line (in part or in whole) appear in the same order
> as they do in the IPv6 header format [RFC2460].  The Version field is
> always elided.  Unicast IPv6 addresses may be compressed to 64 or 16
> bits or completely elided.  Multicast IPv6 addresses may be
> compressed to 8, 32, or 48 bits.  The IPv6 Payload Length field MUST
> always be elided and inferred from lower layers using the 6LoWPAN
> Fragmentation header or the IEEE 802.15.4 header.

— §3.2, `rfc6282.txt:590-596`.

- Strength: must. Class: encoding.
- Check idea: Round-trip fragmented and unfragmented datagrams and compare the restored IPv6 payload lengths.

### RFC6282-ADDR-1

**IID bits that do not match the link-layer mapping cannot be elided.**

> The remainder of this section defines the mapping from IEEE 802.15.4
> [IEEE802.15.4] link-layer addresses to IIDs for both short and
> extended IEEE 802.15.4 addresses.  IID bits not covered by the
> context information MAY be elided if they match the link-layer
> address mapping and MUST NOT be elided if they do not.

— §3.2.2, `rfc6282.txt:655-659`.

- Strength: must not; may. Class: encoding.
- Check idea: Vary IID bits not supplied by context and confirm elision occurs only when the encapsulating-address mapping matches.

### RFC6282-MCAST-1

**Stateless eight-bit multicast compression reconstructs ff02::00XX.**

> 11:  8 bits.  The address takes the form ff02::00XX.

— §3.1.1, `rfc6282.txt:527-527`.

- Strength: description. Class: encoding.
- Check idea: Round-trip ff02::1 and ff02::2 using M one, DAC zero and DAM three.

### RFC6282-NHC-1

**Decompressed extension headers are padded to a multiple of eight octets.**

> IPv6 Hop-by-Hop and Destination Options Headers may use a trailing
> Pad1 or PadN to achieve 8-octet alignment.  When there is a single
> trailing Pad1 or PadN option of 7 octets or less and the containing
> header is a multiple of 8 octets, the trailing Pad1 or PadN option
> MAY be elided by the compressor.  A decompressor MUST ensure that the
> containing header is padded out to a multiple of 8 octets in length,
> using a Pad1 or PadN option if necessary.  Note that Pad1 and PadN
> options that appear in locations other than the end MUST be carried
> in-line as they are used to align subsequent options.

— §4.2, `rfc6282.txt:911-919`.

- Strength: must. Class: encoding.
- Check idea: Elide eligible trailing padding, decompress and check both length alignment and option preservation.

### RFC6282-NHC-2

**Extension headers exceeding 255 octets after the compressed Length field cannot use LOWPAN_NHC.**

> Note that specifying units in octets means that LOWPAN_NHC MUST NOT
> be used to encode IPv6 Extension Headers that have more than 255
> octets following the Length field after compression.

— §4.2, `rfc6282.txt:921-923`.

- Strength: must not. Class: encoding.
- Check idea: Exercise compressed lengths 255 and 256 and verify the encoding limit.

### RFC6282-NHC-3

**An encapsulated IPv6 header uses EID seven, NH zero and LOWPAN_IPHC.**

> When the identified next header is an IPv6 Header (EID=7), the NH bit
> of the LOWPAN_NHC encoding is unused and MUST be set to zero.  The
> following bytes MUST be encoded using LOWPAN_IPHC as defined in
> Section 3.

— §4.2, `rfc6282.txt:925-928`.

- Strength: must. Class: encoding.
- Check idea: Encode an IPv6-in-IPv6 header and check the NHC selector and following IPHC header.

### RFC6282-UDP-1

**Checksum elision requires upper-layer authorization under an allowed integrity case.**

> With this specification, a compressor in the source transport
> endpoint MAY elide the UDP Checksum if it is authorized by the upper
> layer.  The compressor MUST NOT set the C bit unless it has received
> such authorization.  Requiring upper-layer authorization ensures that
> the intended transport peer will have sufficient means to deal with
> any data corruption that occurs before reaching the destination.  The
> upper layer MUST NOT provide the authorization unless one of the
> following cases is satisfied:
>
> Tunneling:  In this case, 6LoWPAN is deployed as a wireless pseudo-
> fieldbus by tunneling existing field protocols over UDP.  If the
> tunneled Protocol Data Unit (PDU) possesses its own addressing,
> security and integrity check (e.g., IPsec Encapsulating Security
> Payload tunnel mode [RFC4303] or IP over UDP encapsulation), the
> tunneling mechanism MAY authorize eliding the UDP checksum in
> order to save on the encapsulation overhead.
>
> Message Integrity Check:  In this case, either IPsec Authentication
> Header [RFC4302] or some other form of integrity check in the UDP
> payload that covers at least the same information as the UDP
> checksum (pseudo-header, data) and has at least the same strength.

— §4.3.2, `rfc6282.txt:979-999`.

- Strength: must not. Class: wire.
- Check idea: With authorization absent, verify C remains clear; enable each permitted authorization case separately.

### RFC6282-UDP-2

**The compressor verifies the UDP checksum with additional integrity protection in place and drops failures.**

> A compressor MUST verify the UDP Checksum before it is elided and
> MUST ensure that the additional integrity check is in place before
> verifying and eliding the checksum.  If verification of the UDP
> Checksum fails, the compressor MUST drop the packet.

— §4.3.2, `rfc6282.txt:1019-1022`.

- Strength: must. Class: end-to-end.
- Check idea: Supply a bad checksum and verify no compressed packet is emitted; verify the integrity prerequisite on a valid packet.

### RFC6282-UDP-3

**The decompressor restores an elided checksum and rejects missing or failed additional integrity protection.**

> A decompressor that expands a 6LoWPAN packet with the C bit set MUST
> compute the UDP Checksum on behalf of the source node and place that
> value in the restored UDP header as specified in the incumbent
> standards [RFC0768], [RFC2460].  The decompressor MUST unambiguously
> determine that an additional integrity check was put in place by the
> compressor and verify the integrity check and SHOULD do so after
> restoring the UDP Checksum.  If the decompressor cannot unambiguously
> determine the presence of an integrity check or verification fails,
> the decompressor MUST drop the packet.

— §4.3.2, `rfc6282.txt:1024-1032`.

- Strength: must; should. Class: end-to-end.
- Check idea: Decode C-set packets with valid, absent and corrupted integrity protection; check the restored checksum or discard.

### RFC6282-UDP-4

**UDP port mode zero carries both ports in full.**

> 00:  All 16 bits for both Source Port and Destination Port are
> carried in-line.

— §4.3.3, `rfc6282.txt:1089-1090`.

- Strength: description. Class: encoding.
- Check idea: Round-trip arbitrary 16-bit ports with P zero and verify four inline port octets.

### RFC6282-UDP-5

**UDP port mode one elides the destination port high byte 0xf0.**

> 01:  All 16 bits for Source Port are carried in-line.  First 8
> bits of Destination Port is 0xf0 and elided.  The remaining 8
> bits of Destination Port are carried in-line.

— §4.3.3, `rfc6282.txt:1092-1094`.

- Strength: description. Class: encoding.
- Check idea: Round-trip destination ports at both ends of 0xf000–0xf0ff with a full source port.

### RFC6282-UDP-6

**UDP port mode two elides the source port high byte 0xf0.**

> 10:  First 8 bits of Source Port are 0xf0 and elided.  The
> remaining 8 bits of Source Port are carried in-line.  All 16
> bits for Destination Port are carried in-line.

— §4.3.3, `rfc6282.txt:1096-1098`.

- Strength: description. Class: encoding.
- Check idea: Round-trip source ports at both ends of 0xf000–0xf0ff with a full destination port.

### RFC6282-UDP-7

**UDP port mode three elides the common 0xf0b prefix of both ports.**

> 11:  First 12 bits of both Source Port and Destination Port are
> 0xf0b and elided.  The remaining 4 bits for each are carried
> in-line.

— §4.3.3, `rfc6282.txt:1100-1102`.

- Strength: description. Class: encoding.
- Check idea: Round-trip ports 0xf0b0 and 0xf0bf and verify two four-bit suffixes.

### RFC6282-UDP-8

**UDP Length is elided and inferred from lower layers.**

> Fields carried in-line (in part or in whole) appear in the same order
> as they do in the UDP header format [RFC0768].  The UDP Length field
> MUST always be elided and is inferred from lower layers using the
> 6LoWPAN Fragmentation header or the IEEE 802.15.4 header.

— §4.3.3, `rfc6282.txt:1104-1107`.

- Strength: must. Class: encoding.
- Check idea: Decompress fragmented and unfragmented UDP packets and verify the restored length.

### RFC6282-DISPATCH-1

**LOWPAN_IPHC preempts the old ESC value and reserves 0x40 as its replacement.**

> This assignment preempts the assignment of 01 111111 for ESC
> [RFC4944]; this preemption is possible because extension bytes that
> would enable the use of ESC have not been allocated yet.  Instead,
> the value:
>
> 01 000000
>
> is reserved as a replacement value for ESC, to be finally assigned
> with the first assignment of extension bytes.

— §5, `rfc6282.txt:1127-1135`.

- Strength: description. Class: encoding.
- Check idea: Interpret 0x7f as an IPHC dispatch and reserve 0x40 for the later ESC definition.
- Overridden by: [RFC8066-ESC-1](../rfc8066/catalog.md#rfc8066-esc-1). RFC 8066 defines the reserved replacement ESC value and its extension octet format.

## Areas outside this selection

This catalog selects IPHC field handling, fragmentation interaction, selected address encodings, extension-header limits and UDP compression. The complete TF, SAC/SAM, DAC/DAM and context-ID encoding matrices (§3), all extension-header EIDs (§4.2), forwarding-node checksum authorization (§4.3.2), future NHC allocations (§5) and the security analysis (§6) are not exhaustively cataloged.
