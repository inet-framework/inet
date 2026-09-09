# IPv6 — feature map

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `IPV6-F-*` · **Stands on:** [standards.md](standards.md), [rfc8200/catalog.md](../../standard/rfc8200/catalog.md), [rfc4443/catalog.md](../../standard/rfc4443/catalog.md), [rfc8504/catalog.md](../../standard/rfc8504/catalog.md)

Step 4 artifact of the standards test workflow. The catalogs are flat, fine-grained, and
per document. This document is the high-level view above them: the capabilities that the
in-scope set of [`standards.md`](standards.md#in-scope-set) defines, the requirement level
of each, and the cross reference into the catalogs.

The map reads in two directions:

- **Down** — which checks tell whether a feature works. A `core` check is one the feature
  cannot work without. A `supporting` check is a detail or an edge case.
- **Up** — which features the simulation model supports. The support of each feature is in
  [`coverage.md`](../../model/ipv6/coverage.md).

The feature list itself comes from the standard texts only. The comparison of the support
values against the standards that the model claims to implement is
[`conformance.md`](../../model/ipv6/conformance.md).

## Index

| ID | Feature |
| --- | --- |
| [IPV6-F-HEADER](#ipv6-f-header) | Every packet carries a version-6 header whose payload length and next header describe it correctly. |
| [IPV6-F-DELIVERY](#ipv6-f-delivery) | A packet crosses the router and reaches the addressed program with its addresses and data intact. |
| [IPV6-F-HOP-LIMIT](#ipv6-f-hop-limit) | Every forwarding node decrements the hop limit, discards a packet that reaches zero, and the destination processes what arrives. |
| [IPV6-F-SOURCE-FRAGMENTATION](#ipv6-f-source-fragmentation) | A source that must send more than the path MTU may split the packet with fragment headers of the prescribed form, never overlapping, never atomic. |
| [IPV6-F-REASSEMBLY](#ipv6-f-reassembly) | The destination reassembles the fragments of one packet, up to 1500 octets, and discards a malformed or overlapping set. |
| [IPV6-F-PACKET-TOO-BIG](#ipv6-f-packet-too-big) | A router never fragments; it discards a packet too large for the next link and reports the link MTU to the source. |
| [IPV6-F-PACKET-SIZE](#ipv6-f-packet-size) | Every link carries 1280 octets, and a node accepts packets as large as its link MTU. |
| [IPV6-F-ERROR-REPORT](#ipv6-f-error-report) | A node that discards a packet reports the failure to the source with an ICMPv6 message from the right source address. |
| [IPV6-F-UPPER-LAYER-CHECKSUM](#ipv6-f-upper-layer-checksum) | UDP over IPv6 always carries a checksum, computed over the IPv6 pseudo-header. |
| [IPV6-F-INPUT-VALIDATION](#ipv6-f-input-validation) | A node discards what it must not process: a zero UDP checksum, an unrecognized next header, an unknown ICMPv6 informational type. |
| [IPV6-F-ERROR-SUPPRESSION](#ipv6-f-error-suppression) | A node sends no ICMPv6 error about an error message, a multicast or link-layer multicast or broadcast packet, or a source that is not one node. |
| [IPV6-F-ERROR-DELIVERY](#ipv6-f-error-delivery) | A received ICMPv6 error reaches the upper-layer process that sent the quoted packet. |

The support of each feature — what the run actually showed — is **not** in this document.
It lives in the coverage ledger, [`coverage.md`](../../model/ipv6/coverage.md).

## Summary table

| Feature | Level | Sources | Core checks |
| --- | --- | --- | --- |
| [IPV6-F-HEADER](#ipv6-f-header) | mandatory | RFC 8200 §3 | RFC8200-HDR-1, HDR-2, HDR-3 |
| [IPV6-F-DELIVERY](#ipv6-f-delivery) | mandatory | RFC 8200 §3, §4 | RFC8200-HDR-4, RFC8200-DLV-1 |
| [IPV6-F-HOP-LIMIT](#ipv6-f-hop-limit) | mandatory | RFC 8200 §3; RFC 4443 §3.3 | RFC8200-HL-1, RFC8200-HL-2 |
| [IPV6-F-SOURCE-FRAGMENTATION](#ipv6-f-source-fragmentation) | optional | RFC 8200 §4.5, §5; RFC 8504 §5.1, §5.2 | RFC8200-FRAG-2, FRAG-3, FRAG-4, FRAG-5, RFC8504-NR-3 (sender half; governs FRAG-6), RFC8504-NR-4 (sender half), RFC8504-NR-8 (sender half) |
| [IPV6-F-REASSEMBLY](#ipv6-f-reassembly) | mandatory | RFC 8200 §4.5, §5; RFC 8504 §5.1 | RFC8200-REASM-1, REASM-2, REASM-4, REASM-8, RFC8200-MTU-4, RFC8504-NR-3 (receiver half; governs REASM-5) |
| [IPV6-F-PACKET-TOO-BIG](#ipv6-f-packet-too-big) | mandatory | RFC 8200 §4.5, §5; RFC 4443 §3.2 | RFC8200-FRAG-1, RFC4443-PTB-1, RFC4443-PTB-2 |
| [IPV6-F-PACKET-SIZE](#ipv6-f-packet-size) | mandatory | RFC 8200 §5 | RFC8200-MTU-2 |
| [IPV6-F-ERROR-REPORT](#ipv6-f-error-report) | mandatory | RFC 4443 §2.2, §2.4, §3.1 to §3.3 | RFC4443-TE-1, RFC4443-PTB-1, RFC4443-ERR-2, RFC4443-SRC-1 |
| [IPV6-F-UPPER-LAYER-CHECKSUM](#ipv6-f-upper-layer-checksum) | mandatory | RFC 8200 §8.1 | RFC8200-CKSUM-1 |
| [IPV6-F-INPUT-VALIDATION](#ipv6-f-input-validation) | mandatory | RFC 8200 §4, §8.1; RFC 8504 §5.2; RFC 4443 §2.4 | RFC8200-CKSUM-1 (the discard half), RFC8504-NR-6 (governs RFC8200-EXT-3), RFC4443-MPR-2 |
| [IPV6-F-ERROR-SUPPRESSION](#ipv6-f-error-suppression) | mandatory | RFC 4443 §2.4 (e) | RFC4443-MPR-4, MPR-6, MPR-7, MPR-8, MPR-9 |
| [IPV6-F-ERROR-DELIVERY](#ipv6-f-error-delivery) | mandatory | RFC 4443 §2.4 (a), (d), §3.1 to §3.3 | RFC4443-MPR-1, UL-1, UL-2, UL-3 |

Twelve features. What a run showed about each one is in
[`coverage.md`](../../model/ipv6/coverage.md).

Two levels deserve a word. The error report is `mandatory` here, unlike its IPv4
counterpart: RFC 4443 says a router "MUST" send Packet Too Big and Time Exceeded, where
RFC 792 says "may". Source fragmentation is `optional`: RFC 8200 §4.5 says a source "may"
divide the packet, and §5 lets a minimal node stay at 1280 octets instead. Its core
statements are musts and descriptions that apply **when** a node fragments, and the rule
of step 4 is applied to the mechanism, not to its conditional format rules; the entry says
so.

## IPV6-F-HEADER

**Every packet carries a version-6 header whose payload length and next header describe the
packet correctly.**

- **Sources** — RFC 8200 §3, `rfc8200.txt:321-346`.
- **Level** — mandatory (reason: only path). The header format is the protocol; every
  other feature reads its fields from this header.
- **Description** — the version is 6, the payload length counts everything after the
  40-octet header (extension headers included), and the next header names what follows.
- **Checks** — core: RFC8200-HDR-1 (version), RFC8200-HDR-2 (payload length),
  RFC8200-HDR-3 (next header).

## IPV6-F-DELIVERY

**A packet sent to a host in another network crosses the router and reaches the addressed
program, with its addresses and its data intact.**

- **Sources** — RFC 8200 §3, `rfc8200.txt:357-363` (the address fields); §4,
  `rfc8200.txt:455-458` (demultiplexing at the destination); §3, `rfc8200.txt:348-349`
  (the only mention of forwarding: "each node that forwards the packet").
- **Level** — mandatory (reason: only path). The document names no other way for a packet
  to reach a host in another network.
- **Description** — the source addresses the packet, a router forwards it by the
  destination address without rewriting the addresses, and the destination hands the
  payload to the protocol that the next header names. This is the feature the other eight
  exist to serve.
- **Checks** — core: RFC8200-HDR-4 (the addresses are the originator's and the
  recipient's, and survive the router), RFC8200-DLV-1 (the payload goes to the named
  protocol and reaches the program). Supporting: RFC8504-NR-1 (a node supports RFC 8200
  and follows its transmission rules; the whole RFC 8200 pass is its check).

## IPV6-F-HOP-LIMIT

**Every node that forwards a packet decrements its hop limit, a forwarding node discards a
packet whose hop limit reaches zero, and the destination processes a packet however small
its hop limit.**

- **Sources** — RFC 8200 §3, `rfc8200.txt:348-355`; RFC 4443 §3.3, `rfc4443.txt:605-609`.
- **Level** — mandatory (reason: only path; the words carry obligation without a
  keyword: "is discarded"). RFC 4443 states the router's half with a keyword: "it MUST
  discard the packet".
- **Description** — the field bounds the number of hops a packet can travel and stops a
  packet in a routing loop. RFC 8200 makes explicit what IPv4 left to RFC 1122: the
  discard is a forwarding rule, and a destination should process a packet even with hop
  limit zero.
- **Checks** — core: RFC8200-HL-1 (the decrement per hop), RFC8200-HL-2 (the discard when
  forwarding). Supporting: RFC8200-HL-3 (the destination does not discard; a should). The
  report that follows the discard belongs to [IPV6-F-ERROR-REPORT](#ipv6-f-error-report).

## IPV6-F-SOURCE-FRAGMENTATION

**A source node that must send a packet larger than the path MTU may divide it into
fragments that carry a fragment header of the prescribed form, and only a source node does
so.**

- **Sources** — RFC 8200 §4.5, `rfc8200.txt:816-1051`; §5, `rfc8200.txt:1407-1412`;
  RFC 8504 §5.1, `rfc8504.txt:324-352` and §5.2, `rfc8504.txt:399-409` (a node can always
  fragment; never an overlapping or an atomic fragment; the whole header chain in the first
  fragment; unpredictable identifications, a should).
- **Level** — optional (reason: keyword). "a source node may divide the packet into
  fragments" (`rfc8200.txt:859-862`); "a node may use the IPv6 Fragment header to fragment
  the packet at the source" (`rfc8200.txt:1407-1409`); a minimal node may stay at 1280
  octets instead (`rfc8200.txt:1395-1398`). The core statements below are musts and
  descriptions about the **form** of the fragments when a node does fragment; they do not
  turn the mechanism into an obligation, and the level is judged on the mechanism.
- **Description** — the source splits the fragmentable part on 8-octet boundaries, gives
  every fragment the per-fragment headers with next header 44 and a fragment header with
  the offset, the M flag and one identification, and never lets fragments overlap. Routers
  forward the fragments untouched.
- **Checks** — core: RFC8200-FRAG-2 (distinct identifications), RFC8200-FRAG-3 (fit the
  MTU, multiples of 8), RFC8200-FRAG-4 (the first fragment), RFC8200-FRAG-5 (the later
  fragments), RFC8504-NR-3 sender half (no overlap; RFC8200-FRAG-6 is the overridden base
  statement, and one check establishes both), RFC8504-NR-4 sender half (no atomic
  fragment), RFC8504-NR-8 sender half (the header chain in the first fragment; the same
  observation as item 3 of FRAG-4). Supporting: RFC8200-EXT-1 (the fragment header crosses
  the router unchanged), RFC8504-NR-2 (a node can always fragment), RFC8504-NR-5
  (unpredictable identifications, a should), RFC8200-MTU-5 (a should for the upper layer).

## IPV6-F-REASSEMBLY

**The destination reassembles the fragments that share source, destination and
identification into the original packet, and accepts a reassembled packet of 1500 octets.**

- **Sources** — RFC 8200 §4.5, `rfc8200.txt:1071-1187`; §5, `rfc8200.txt:1414-1416`;
  RFC 8504 §5.1, `rfc8504.txt:327-348` (governs the overlap and the atomic fragment) and
  §5.7.1, `rfc8504.txt:612-613`.
- **Level** — mandatory (reason: keyword). "A node must be able to accept a fragmented
  packet that, after reassembly, is as large as 1500 octets" (`rfc8200.txt:1414-1415`);
  "the entire datagram (and any constituent fragments) MUST be silently discarded"
  (`rfc8504.txt:329-330`).
- **Description** — three fields select the fragments that belong together, the offset
  places each piece, the fragment header disappears, and the payload length is recomputed.
  A set of fragments that overlap, a non-final fragment that is not a multiple of 8, or a
  fragment that would push the packet past 65,535 octets is discarded; an atomic fragment
  is a whole packet.
- **Checks** — core: RFC8200-REASM-1 (the three-field key), RFC8200-REASM-2 (the
  reassembled packet), RFC8200-MTU-4 (1500 octets), RFC8504-NR-3 receiver half (overlap;
  RFC8200-REASM-5 is the overridden base statement), RFC8200-REASM-4 (a short non-final
  fragment), RFC8200-REASM-8 (an oversized offset). Supporting: RFC8504-NR-4 receiver half
  (an atomic fragment is processed as a whole packet; RFC8200-REASM-6 is the overridden
  base statement), RFC8504-NR-8 receiver half and RFC8200-REASM-7 (a first fragment
  without the header chain; a should), RFC8200-REASM-3 (the 60-second timeout; level 4),
  RFC8504-NR-2 and RFC8504-NR-10 (a node reassembles, by the rules; implementation).

## IPV6-F-PACKET-TOO-BIG

**A router never fragments a packet; when a packet is larger than the MTU of the next link,
the router discards it and reports the MTU of that link to the source.**

- **Sources** — RFC 8200 §4.5, `rfc8200.txt:817-819`; §5, `rfc8200.txt:1393-1395`;
  RFC 4443 §3.2, `rfc4443.txt:541-551`.
- **Level** — mandatory (reason: keyword). "A Packet Too Big MUST be sent by a router in
  response to a packet that it cannot forward because the packet is larger than the MTU of
  the outgoing link" (`rfc4443.txt:548-550`).
- **Description** — this is the mechanism that replaces router fragmentation: the source
  learns the path MTU from the reports and fragments or shrinks its packets itself
  (RFC 8201, out of scope). The message is useless without the MTU value in it.
- **Checks** — core: RFC8200-FRAG-1 (no fragment and no copy of the packet on the next
  link), RFC4443-PTB-1 (the report is sent), RFC4443-PTB-2 (the report carries the
  next-hop MTU and code 0). Supporting: RFC8504-NR-11 (the report is not filtered on its
  way to the source).

## IPV6-F-PACKET-SIZE

**Every link carries a 1280-octet packet, and a node accepts packets as large as the MTU of
each link it is attached to.**

- **Sources** — RFC 8200 §5, `rfc8200.txt:1378-1398`.
- **Level** — mandatory (reason: keyword). "the node must be able to accept packets as
  large as that link's MTU" (`rfc8200.txt:1390-1391`).
- **Description** — the 1280-octet floor is what lets a source assume a minimum service
  from an unknown path; the link MTU is what a node must take in whole.
- **Checks** — core: RFC8200-MTU-2 (a packet as large as the link MTU is delivered whole).
  Supporting: RFC8200-MTU-1 (the floor, a scenario constraint), RFC8200-MTU-3 and
  RFC8504-NR-9 (path MTU discovery, a should; level 4).

## IPV6-F-ERROR-REPORT

**A node that discards a packet reports the failure to the source with an ICMPv6 message
that quotes the packet.**

- **Sources** — RFC 4443 §3.3, `rfc4443.txt:605-609` (Time Exceeded); §3.2,
  `rfc4443.txt:548-551` (Packet Too Big); §3.1, `rfc4443.txt:481-484` (Destination
  Unreachable code 4); §2.4, `rfc4443.txt:295-298` (the quote); §3.1 to §3.3 (the
  destination of a report); §2.2, `rfc4443.txt:238-260` (the source address of a report);
  §2.4 (f), `rfc4443.txt:351-355` (the rate limit); RFC 8504 §5.8, `rfc8504.txt:647`.
- **Level** — mandatory (reason: keyword). Both router reports say "MUST".
- **Description** — the report turns a silent drop into a signal that the source can act
  on. In IPv6 the two router reports are obligations, and Packet Too Big drives path MTU
  discovery.
- **Checks** — core: RFC4443-TE-1 (Time Exceeded after a hop limit discard), RFC4443-PTB-1
  (Packet Too Big after an MTU discard; core here and in
  [IPV6-F-PACKET-TOO-BIG](#ipv6-f-packet-too-big)), RFC4443-ERR-2 (the report goes to the
  source), RFC4443-SRC-1 (a host's report carries the address the packet was sent to).
  Supporting: RFC4443-DU-4 (the host's report of a closed port, a should), RFC4443-ERR-1
  (the quote), RFC4443-SRC-2 (a router's report carries one of its own addresses),
  RFC4443-MPR-10 (the rate limit; level 4), RFC4443-DU-C (no report for congestion; level
  4), RFC4443-DU-P (a point-to-point rule; a later mockup), RFC8504-NR-12 (ICMPv6 is
  supported; implementation).

## IPV6-F-UPPER-LAYER-CHECKSUM

**A UDP datagram over IPv6 always carries a checksum, computed over the IPv6
pseudo-header; ICMPv6 computes its checksum the same way.**

- **Sources** — RFC 8200 §8.1, `rfc8200.txt:1467-1553`.
- **Level** — mandatory (reason: keyword). "an IPv6 node must compute a UDP checksum over
  the packet and the pseudo-header" (`rfc8200.txt:1532-1534`).
- **Description** — IPv6 has no header checksum, so the transport checksums protect the
  addresses; that is why UDP loses its IPv4 option to omit the checksum.
- **Checks** — core: RFC8200-CKSUM-1 (the UDP checksum is never zero; its discard half is
  core in [IPV6-F-INPUT-VALIDATION](#ipv6-f-input-validation)). Supporting:
  RFC8200-CKSUM-2 (the ICMPv6 pseudo-header; an encoding statement for a unit test).

## IPV6-F-INPUT-VALIDATION

**A node discards input that it must not process: a UDP datagram with a zero checksum, a
packet whose next header names nothing the node knows, and an ICMPv6 informational message
of unknown type; it reports the second and stays silent about the third.**

- **Sources** — RFC 8200 §8.1, `rfc8200.txt:1536-1537`; §4, `rfc8200.txt:455-473`;
  RFC 8504 §5.2, `rfc8504.txt:373-383` (governs the unrecognized next header); RFC 4443
  §2.4 (b), `rfc4443.txt:292-293`.
- **Level** — mandatory (reason: keyword). "IPv6 receivers must discard UDP packets
  containing a zero checksum"; "Any unrecognized extension headers or options MUST be
  processed as described in RFC 8200"; "it MUST be silently discarded".
- **Description** — the three rules keep a malformed or unknown input from reaching a
  program or from producing a reply the sender cannot use. IPv6 has no header checksum, so
  the transport checksum is the one integrity check a receiver has.
- **Checks** — core: RFC8200-CKSUM-1 (the discard of a zero checksum), RFC8504-NR-6 (an
  unrecognized next header is discarded and reported with Parameter Problem code 1;
  RFC8200-EXT-3 is the overridden base statement), RFC4443-MPR-2 (an unknown informational
  type is silently discarded). Supporting: RFC8200-HL-3 (a destination processes a packet
  that arrives with hop limit 0; a should, core in
  [IPV6-F-HOP-LIMIT](#ipv6-f-hop-limit) as supporting), RFC8200-EXT-2 (extension headers in
  order; level 5), RFC8504-NR-7 (the extension headers are processed; implementation).

## IPV6-F-ERROR-SUPPRESSION

**A node sends no ICMPv6 error message about an error message, about a packet destined to a
multicast address or sent as a link-layer multicast or broadcast, or about a packet whose
source address is not one node; the Packet Too Big message is the exception for the three
multicast cases.**

- **Sources** — RFC 4443 §2.4 (e), `rfc4443.txt:317-349`, and its note,
  `rfc4443.txt:379-381`.
- **Level** — mandatory (reason: keyword). "An ICMPv6 error message MUST NOT be originated
  as a result of receiving the following".
- **Description** — the six rules prevent error storms and error loops; the exceptions
  keep path MTU discovery alive for multicast. Each rule needs a scenario of its own,
  because each fault arrives in a different form.
- **Checks** — core: RFC4443-MPR-4 (an error about an error), RFC4443-MPR-6 (a multicast
  destination), RFC4443-MPR-7 (a link-layer multicast), RFC4443-MPR-8 (a link-layer
  broadcast), RFC4443-MPR-9 (a source that is not one node). Supporting: RFC4443-MPR-5 (a
  redirect; needs neighbor discovery), RFC4443-DU-C (congestion; level 4).

## IPV6-F-ERROR-DELIVERY

**A received ICMPv6 error message, of a known or of an unknown type, reaches the upper-layer
process that sent the quoted packet, selected by the protocol of the quoted header.**

- **Sources** — RFC 4443 §2.4 (a) and (d), `rfc4443.txt:287-304`; §3.1, §3.2, §3.3,
  `rfc4443.txt:513-515`, `rfc4443.txt:569-571`, `rfc4443.txt:625-627`.
- **Level** — mandatory (reason: keyword). Each source sentence says "MUST be passed" or
  "MUST notify".
- **Description** — the report is useless unless it reaches the protocol that sent the
  packet; the quoted header is what makes the handoff possible.
- **Checks** — core: RFC4443-MPR-1 (an error of unknown type is passed up), RFC4443-UL-1,
  UL-2, UL-3 (each known error type is passed up). Supporting: RFC4443-MPR-3 (the
  selection by the quoted protocol). Every core statement is of class `internal`: the
  handoff happens inside the node, and no packet shows it. The map keeps the feature
  because the standard defines it; the category decision belongs to step 9. The one
  observable half, that the node stays silent about the unknown error, is RFC4443-MPR-4.

## Coverage of the catalogs

Step 4 requires that every area of every in-scope catalog appears in at least one feature,
or that this map marks the area as out of scope.

| Catalog area | Feature |
| --- | --- |
| RFC 8200, Header format | IPV6-F-HEADER, IPV6-F-DELIVERY |
| RFC 8200, Hop limit | IPV6-F-HOP-LIMIT |
| RFC 8200, Extension headers | IPV6-F-SOURCE-FRAGMENTATION (the fragment header in transit), IPV6-F-INPUT-VALIDATION (the unrecognized next header, the order) |
| RFC 8200, Fragmentation | IPV6-F-SOURCE-FRAGMENTATION, IPV6-F-PACKET-TOO-BIG |
| RFC 8200, Reassembly | IPV6-F-REASSEMBLY |
| RFC 8200, Packet size | IPV6-F-PACKET-SIZE, IPV6-F-REASSEMBLY, IPV6-F-PACKET-TOO-BIG |
| RFC 8200, Upper-layer checksums | IPV6-F-UPPER-LAYER-CHECKSUM, IPV6-F-INPUT-VALIDATION (the discard) |
| RFC 4443, Error signals | IPV6-F-ERROR-REPORT, IPV6-F-PACKET-TOO-BIG |
| RFC 4443, Source address of a message | IPV6-F-ERROR-REPORT |
| RFC 4443, Message processing rules | IPV6-F-ERROR-SUPPRESSION, IPV6-F-ERROR-DELIVERY, IPV6-F-INPUT-VALIDATION, IPV6-F-ERROR-REPORT (the rate limit) |
| RFC 4443, Destination Unreachable, further rules | IPV6-F-ERROR-REPORT, IPV6-F-ERROR-SUPPRESSION |
| RFC 4443, Upper-layer notification | IPV6-F-ERROR-DELIVERY |
| RFC 8504, Internet Protocol version 6 | IPV6-F-SOURCE-FRAGMENTATION, IPV6-F-REASSEMBLY, IPV6-F-DELIVERY (RFC8504-NR-1) |
| RFC 8504, Extension headers | IPV6-F-INPUT-VALIDATION, IPV6-F-SOURCE-FRAGMENTATION, IPV6-F-REASSEMBLY |
| RFC 8504, Path MTU discovery and packet size | IPV6-F-PACKET-SIZE, IPV6-F-PACKET-TOO-BIG, IPV6-F-REASSEMBLY |
| RFC 8504, ICMPv6 | IPV6-F-ERROR-REPORT |

All 32 entries of RFC 8200, all 23 entries of RFC 4443, and all 12 entries of RFC 8504
appear in the map, and no entry appears in none. RFC8504-NR-1 (support RFC 8200 and follow
its transmission rules) is a supporting statement of IPV6-F-DELIVERY and, through it, of
every RFC 8200 feature.

Out of scope in the map, because the catalogs put them out of scope: the other extension
headers and their options, the flow label, the traffic class, jumbograms, the routing
header responses, and the ICMPv6 messages other than the three error reports. Each becomes
a feature on the day its catalog entries exist.
