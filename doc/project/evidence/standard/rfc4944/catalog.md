# RFC 4944 (6LoWPAN) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC4944-*` · **Stands on:** [standards.md](../../protocol/6lowpan/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This is the statement-catalog artifact for RFC 4944 in the selected 6LoWPAN family.
Source: [rfc4944.txt](rfc4944.txt), downloaded unmodified on 2026-09-18 from
<https://www.rfc-editor.org/rfc/rfc4944.txt>. The publication version and family
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
| [RFC4944-LINK-1](#rfc4944-link-1) | IPv6 packets use IEEE 802.15.4 data frames. |
| [RFC4944-LINK-2](#rfc4944-link-2) | Multicast broadcast frames carry the destination PAN identifier. |
| [RFC4944-LINK-3](#rfc4944-link-3) | Multicast broadcast frames use short destination address 0xffff. |
| [RFC4944-MTU-1](#rfc4944-mtu-1) | The IPv6 link MTU is 1280 octets. |
| [RFC4944-DISPATCH-1](#rfc4944-dispatch-1) | Stacked headers follow the prescribed mesh, broadcast, fragmentation order. |
| [RFC4944-DISPATCH-2](#rfc4944-dispatch-2) | Protocol datagrams have a valid LoWPAN encapsulation header. |
| [RFC4944-MESH-1](#rfc4944-mesh-1) | The V bit identifies the originator address width. |
| [RFC4944-MESH-2](#rfc4944-mesh-2) | The F bit identifies the final destination address width. |
| [RFC4944-MESH-3](#rfc4944-mesh-3) | Each forwarder decrements Hops Left and stops forwarding at zero. |
| [RFC4944-FRAG-1](#rfc4944-frag-1) | Oversized datagrams use fragmentation, with eight-octet alignment except for the last fragment. |
| [RFC4944-FRAG-2](#rfc4944-frag-2) | Subsequent fragments carry the subsequent-fragment header. |
| [RFC4944-FRAG-3](#rfc4944-frag-3) | The fragment datagram size describes the uncompressed IP packet. |
| [RFC4944-FRAG-4](#rfc4944-frag-4) | All fragments share a tag, incremented for successive datagrams with wrap at 65535. |
| [RFC4944-FRAG-5](#rfc4944-frag-5) | Subsequent-fragment offsets count eight-octet units from the start of the datagram. |
| [RFC4944-REASM-1](#rfc4944-reasm-1) | Reassembly identifies a datagram by source, destination, size and tag. |
| [RFC4944-REASM-2](#rfc4944-reasm-2) | An overlapping fragment with a different size or offset discards accumulated fragments. |
| [RFC4944-REASM-3](#rfc4944-reasm-3) | Incomplete reassembly is discarded on timeout, with a maximum timeout of 60 seconds. |
| [RFC4944-ADDR-1](#rfc4944-addr-1) | Stateless autoconfiguration uses a 64-bit prefix. |
| [RFC4944-LEGACY-1](#rfc4944-legacy-1) | Legacy HC1 carries Hop Limit after the encoding fields. |
| [RFC4944-MESH-4](#rfc4944-mesh-4) | Transmission through a mesh forwarder includes originator and ultimate destination addresses. |

## Checkable statements

### RFC4944-LINK-1

**IPv6 packets use IEEE 802.15.4 data frames.**

> IEEE 802.15.4 defines four types of frames: beacon frames, MAC
> command frames, acknowledgement frames, and data frames.  IPv6
> packets MUST be carried on data frames.  Data frames may optionally
> request that they be acknowledged.  In keeping with [RFC3819], it is
> recommended that IPv6 packets be carried in frames for which
> acknowledgements are requested so as to aid link-layer recovery.

— §2, `rfc4944.txt:145-150`.

- Strength: must. Class: wire.
- Check idea: Send an IPv6 datagram and inspect the MAC frame type.

### RFC4944-LINK-2

**Multicast broadcast frames carry the destination PAN identifier.**

> 1.  A destination PAN identifier is included in the frame, and it
> MUST match the PAN ID of the link in question.

— §3, `rfc4944.txt:217-218`.

- Strength: must. Class: wire.
- Check idea: Send multicast traffic and check that the destination PAN matches the link.

### RFC4944-LINK-3

**Multicast broadcast frames use short destination address 0xffff.**

> 2.  A short destination address is included in the frame, and it MUST
> match the broadcast address (0xffff).

— §3, `rfc4944.txt:220-221`.

- Strength: must. Class: wire.
- Check idea: Check the destination addressing mode and broadcast address of multicast traffic.

### RFC4944-MTU-1

**The IPv6 link MTU is 1280 octets.**

> The MTU size for IPv6 packets over IEEE 802.15.4 is 1280 octets.
> However, a full IPv6 packet does not fit in an IEEE 802.15.4 frame.
> 802.15.4 protocol data units have different sizes depending on how
> much overhead is present [ieee802.15.4].  Starting from a maximum
> physical layer packet size of 127 octets (aMaxPHYPacketSize) and a
> maximum frame overhead of 25 (aMaxFrameOverhead), the resultant
> maximum frame size at the media access control layer is 102 octets.
> Link-layer security imposes further overhead, which in the maximum
> case (21 octets of overhead in the AES-CCM-128 case, versus 9 and 13
> for AES-CCM-32 and AES-CCM-64, respectively) leaves only 81 octets
> available.  This is obviously far below the minimum IPv6 packet size
> of 1280 octets, and in keeping with Section 5 of the IPv6
> specification [RFC2460], a fragmention and reassembly adaptation
> layer must be provided at the layer below IP.  Such a layer is
> defined below in Section 5.

— §4, `rfc4944.txt:241-255`.

- Strength: description. Class: end-to-end.
- Check idea: Transfer a 1280-octet IPv6 packet over multiple link frames and verify complete delivery.

### RFC4944-DISPATCH-1

**Stacked headers follow the prescribed mesh, broadcast, fragmentation order.**

> When more than one LoWPAN header is used in the same packet, they
> MUST appear in the following order:
>
> Mesh Addressing Header
>
> Broadcast Header
>
> Fragmentation Header

— §5, `rfc4944.txt:358-365`.

- Strength: must. Class: encoding.
- Check idea: Encode combined headers and verify the order listed in the source, including the broadcast header when present.

### RFC4944-DISPATCH-2

**Protocol datagrams have a valid LoWPAN encapsulation header.**

> All protocol datagrams (e.g., IPv6, compressed IPv6 headers, etc.)
> SHALL be preceded by one of the valid LoWPAN encapsulation headers,
> examples of which are given above.  This permits uniform software
> treatment of datagrams without regard to the mode of their
> transmission.

— §5, `rfc4944.txt:367-371`.

- Strength: shall. Class: wire.
- Check idea: Inspect uncompressed and compressed IPv6 transmissions for their encapsulation dispatch.

### RFC4944-MESH-1

**The V bit identifies the originator address width.**

> V: This 1-bit field SHALL be zero if the Originator (or "Very first")
> Address is an IEEE extended 64-bit address (EUI-64), or 1 if it is
> a short 16-bit addresses.

— §5.2, `rfc4944.txt:526-528`.

- Strength: shall. Class: encoding.
- Check idea: Encode 16-bit and 64-bit originators and check V and the address length.

### RFC4944-MESH-2

**The F bit identifies the final destination address width.**

> F: This 1-bit field SHALL be zero if the Final Destination Address is
> an IEEE extended 64-bit address (EUI-64), or 1 if it is a short
> 16-bit addresses.

— §5.2, `rfc4944.txt:530-532`.

- Strength: shall. Class: encoding.
- Check idea: Encode both destination widths and check F and the address length.

### RFC4944-MESH-3

**Each forwarder decrements Hops Left and stops forwarding at zero.**

> Hops Left:  This 4-bit field SHALL be decremented by each forwarding
> node before sending this packet towards its next hop.  The packet
> is not forwarded any further if Hops Left is decremented to zero.
> The value 0xF is reserved and signifies an 8-bit Deep Hops Left
> field immediately following, and allows a source node to specify a
> hop limit greater than 14 hops.

— §5.2, `rfc4944.txt:534-539`.

- Strength: shall; description. Class: wire.
- Check idea: Inject mesh packets with Hops Left 1 and 2 and observe forwarding and decrement behavior.

### RFC4944-FRAG-1

**Oversized datagrams use fragmentation, with eight-octet alignment except for the last fragment.**

> If an entire payload (e.g., IPv6) datagram fits within a single
> 802.15.4 frame, it is unfragmented and the LoWPAN encapsulation
> should not contain a fragmentation header.  If the datagram does not
> fit within a single IEEE 802.15.4 frame, it SHALL be broken into link
> fragments.  As the fragment offset can only express multiples of
> eight bytes, all link fragments for a datagram except the last one
> MUST be multiples of eight bytes in length.  The first link fragment
> SHALL contain the first fragment header as defined below.

— §5.3, `rfc4944.txt:569-576`.

- Strength: shall; must. Class: wire.
- Check idea: Use datagrams just below and above frame capacity; verify non-final payload alignment and the first-fragment header.
- Scope: original RFC 4944 fragmentation. [RFC 8931](../rfc8931/catalog.md#rfc8931-frag-3) defines a distinct recoverable format; do not apply these layout rules to RFRAG.

### RFC4944-FRAG-2

**Subsequent fragments carry the subsequent-fragment header.**

> The second and subsequent link fragments (up to and including the
> last) SHALL contain a fragmentation header that conforms to the
> format shown below.

— §5.3, `rfc4944.txt:586-588`.

- Strength: shall. Class: encoding.
- Check idea: Inspect FRAGN headers and their offsets for every fragment after the first.
- Scope: original RFC 4944 fragmentation. [RFC 8931](../rfc8931/catalog.md#rfc8931-frag-3) defines a distinct recoverable format; do not apply these layout rules to RFRAG.

### RFC4944-FRAG-3

**The fragment datagram size describes the uncompressed IP packet.**

> datagram_size:  This 11-bit field encodes the size of the entire IP
> packet before link-layer fragmentation (but after IP layer
> fragmentation).  The value of datagram_size SHALL be the same for
> all link-layer fragments of an IP packet.  For IPv6, this SHALL be
> 40 octets (the size of the uncompressed IPv6 header) more than the
> value of Payload Length in the IPv6 header [RFC2460] of the
> packet.  Note that this packet may already be fragmented by hosts
> involved in the communication, i.e., this field needs to encode a
> maximum length of 1280 octets (the IEEE 802.15.4 link MTU, as
> defined in this document).

— §5.3, `rfc4944.txt:600-609`.

- Strength: shall. Class: encoding.
- Check idea: Compress and fragment a known IPv6 datagram; compare the advertised size with its uncompressed length.
- Scope: original RFC 4944 fragmentation. [RFC 8931](../rfc8931/catalog.md#rfc8931-frag-3) defines a distinct recoverable format; do not apply these layout rules to RFRAG.

### RFC4944-FRAG-4

**All fragments share a tag, incremented for successive datagrams with wrap at 65535.**

> datagram_tag:  The value of datagram_tag (datagram tag) SHALL be the
> same for all link fragments of a payload (e.g., IPv6) datagram.
> The sender SHALL increment datagram_tag for successive, fragmented
> datagrams.  The incremented value of datagram_tag SHALL wrap from
> 65535 back to zero.  This field is 16 bits long, and its initial
> value is not defined.

— §5.3, `rfc4944.txt:629-634`.

- Strength: shall. Class: wire.
- Check idea: Send successive fragmented datagrams through tag wrap and check per-datagram consistency.
- Scope: original RFC 4944 fragmentation. [RFC 8931](../rfc8931/catalog.md#rfc8931-frag-3) defines a distinct recoverable format; do not apply these layout rules to RFRAG.

### RFC4944-FRAG-5

**Subsequent-fragment offsets count eight-octet units from the start of the datagram.**

> datagram_offset:  This field is present only in the second and
> subsequent link fragments and SHALL specify the offset, in
> increments of 8 octets, of the fragment from the beginning of the
> payload datagram.  The first octet of the datagram (e.g., the
> start of the IPv6 header) has an offset of zero; the implicit
> value of datagram_offset in the first link fragment is zero.  This
> field is 8 bits long.

— §5.3, `rfc4944.txt:636-642`.

- Strength: shall. Class: encoding.
- Check idea: Use a known payload and verify every encoded offset against its position in the uncompressed datagram.
- Scope: original RFC 4944 fragmentation. [RFC 8931](../rfc8931/catalog.md#rfc8931-frag-3) defines a distinct recoverable format; do not apply these layout rules to RFRAG.

### RFC4944-REASM-1

**Reassembly identifies a datagram by source, destination, size and tag.**

> The recipient of link fragments SHALL use (1) the sender's 802.15.4
> source address (or the Originator Address if a Mesh Addressing field
> is present), (2) the destination's 802.15.4 address (or the Final
> Destination address if a Mesh Addressing field is present), (3)
> datagram_size, and (4) datagram_tag to identify all the link
> fragments that belong to a given datagram.

— §5.3, `rfc4944.txt:644-649`.

- Strength: shall. Class: internal.
- Check idea: Interleave fragments differing in each key component and check that reassembly state remains separate.
- Scope: original RFC 4944 fragmentation. [RFC 8931](../rfc8931/catalog.md#rfc8931-frag-3) defines a distinct recoverable format; do not apply these layout rules to RFRAG.

### RFC4944-REASM-2

**An overlapping fragment with a different size or offset discards accumulated fragments.**

> If a link fragment that overlaps another fragment is received, as
> identified above, and differs in either the size or datagram_offset
> of the overlapped fragment, the fragment(s) already accumulated in
> the reassembly buffer SHALL be discarded.  A fresh reassembly may be
> commenced with the most recently received link fragment.  Fragment
> overlap is determined by the combination of datagram_offset from the
> encapsulation header and "Frame Length" from the 802.15.4 Physical
> Layer Protocol Data Unit (PPDU) packet header.

— §5.3, `rfc4944.txt:660-667`.

- Strength: shall. Class: end-to-end.
- Check idea: Inject the specified overlap and check that the old incomplete datagram is not delivered; allow the fresh reassembly permitted by the source.
- Scope: original RFC 4944 fragmentation. [RFC 8931](../rfc8931/catalog.md#rfc8931-frag-3) defines a distinct recoverable format; do not apply these layout rules to RFRAG.

### RFC4944-REASM-3

**Incomplete reassembly is discarded on timeout, with a maximum timeout of 60 seconds.**

> When this time expires, if the entire packet has not been
> reassembled, the existing fragments MUST be discarded and the
> reassembly state MUST be flushed.  The reassembly timeout MUST be set
> to a maximum of 60 seconds (this is also the timeout in the IPv6
> reassembly procedure [RFC2460]).

— §5.3, `rfc4944.txt:683-687`.

- Strength: must. Class: internal.
- Check idea: Withhold a fragment and verify that both buffered data and reassembly state expire within the bound.
- Scope: original RFC 4944 fragmentation. [RFC 8931](../rfc8931/catalog.md#rfc8931-frag-3) defines a distinct recoverable format; do not apply these layout rules to RFRAG.

### RFC4944-ADDR-1

**Stateless autoconfiguration uses a 64-bit prefix.**

> An IPv6 address prefix used for stateless autoconfiguration [RFC4862]
> of an IEEE 802.15.4 interface MUST have a length of 64 bits.

— §6, `rfc4944.txt:725-726`.

- Strength: must. Class: internal.
- Check idea: Configure a valid 64-bit prefix and check address formation; exercise unsupported prefix lengths separately.

### RFC4944-LEGACY-1

**Legacy HC1 carries Hop Limit after the encoding fields.**

> The non-compressed IPv6 field that MUST be always present is the Hop
> Limit (8 bits).  This field MUST always follow the encoding fields
> (e.g., "HC1 encoding" as shown in Figure 9), perhaps including other
> future encoding fields).  Other non-compressed fields MUST follow the
> Hop Limit as implied by the "HC1 encoding" in the exact same order as
> shown above (Section 10.1): source address prefix (64 bits) and/or
> interface identifier (64 bits), destination address prefix (64 bits)
> and/or interface identifier (64 bits), Traffic Class (8 bits), Flow
> Label (20 bits) and Next Header (8 bits).  The actual next header
> (e.g., UDP, TCP, ICMP, etc) follows the non-compressed fields.

— §10.3.1, `rfc4944.txt:1142-1151`.

- Strength: must. Class: encoding.
- Check idea: For an explicitly enabled legacy HC1 decoder, check Hop Limit position and the ordering of remaining inline fields.
- Overridden by: [RFC6282-LEGACY-1](../rfc6282/catalog.md#rfc6282-legacy-1). For modern transmission; the original layout still governs an optional legacy decoder.

### RFC4944-MESH-4

**Transmission through a mesh forwarder includes originator and ultimate destination addresses.**

> If a node wishes to use a default mesh forwarder to deliver a packet
> (i.e., because it does not have direct reachability to the
> destination), it MUST include a Mesh Addressing header with the
> originator's link-layer address set to its own, and the final
> destination's link-layer address set to the packet's ultimate
> destination.  It sets the source address in the 802.15.4 header to
> its own link-layer address, and puts the forwarder's link-layer
> address in the 802.15.4 header's destination address field.  Finally,
> it transmits the packet.

— §11, `rfc4944.txt:1201-1209`.

- Strength: must. Class: wire.
- Check idea: Send to a non-neighbor through a mesh forwarder and inspect both mesh addresses.

## Areas outside this selection

This catalog selects link carriage, encapsulation, mesh forwarding, basic fragmentation, reassembly and prefix-length rules. Full address derivation and mapping (§§6–9), legacy HC1/HC2 formats (§10), broadcast sequence handling (§11.1), registry administration (§12), security analysis (§13) and the informative mesh alternatives (Appendix A) are not exhaustively cataloged. The one legacy entry exists to distinguish the old format from the modern baseline.
