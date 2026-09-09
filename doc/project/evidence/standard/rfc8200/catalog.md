# RFC 8200 (IPv6) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC8200-*` · **Stands on:** [standards.md](../../protocol/ipv6/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow, for one document of
the in-scope set: RFC 8200. It lists statements of that document that a test can check.
The catalog comes from the RFC text only. It contains no simulation model names and no code
references — that mapping happens in later steps.

Source, cached in this folder:

- `rfc8200.txt` — Internet Protocol, Version 6 (IPv6) Specification, July 2017. Downloaded
  2026-09-09 from <https://www.rfc-editor.org/rfc/rfc8200.txt>.

RFC 8200 delegates error reports to ICMPv6. Those statements belong to RFC 4443 and live in
[`rfc4443/catalog.md`](../rfc4443/catalog.md). The documents of the in-scope set, their
relatives, and the override rows are in
[`standards.md`](../../protocol/ipv6/standards.md). A feature that spans both documents is
in [`features.md`](../../protocol/ipv6/features.md).

Quotes are verbatim. A reference such as `rfc8200.txt:348` points to a line of the cached
file in this folder.

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`ipv6/coverage.md`](../../model/ipv6/coverage.md). Keeping it out is deliberate: this
catalog states what the standard says, so a new test or a new run must never force an edit
here.

## Index

| ID | Statement |
| --- | --- |
| [RFC8200-HDR-1](#rfc8200-hdr-1) | The version field is 6. |
| [RFC8200-HDR-2](#rfc8200-hdr-2) | The payload length counts everything after the IPv6 header, extension headers included. |
| [RFC8200-HDR-3](#rfc8200-hdr-3) | The next header field names the header that follows. |
| [RFC8200-HDR-4](#rfc8200-hdr-4) | The source and destination fields carry the originator and the intended recipient. |
| [RFC8200-DLV-1](#rfc8200-dlv-1) | The destination demultiplexes on the next header field to the upper-layer header. |
| [RFC8200-HL-1](#rfc8200-hl-1) | Each forwarding node decrements the hop limit by 1. |
| [RFC8200-HL-2](#rfc8200-hl-2) | A forwarding node discards a packet whose hop limit was zero or reaches zero. |
| [RFC8200-HL-3](#rfc8200-hl-3) | The destination should not discard a packet for a hop limit of zero. |
| [RFC8200-EXT-1](#rfc8200-ext-1) | Nodes en route do not process, insert, or delete extension headers. |
| [RFC8200-FRAG-1](#rfc8200-frag-1) | Only source nodes fragment; routers do not. |
| [RFC8200-FRAG-2](#rfc8200-frag-2) | The identification differs from any recent fragmented packet of the same source and destination. |
| [RFC8200-FRAG-3](#rfc8200-frag-3) | Fragments fit the path MTU; each but the last is a multiple of 8 octets. |
| [RFC8200-FRAG-4](#rfc8200-frag-4) | The first fragment: per-fragment headers with next header 44, a fragment header with offset 0 and M = 1, the upper-layer header, the first piece. |
| [RFC8200-FRAG-5](#rfc8200-frag-5) | A later fragment: the same headers, an offset in 8-octet units, M = 0 on the last, the same identification. |
| [RFC8200-FRAG-6](#rfc8200-frag-6) | Fragments of one packet do not overlap. |
| [RFC8200-REASM-1](#rfc8200-reasm-1) | A packet is reassembled only from fragments with the same source, destination, and identification. |
| [RFC8200-REASM-2](#rfc8200-reasm-2) | The reassembled packet: the per-fragment headers, the pieces in offset order, no fragment header, the payload length recomputed. |
| [RFC8200-REASM-3](#rfc8200-reasm-3) | Reassembly is abandoned after 60 seconds; Time Exceeded code 1 if the first fragment arrived. |
| [RFC8200-REASM-4](#rfc8200-reasm-4) | A non-final fragment whose length is not a multiple of 8 is discarded, with Parameter Problem code 0. |
| [RFC8200-REASM-5](#rfc8200-reasm-5) | Overlapping fragments abandon the reassembly. |
| [RFC8200-REASM-6](#rfc8200-reasm-6) | A fragment with offset 0 and M = 0 is processed as a whole packet. |
| [RFC8200-MTU-1](#rfc8200-mtu-1) | Every link has an MTU of at least 1280 octets. |
| [RFC8200-MTU-2](#rfc8200-mtu-2) | A node accepts packets as large as the MTU of each attached link. |
| [RFC8200-MTU-3](#rfc8200-mtu-3) | A node should discover the path MTU, or send no more than 1280 octets. |
| [RFC8200-MTU-4](#rfc8200-mtu-4) | A node accepts a fragmented packet that reassembles to 1500 octets. |
| [RFC8200-MTU-5](#rfc8200-mtu-5) | Fragmented packets above 1500 octets should not be sent without assurance. |
| [RFC8200-CKSUM-1](#rfc8200-cksum-1) | The UDP checksum is mandatory over IPv6; a zero checksum is discarded. |
| [RFC8200-CKSUM-2](#rfc8200-cksum-2) | The ICMPv6 checksum covers a pseudo-header. |

## How to read an entry

- **Strength** — the word the RFC uses: `must`, `must not`, `should`, `should not`, `may`,
  or `description` for normative prose without a keyword. RFC 8200 writes its rules in
  lowercase and cites no RFC 2119; the words carry their plain meaning.
- **Class** — how a test can observe the statement:
  - `wire` — fields of packets on a link between two nodes.
  - `end-to-end` — what the destination accepts and delivers upward.
  - `error-signal` — an ICMPv6 message that reports a failure.
  - `internal` — state inside a module; not visible from outside.
  - `encoding` — the exact bit layout of a field; a serializer concern.
- **Overridden by** — appears only when a later document of the in-scope set changes the
  statement. No entry carries the field today, because the in-scope set is RFC 8200 and
  RFC 4443 alone.

## Header format

### RFC8200-HDR-1

**The version field is 6.**

> "Version             4-bit Internet Protocol version number = 6." — §3,
> `rfc8200.txt:321`

- Strength: description. Class: wire.
- Check idea: every packet on every link carries version 6.

### RFC8200-HDR-2

**The payload length counts everything after the IPv6 header, extension headers included.**

> "Payload Length      16-bit unsigned integer.  Length of the IPv6 payload, i.e., the rest
> of the packet following this IPv6 header, in octets.  (Note that any extension headers
> (see Section 4) present are considered part of the payload, i.e., included in the length
> count.)" — §3, `rfc8200.txt:327-332`

- Strength: description. Class: wire.
- Check idea: for a UDP datagram with a known data size and no extension headers, the
  payload length equals 8 plus the size of the data. For a fragment, it equals the fragment
  header plus the piece.

### RFC8200-HDR-3

**The next header field names the header that follows the IPv6 header.**

> "Next Header         8-bit selector.  Identifies the type of header immediately following
> the IPv6 header.  Uses the same values as the IPv4 Protocol field [IANA-PN]." — §3,
> `rfc8200.txt:343-346`

- Strength: description. Class: wire.
- Check idea: a packet that carries UDP shows 17; a fragment shows 44, and its fragment
  header shows 17.

### RFC8200-HDR-4

**The source and destination address fields carry the originator and the intended
recipient of the packet.**

> "Source Address      128-bit address of the originator of the packet.  See [RFC4291]." —
> §3, `rfc8200.txt:357-358`
> "Destination Address 128-bit address of the intended recipient of the packet (possibly
> not the ultimate recipient, if a Routing header is present).  See [RFC4291] and Section
> 4.4." — §3, `rfc8200.txt:360-363`

- Strength: description. Class: wire.
- Check idea: a packet sent from host A to host B carries host A's address as source and
  host B's as destination on every link, and a gateway does not rewrite them.

### RFC8200-DLV-1

**At the destination, normal demultiplexing on the next header field invokes the module
for the upper-layer header.**

> "At the destination node, normal demultiplexing on the Next Header field of the IPv6
> header invokes the module to process the first extension header, or the upper-layer
> header if no extension header is present." — §4, `rfc8200.txt:455-458`

- Strength: description. Class: end-to-end.
- Check idea: a packet with next header 17 is handed to UDP at the destination, and UDP
  hands the data to the program.

## Hop limit

### RFC8200-HL-1

**Each node that forwards the packet decrements the hop limit by 1.**

> "Hop Limit           8-bit unsigned integer.  Decremented by 1 by each node that forwards
> the packet." — §3, `rfc8200.txt:348-349`

- Strength: description. Class: wire.
- Check idea: compare the hop limit on the link before a gateway with the hop limit on the
  link after it. The value decreases by exactly 1.

### RFC8200-HL-2

**When forwarding, a node discards a packet whose hop limit was zero on receipt or reaches
zero when decremented.**

> "When forwarding, the packet is discarded if Hop Limit was zero when received or is
> decremented to zero." — §3, `rfc8200.txt:349-352`

- Strength: description. Class: wire (absence after the gateway) plus error-signal
  ([RFC4443-TE-1](../rfc4443/catalog.md#rfc4443-te-1)).
- Check idea: send a packet whose hop limit is 1. Nothing crosses the link after the
  gateway, and the destination never receives it.

### RFC8200-HL-3

**A destination node should not discard a packet because its hop limit is zero.**

> "A node that is the destination of a packet should not discard a packet with Hop Limit
> equal to zero; it should process the packet normally." — §3, `rfc8200.txt:352-355`

- Strength: should. Class: end-to-end.
- Check idea: a packet whose hop limit is exactly the hop count of the path arrives at the
  destination with hop limit 1 and is delivered; the rule that discards is a forwarding
  rule. A packet that arrives with hop limit 0 needs a crafted sender and is a later pass.

## Extension headers

### RFC8200-EXT-1

**Nodes along the path do not process, insert, or delete extension headers, except the
hop-by-hop options header.**

> "Extension headers (except for the Hop-by-Hop Options header) are not processed,
> inserted, or deleted by any node along a packet's delivery path, until the packet reaches
> the node (or each of the set of nodes, in the case of multicast) identified in the
> Destination Address field of the IPv6 header." — §4, `rfc8200.txt:424-428`

- Strength: description. Class: wire.
- Check idea: a fragment header that leaves the source appears unchanged on the link after
  the gateway.

## Fragmentation

### RFC8200-FRAG-1

**Fragmentation is performed by source nodes only, not by routers along the path.**

> "(Note: unlike IPv4, fragmentation in IPv6 is performed only by source nodes, not by
> routers along a packet's delivery path -- see Section 5.)" — §4.5,
> `rfc8200.txt:817-819`

- Strength: description. Class: wire (absence of fragments after a gateway) plus
  error-signal ([RFC4443-PTB-1](../rfc4443/catalog.md#rfc4443-ptb-1)).
- Check idea: a packet larger than the MTU of the link after the gateway produces no
  fragment on that link and no copy of itself; the gateway reports instead.

### RFC8200-FRAG-2

**The identification of a fragmented packet differs from that of any packet fragmented
recently with the same source and destination.**

> "For every packet that is to be fragmented, the source node generates an Identification
> value.  The Identification must be different than that of any other fragmented packet
> sent recently* with the same Source Address and Destination Address." — §4.5,
> `rfc8200.txt:864-867`

- Strength: must. Class: wire.
- Check idea: two fragmented packets of one flow, sent close together, carry two different
  identifications in their fragment headers.

### RFC8200-FRAG-3

**The fragments fit the path MTU, and every fragment but the last is a multiple of 8
octets long.**

> "The Fragmentable Part of the original packet is divided into fragments.  The lengths of
> the fragments must be chosen such that the resulting fragment packets fit within the MTU
> of the path to the packet's destination(s).  Each complete fragment, except possibly the
> last ("rightmost") one, is an integer multiple of 8 octets long." — §4.5,
> `rfc8200.txt:933-937`

- Strength: must (fit the MTU); description (the multiple of 8). Class: wire.
- Check idea: with a known original size and a known link MTU at the source, every fragment
  packet is at most the MTU long, and the piece of every fragment but the last is a
  multiple of 8.

### RFC8200-FRAG-4

**The first fragment packet consists of the per-fragment headers with the payload length
of the fragment and next header 44, a fragment header with the original next header,
offset 0, M = 1 and the identification, then the upper-layer header and the first piece.**

> "The first fragment packet is composed of: (1) The Per-Fragment headers of the original
> packet, with the Payload Length of the original IPv6 header changed to contain the
> length of this fragment packet only (excluding the length of the IPv6 header itself), and
> the Next Header field of the last header of the Per-Fragment headers changed to 44. (2) A
> Fragment header containing: The Next Header value that identifies the first header after
> the Per-Fragment headers of the original packet. A Fragment Offset containing the offset
> of the fragment, in 8-octet units, relative to the start of the Fragmentable Part of the
> original packet.  The Fragment Offset of the first ("leftmost") fragment is 0. An M flag
> value of 1 as this is the first fragment. [...] The Identification value generated for the
> original packet. (3) Extension headers, if any, and the Upper-Layer header.  These
> headers must be in the first fragment. [...] (4) The first fragment." — §4.5,
> `rfc8200.txt:988-1023`

- Strength: description, with one must (the upper-layer header is in the first fragment).
  Class: wire.
- Check idea: the first fragment on the source's link shows next header 44, a payload
  length of 8 plus the piece, a fragment header with next header 17, offset 0 and M = 1,
  and the UDP header behind it.

### RFC8200-FRAG-5

**A subsequent fragment packet consists of the per-fragment headers, a fragment header with
the offset in 8-octet units, M = 0 on the last fragment, and the same identification, then
the piece.**

> "The subsequent fragment packets are composed of: (1) The Per-Fragment headers of the
> original packet, with the Payload Length of the original IPv6 header changed to contain
> the length of this fragment packet only (excluding the length of the IPv6 header itself),
> and the Next Header field of the last header of the Per-Fragment headers changed to 44.
> (2) A Fragment header containing: [...] A Fragment Offset containing the offset of the
> fragment, in 8-octet units, relative to the start of the Fragmentable Part of the original
> packet. An M flag value of 0 if the fragment is the last ("rightmost") one, else an M
> flag value of 1. The Identification value generated for the original packet. (3) The
> fragment itself." — §4.5, `rfc8200.txt:1025-1048`

- Strength: description. Class: wire.
- Check idea: the last fragment shows the identification of the first, an offset equal to
  the length of the earlier pieces in 8-octet units, M = 0, and a payload length of 8 plus
  its piece.

### RFC8200-FRAG-6

**Fragments created from one packet do not overlap.**

> "Fragments must not be created that overlap with any other fragments created from the
> original packet." — §4.5, `rfc8200.txt:1050-1051`

- Strength: must not. Class: wire.
- Check idea: the offsets and piece lengths of the fragments tile the fragmentable part
  without a gap and without an overlap; the pieces add up to its length.

## Reassembly

### RFC8200-REASM-1

**A packet is reassembled only from fragments that share its source, destination, and
identification.**

> "An original packet is reassembled only from fragment packets that have the same Source
> Address, Destination Address, and Fragment Identification." — §4.5,
> `rfc8200.txt:1083-1085`

- Strength: description. Class: end-to-end.
- Check idea: the fragments of one packet are delivered as that packet; fragments of two
  packets do not mix.

### RFC8200-REASM-2

**The reassembled packet consists of the per-fragment headers of the first fragment, the
pieces placed by offset, without the fragment header, with the next header and the payload
length recomputed.**

> "The Per-Fragment headers of the reassembled packet consists of all headers up to, but
> not including, the Fragment header of the first fragment packet (that is, the packet
> whose Fragment Offset is zero), with the following two changes: The Next Header field of
> the last header of the Per-Fragment headers is obtained from the Next Header field of
> the first fragment's Fragment header. The Payload Length of the reassembled packet is
> computed from the length of the Per-Fragment headers and the length and offset of the
> last fragment." — §4.5, `rfc8200.txt:1087-1098`
> "The Fragment header is not present in the final, reassembled packet." —
> `rfc8200.txt:1130-1131`

- Strength: description. Class: end-to-end.
- Check idea: the destination hands UDP a datagram whose length equals the original
  fragmentable part, and the fragment header is gone.

### RFC8200-REASM-3

**Reassembly is abandoned after 60 seconds, and Time Exceeded code 1 is sent if the first
fragment arrived.**

> "If insufficient fragments are received to complete reassembly of a packet within 60
> seconds of the reception of the first-arriving fragment of that packet, reassembly of
> that packet must be abandoned and all the fragments that have been received for that
> packet must be discarded.  If the first fragment (i.e., the one with a Fragment Offset of
> zero) has been received, an ICMP Time Exceeded -- Fragment Reassembly Time Exceeded
> message should be sent to the source of that fragment." — §4.5, `rfc8200.txt:1145-1152`

- Strength: must (abandon and discard); should (the report). Class: internal with an
  error-signal effect.
- Note: a timer; level 4 by the level table of the guide.

### RFC8200-REASM-4

**A non-final fragment whose length is not a multiple of 8 octets is discarded, and
Parameter Problem code 0 should be sent.**

> "If the length of a fragment, as derived from the fragment packet's Payload Length field,
> is not a multiple of 8 octets and the M flag of that fragment is 1, then that fragment
> must be discarded and an ICMP Parameter Problem, Code 0, message should be sent to the
> source of the fragment, pointing to the Payload Length field of the fragment packet." —
> §4.5, `rfc8200.txt:1154-1159`

- Strength: must (discard); should (the report). Class: end-to-end (absence) plus
  error-signal.
- Note: needs a crafted fragment; level 3.

### RFC8200-REASM-5

**Overlapping fragments abandon the reassembly.**

> "If any of the fragments being reassembled overlap with any other fragments being
> reassembled for the same packet, reassembly of that packet must be abandoned and all the
> [fragments discarded]" — §4.5, `rfc8200.txt:1183-1185`; the sentence continues on the
> next page of the text.

- Strength: must. Class: end-to-end (absence).
- Note: needs crafted fragments; level 3.

### RFC8200-REASM-6

**A fragment with offset 0 and M = 0 is a whole packet and is processed as reassembled.**

> "If the fragment is a whole datagram (that is, both the Fragment Offset field and the M
> flag are zero), then it does not need any further reassembly and should be processed as a
> fully reassembled packet (i.e., updating Next Header, adjust Payload Length, removing the
> Fragment header, etc.).  Any other fragments that match this packet (i.e., the same IPv6
> Source Address, IPv6 Destination Address, and Fragment Identification) should be
> processed independently." — §4.5, `rfc8200.txt:1133-1140`

- Strength: should. Class: end-to-end.
- Note: needs a crafted fragment header; level 3.

## Packet size

### RFC8200-MTU-1

**Every link has an MTU of at least 1280 octets.**

> "IPv6 requires that every link in the Internet have an MTU of 1280 octets or greater.
> This is known as the IPv6 minimum link MTU.  On any link that cannot convey a 1280-octet
> packet in one piece, link-specific fragmentation and reassembly must be provided at a
> layer below IPv6." — §5, `rfc8200.txt:1378-1382`

- Strength: description (a property of links); must (for links below it). Class: wire,
  as a scenario constraint.
- Note: no check chooses an MTU below 1280 for an IPv6 link. This is a rule for the mockup
  rather than for a node.

### RFC8200-MTU-2

**A node accepts packets as large as the MTU of each link it is attached to.**

> "From each link to which a node is directly attached, the node must be able to accept
> packets as large as that link's MTU." — §5, `rfc8200.txt:1390-1391`

- Strength: must. Class: end-to-end.
- Check idea: a packet as large as the link MTU, 1500 octets on Ethernet, crosses both
  links whole and is delivered.

### RFC8200-MTU-3

**A node should discover the path MTU; a minimal node may send no more than 1280 octets
instead.**

> "It is strongly recommended that IPv6 nodes implement Path MTU Discovery [RFC8201], in
> order to discover and take advantage of path MTUs greater than 1280 octets.  However, a
> minimal IPv6 implementation (e.g., in a boot ROM) may simply restrict itself to sending
> packets no larger than 1280 octets, and omit implementation of Path MTU Discovery." —
> §5, `rfc8200.txt:1393-1398`

- Strength: should (strongly recommended); may. Class: wire.
- Note: path MTU discovery is RFC 8201 and level 4. The check of this pass establishes
  only the message it starts from ([RFC4443-PTB-1](../rfc4443/catalog.md#rfc4443-ptb-1)).

### RFC8200-MTU-4

**A node accepts a fragmented packet that reassembles to 1500 octets.**

> "A node must be able to accept a fragmented packet that, after reassembly, is as large as
> 1500 octets.  A node is permitted to accept fragmented packets that reassemble to more
> than 1500 octets." — §5, `rfc8200.txt:1414-1416`

- Strength: must. Class: end-to-end.
- Check idea: a 1500-octet packet fragmented at the source is delivered whole at the
  destination.

### RFC8200-MTU-5

**A protocol that relies on fragmentation should not send more than 1500 octets without
assurance that the destination reassembles it.**

> "An upper-layer protocol or application that depends on IPv6 fragmentation to send
> packets larger than the MTU of a path should not send packets larger than 1500 octets
> unless it has assurance that the destination is capable of reassembling packets of that
> larger size." — §5, `rfc8200.txt:1417-1421`

- Strength: should not. Class: wire.
- Note: a rule for the upper layer; the checks of this pass stay at 1500 octets.

## Upper-layer checksums

### RFC8200-CKSUM-1

**A node that originates a UDP packet over IPv6 computes the UDP checksum, writes FFFF for
a zero result, and a receiver discards a UDP packet with a zero checksum.**

> "Unlike IPv4, the default behavior when UDP packets are originated by an IPv6 node is
> that the UDP checksum is not optional.  That is, whenever originating a UDP packet, an
> IPv6 node must compute a UDP checksum over the packet and the pseudo-header, and, if that
> computation yields a result of zero, it must be changed to hex FFFF for placement in the
> UDP header.  IPv6 receivers must discard UDP packets containing a zero checksum and
> should log the error." — §8.1, `rfc8200.txt:1530-1537`

- Strength: must (compute; discard zero); should (log). Class: wire (the field is never
  zero) plus end-to-end (the discard, which needs a crafted datagram; level 3).
- Check idea: every UDP datagram a node originates over IPv6 carries a non-zero checksum.

### RFC8200-CKSUM-2

**The ICMPv6 checksum covers the IPv6 pseudo-header with next header 58.**

> "The IPv6 version of ICMP [RFC4443] includes the above pseudo-header in its checksum
> computation; this is a change from the IPv4 version of ICMP, which does not include a
> pseudo-header in its checksum. [...] The Next Header field in the pseudo-header for ICMP
> contains the value 58, which identifies the IPv6 version of ICMP." — §8.1,
> `rfc8200.txt:1546-1553`

- Strength: description. Class: encoding.
- Note: a serializer concern; a unit test is its home.

## Out of scope in this catalog

The extension headers other than the fragment header, their order and their options (§4.1
to §4.4, §4.6 to §4.8), the flow label (§6, RFC 6437), the traffic class (§7, RFC 2474 and
RFC 3168), the maximum packet lifetime note (§8.2, which states that no lifetime is
enforced), the maximum upper-layer payload size (§8.3, a rule for the transport protocol),
the response to routing headers (§8.4), the jumbo payload (RFC 2675), and the security
considerations. Each becomes a catalog entry in a later pass.

The error reports that RFC 8200 delegates to ICMPv6 are in
[`rfc4443/catalog.md`](../rfc4443/catalog.md).
