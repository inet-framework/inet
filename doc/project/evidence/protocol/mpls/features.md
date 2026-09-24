# MPLS — feature map

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `MPLS-F-*` · **Stands on:** [standards.md](standards.md), [rfc3031/catalog.md](../../standard/rfc3031/catalog.md), [rfc3032/catalog.md](../../standard/rfc3032/catalog.md), [rfc3443/catalog.md](../../standard/rfc3443/catalog.md), [rfc5462/catalog.md](../../standard/rfc5462/catalog.md)

Step 4 artifact of the standards test workflow. The catalogs are flat, fine-grained, and per
document. This document is the high-level view above them: the capabilities that the
in-scope set of [`standards.md`](standards.md#in-scope-set) defines, the requirement level of
each, and the cross reference into the catalogs.

The map reads in two directions:

- **Down** — which checks tell whether a feature works. A `core` check is one the feature
  cannot work without. A `supporting` check is a detail or an edge case.
- **Up** — which features the simulation model supports. Step 7 writes the answer into
  [`coverage.md`](../../model/mpls/coverage.md), and step 8 compares it with the claims of the
  model in [`conformance.md`](../../model/mpls/conformance.md).

The feature list comes from the standard texts only. The support of each feature — what the
run actually showed — is **not** in this document. It lives in the coverage ledger,
[`coverage.md`](../../model/mpls/coverage.md).

## How this map decides the level

The rule of the guide: `mandatory` when any core statement says `must` or `shall`, or when the
mechanism is the only path the document gives to a state or an outcome; `optional` when every
core statement says `may` or `should`; `unstated` otherwise.

One refinement, which the earlier maps state too: **a conditional keyword does not raise the
level of a feature.** A rule that holds "if" an LSR does an optional thing takes the level of that
thing.

A second refinement matters more here than in any earlier map: **most of RFC 3031, RFC 3443 and
RFC 5462 is written without keywords.** RFC 3031 defines the architecture in terms, and RFC 3443
states its TTL rules as descriptions. Where a feature is the only way the documents give to do
something — the one encoding of a label stack entry, the one forwarding step of a labeled
packet, the one encoding on a LAN — the level is `mandatory` by the "only path" rule of the
guide, and the feature says so.

## Index

| ID | Feature |
| --- | --- |
| [MPLS-F-LABEL-STACK-ENCODING](#mpls-f-label-stack-encoding) | A label stack entry is 4 octets: a 20-bit label, a 3-bit Traffic Class field, the S bit and an 8-bit TTL; the stack sits between the link header and the network header, top entry first, and only the bottom entry has the S bit set. |
| [MPLS-F-RESERVED-LABELS](#mpls-f-reserved-labels) | The label values 0 to 3 have fixed meanings — IPv4 Explicit NULL, Router Alert, IPv6 Explicit NULL and Implicit NULL — and the values 4 to 15 are reserved. |
| [MPLS-F-LABEL-FORWARDING](#mpls-f-label-forwarding) | An LSR forwards a labeled packet by its top label: the Incoming Label Map gives an NHLFE, whose operation swaps the label, pops the stack, or swaps and pushes, and whose next hop gets the packet. |
| [MPLS-F-INGRESS-LABELING](#mpls-f-ingress-labeling) | An ingress LSR maps an unlabeled packet to a FEC, finds its NHLFE in the FEC-to-NHLFE map, and pushes the label of the FEC, a label that identifies the network layer of the packet. |
| [MPLS-F-EGRESS-DECAPSULATION](#mpls-f-egress-decapsulation) | The LSR that pops the last label identifies the network layer protocol of the packet from the label, and processes the packet by its network header. |
| [MPLS-F-PENULTIMATE-HOP-POPPING](#mpls-f-penultimate-hop-popping) | The penultimate LSR of an LSP can pop the stack, so that the egress gets the packet with the label below or unlabeled; an LSR that can pop does so when its downstream peer asks. |
| [MPLS-F-TTL](#mpls-f-ttl) | The TTL of the first label is the IP TTL, each LSR forwards with an outgoing TTL one below the incoming one and never with zero, and when the last label is popped the IP TTL gets the outgoing TTL. |
| [MPLS-F-PIPE-MODELS](#mpls-f-pipe-models) | In the Pipe and the Short Pipe Models, the LSP counts as one hop for the tunneled packet: the pushed label gets a TTL of the operator, and the tunneled TTL drops by two in all. |
| [MPLS-F-TRAFFIC-CLASS](#mpls-f-traffic-class) | The Traffic Class field can carry QoS and ECN marks, and a pushed entry can copy it from the entry below. |
| [MPLS-F-INVALID-LABEL](#mpls-f-invalid-label) | An LSR discards a labeled packet whose incoming label is invalid, or has no NHLFE, unless it knows that forwarding it unlabeled does no harm. |
| [MPLS-F-ICMP](#mpls-f-icmp) | An LSR can send an ICMP message about a labeled IP packet, when it knows the packet is IP and can reach its source, also by the label stack of the packet. |
| [MPLS-F-FRAGMENTATION](#mpls-f-fragmentation) | A labeled IP datagram that is too big for the next link is fragmented, each fragment with the same label stack, or discarded with an ICMP report when it may not be fragmented. |
| [MPLS-F-PPP](#mpls-f-ppp) | On a PPP link a labeled packet travels with the PPP protocol 0281 hex, after the MPLS Control Protocol has opened. |
| [MPLS-F-LAN](#mpls-f-lan) | On a LAN a labeled packet travels in a frame of ethertype 8847 hex, with the label stack right after the link headers. |

## Summary table

| Feature | Level | Sources | Core checks |
| --- | --- | --- | --- |
| [MPLS-F-LABEL-STACK-ENCODING](#mpls-f-label-stack-encoding) | mandatory | RFC 3032 §2.1; RFC 5462 §2.1; RFC 3031 §3.1, §3.9 | RFC3032-ENC-1, ENC-2, ENC-3, ENC-5, ENC-6, ENC-7, ENC-8, ENC-9, ENC-10, RFC5462-TC-1 |
| [MPLS-F-RESERVED-LABELS](#mpls-f-reserved-labels) | mandatory | RFC 3032 §2.1; RFC 3031 §4.1.5 | RFC3032-ENC-11, ENC-14, ENC-15, ENC-16 |
| [MPLS-F-LABEL-FORWARDING](#mpls-f-label-forwarding) | mandatory | RFC 3031 §3.1, §3.9 to §3.11, §3.13, §3.15 | RFC3031-ILM-1, SWAP-1, NHLFE-1, NHLFE-2, NHLFE-3, NHLFE-4, STK-2, LSP-4 |
| [MPLS-F-INGRESS-LABELING](#mpls-f-ingress-labeling) | mandatory | RFC 3031 §3.12, §3.13, §3.15; RFC 3032 §2.2 | RFC3031-FTN-1, SWAP-2, LSP-1, RFC3032-NLP-4 |
| [MPLS-F-EGRESS-DECAPSULATION](#mpls-f-egress-decapsulation) | mandatory | RFC 3032 §2.2 | RFC3032-NLP-1, NLP-2, NLP-3, NLP-5 |
| [MPLS-F-PENULTIMATE-HOP-POPPING](#mpls-f-penultimate-hop-popping) | mandatory | RFC 3031 §3.16 | RFC3031-PHP-1, PHP-2, PHP-3, PHP-6 |
| [MPLS-F-TTL](#mpls-f-ttl) | mandatory | RFC 3032 §2.4; RFC 3031 §3.23; RFC 3443 §2.2, §3.1, §3.4 to §3.7 | RFC3032-TTL-1, TTL-2, TTL-3, TTL-5, TTL-9, TTL-10, RFC3031-TTL-3 |
| [MPLS-F-PIPE-MODELS](#mpls-f-pipe-models) | optional | RFC 3443 §3.2 to §3.7 | RFC3443-MOD-2, MOD-3, MOD-4, PUSH-2, OTTL-4 |
| [MPLS-F-TRAFFIC-CLASS](#mpls-f-traffic-class) | optional | RFC 5462 §3 | RFC5462-TC-2, TC-3 |
| [MPLS-F-INVALID-LABEL](#mpls-f-invalid-label) | mandatory | RFC 3031 §3.18, §3.22 | RFC3031-INV-1, NOL-1 |
| [MPLS-F-ICMP](#mpls-f-icmp) | optional | RFC 3032 §2.3 | RFC3032-ICMP-1, ICMP-2, ICMP-4 |
| [MPLS-F-FRAGMENTATION](#mpls-f-fragmentation) | mandatory | RFC 3032 §3 | RFC3032-FRAG-8, FRAG-9, FRAG-11, FRAG-12, FRAG-13, FRAG-18, FRAG-19 |
| [MPLS-F-PPP](#mpls-f-ppp) | mandatory | RFC 3032 §4 | RFC3032-PPP-1, PPP-11, PPP-12, PPP-13, PPP-16 |
| [MPLS-F-LAN](#mpls-f-lan) | mandatory | RFC 3032 §5 | RFC3032-LAN-1, LAN-2, LAN-3 |

## MPLS-F-LABEL-STACK-ENCODING

**A label stack entry is 4 octets: a 20-bit label, a 3-bit Traffic Class field, the S bit and an 8-bit TTL; the stack sits between the link header and the network header, top entry first, and only the bottom entry has the S bit set.**

- **Sources** — RFC 3032 §2.1; RFC 5462 §2.1; RFC 3031 §3.1, §3.9.
- **Level** — mandatory (reason: only path). RFC 3032 §2.1 gives the one encoding of a label stack entry.
- **Description** — The network layer packet follows the entry with the S bit. RFC 5462 renames the Exp field to the Traffic Class field and keeps its position and width. The label carries no encoded form of the destination address, and the two ends agree on the encoding.
- **Checks** — core: RFC3032-ENC-1, ENC-2, ENC-3, ENC-5, ENC-6, ENC-7, ENC-8, ENC-9, ENC-10, RFC5462-TC-1. Supporting: RFC3032-ENC-4, RFC3031-STK-1, LBL-3, LBL-4.

## MPLS-F-RESERVED-LABELS

**The label values 0 to 3 have fixed meanings — IPv4 Explicit NULL, Router Alert, IPv6 Explicit NULL and Implicit NULL — and the values 4 to 15 are reserved.**

- **Sources** — RFC 3032 §2.1; RFC 3031 §4.1.5.
- **Level** — mandatory (reason: keyword). RFC3032-ENC-11 has the strength "must (lower case)".
- **Description** — An Explicit NULL label is legal only at the bottom of the stack; the LSR pops it and forwards by the network header of its protocol. A Router Alert label goes to local software, and comes back on top when the packet is forwarded. Implicit NULL never appears on the wire: an LSR that would swap to it pops the stack.
- **Checks** — core: RFC3032-ENC-11, ENC-14, ENC-15, ENC-16. Supporting: RFC3032-ENC-12, ENC-13, RFC3031-NULL-1, NULL-2, NULL-3.

## MPLS-F-LABEL-FORWARDING

**An LSR forwards a labeled packet by its top label: the Incoming Label Map gives an NHLFE, whose operation swaps the label, pops the stack, or swaps and pushes, and whose next hop gets the packet.**

- **Sources** — RFC 3031 §3.1, §3.9 to §3.11, §3.13, §3.15.
- **Level** — mandatory (reason: only path). The Incoming Label Map and the NHLFE are the one way RFC 3031 forwards a labeled packet.
- **Description** — The NHLFE holds the next hop and may hold the link encapsulation and the stack encoding. When the next hop is the LSR itself, it pops and forwards again. Every intermediate LSR of an LSP gets the packet with the same stack depth, and forwards it by the top label alone.
- **Checks** — core: RFC3031-ILM-1, SWAP-1, NHLFE-1, NHLFE-2, NHLFE-3, NHLFE-4, STK-2, LSP-4. Supporting: RFC3031-ILM-2, NHLFE-5, NHLFE-6, NHLFE-7, SWAP-4, LSP-2, LSP-3, LSP-5, LBL-1, LBL-2.

## MPLS-F-INGRESS-LABELING

**An ingress LSR maps an unlabeled packet to a FEC, finds its NHLFE in the FEC-to-NHLFE map, and pushes the label of the FEC, a label that identifies the network layer of the packet.**

- **Sources** — RFC 3031 §3.12, §3.13, §3.15; RFC 3032 §2.2.
- **Level** — mandatory (reason: keyword). RFC3032-NLP-4 has the strength "must (lower case)".
- **Description** — The LSR that pushes the first label is the LSP Ingress. Popping is not a valid operation on an unlabeled packet. A label pushed onto a labeled packet belongs to a FEC whose egress is the LSR that the label below is for.
- **Checks** — core: RFC3031-FTN-1, SWAP-2, LSP-1, RFC3032-NLP-4. Supporting: RFC3031-FTN-2, SWAP-3, LSP-6, LSP-7.

## MPLS-F-EGRESS-DECAPSULATION

**The LSR that pops the last label identifies the network layer protocol of the packet from the label, and processes the packet by its network header.**

- **Sources** — RFC 3032 §2.2.
- **Level** — mandatory (reason: keyword). RFC3032-NLP-2 has the strength "must (lower case)".
- **Description** — The label stack names no protocol, so each label value identifies one; a swap keeps that property. A packet whose protocol the LSR cannot identify is silently discarded.
- **Checks** — core: RFC3032-NLP-1, NLP-2, NLP-3, NLP-5. Supporting: RFC3032-NLP-6, NLP-7.

## MPLS-F-PENULTIMATE-HOP-POPPING

**The penultimate LSR of an LSP can pop the stack, so that the egress gets the packet with the label below or unlabeled; an LSR that can pop does so when its downstream peer asks.**

- **Sources** — RFC 3031 §3.16.
- **Level** — mandatory (reason: keyword). RFC3031-PHP-6 has the strength "must". Its condition, an LSR that can pop the label stack at all, holds for every LSR that ends an LSP, so the refinement for conditional keywords does not apply.
- **Description** — The egress then forwards by the label now on top, or by the network header. The egress need not be an LSR. Label distribution lets each LSR learn whether its neighbors can pop.
- **Checks** — core: RFC3031-PHP-1, PHP-2, PHP-3, PHP-6. Supporting: RFC3031-PHP-4, PHP-5, PHP-7, PHP-8.

## MPLS-F-TTL

**The TTL of the first label is the IP TTL, each LSR forwards with an outgoing TTL one below the incoming one and never with zero, and when the last label is popped the IP TTL gets the outgoing TTL.**

- **Sources** — RFC 3032 §2.4; RFC 3031 §3.23; RFC 3443 §2.2, §3.1, §3.4 to §3.7.
- **Level** — mandatory (reason: keyword). RFC3032-TTL-3 has the strength "must not".
- **Description** — This is the Uniform Model of RFC 3443: the TTL of the packet after the LSP is the TTL it would have had without label switching. The outgoing TTL depends only on the incoming TTL, whatever the stack operations. A packet whose outgoing TTL is zero is discarded, or passed to the network layer for an ICMP message.
- **Checks** — core: RFC3032-TTL-1, TTL-2, TTL-3, TTL-5, TTL-9, TTL-10, RFC3031-TTL-3. Supporting: RFC3032-TTL-4, TTL-6, TTL-7, TTL-8, RFC3031-TTL-1, TTL-2, TTL-4, TTL-5, TTL-6, TTL-7, TTL-8, RFC3443-TERM-1, TERM-2, TERM-3, TERM-4, IMPL-1, IMPL-2, MOD-1, ITTL-4, OTTL-5, PUSH-1.

## MPLS-F-PIPE-MODELS

**In the Pipe and the Short Pipe Models, the LSP counts as one hop for the tunneled packet: the pushed label gets a TTL of the operator, and the tunneled TTL drops by two in all.**

- **Sources** — RFC 3443 §3.2 to §3.7.
- **Level** — optional (reason: base document). RFC 3443 gives the Pipe and the Short Pipe Models as choices of a network, beside the Uniform Model of RFC 3032 §2.4, which MPLS-F-TTL holds.
- **Description** — With penultimate hop popping, the Short Pipe egress decrements only the exposed header, and the penultimate LSR does not touch it. The default TTL of a pushed Pipe label is 255.
- **Checks** — core: RFC3443-MOD-2, MOD-3, MOD-4, PUSH-2, OTTL-4. Supporting: RFC3443-ITTL-1, ITTL-2, ITTL-3, ITTL-5, OTTL-1, OTTL-2, OTTL-3, PUSH-3, IMPL-3.

## MPLS-F-TRAFFIC-CLASS

**The Traffic Class field can carry QoS and ECN marks, and a pushed entry can copy it from the entry below.**

- **Sources** — RFC 5462 §3.
- **Level** — optional (reason: keyword). Every core statement says should or may.
- **Description** — The field has few bits, so a QoS or an ECN function may rewrite some or all of them.
- **Checks** — core: RFC5462-TC-2, TC-3.

## MPLS-F-INVALID-LABEL

**An LSR discards a labeled packet whose incoming label is invalid, or has no NHLFE, unless it knows that forwarding it unlabeled does no harm.**

- **Sources** — RFC 3031 §3.18, §3.22.
- **Level** — mandatory (reason: keyword). RFC3031-INV-1 has the strength "must".
- **Description** — The standards map places both rules at level 3: a label that no binding holds.
- **Checks** — core: RFC3031-INV-1, NOL-1.

## MPLS-F-ICMP

**An LSR can send an ICMP message about a labeled IP packet, when it knows the packet is IP and can reach its source, also by the label stack of the packet.**

- **Sources** — RFC 3032 §2.3.
- **Level** — optional (reason: conditional keyword). The must of RFC3032-ICMP-1, ICMP-2 and ICMP-4 holds only for an LSR that sends an ICMP message, and RFC3032-TTL-4 lets an LSR discard the packet instead.
- **Description** — The copied label stack keeps the label values, and its TTL is long enough for the way through the egress and back.
- **Checks** — core: RFC3032-ICMP-1, ICMP-2, ICMP-4. Supporting: RFC3032-ICMP-3, ICMP-5, ICMP-6.

## MPLS-F-FRAGMENTATION

**A labeled IP datagram that is too big for the next link is fragmented, each fragment with the same label stack, or discarded with an ICMP report when it may not be fragmented.**

- **Sources** — RFC 3032 §3.
- **Level** — mandatory (reason: keyword). RFC3032-FRAG-8 has the strength "must".
- **Description** — IPv4 without DF: fragments, or a silent discard; IPv4 with DF: an ICMP Destination Unreachable with the MTU; IPv6: an ICMP Packet Too Big, or fragments when the datagram has a fragment header and is at most 1280 octets. The Maximum Initially Labeled IP Datagram Size limits the size at the ingress.
- **Checks** — core: RFC3032-FRAG-8, FRAG-9, FRAG-11, FRAG-12, FRAG-13, FRAG-18, FRAG-19. Supporting: RFC3032-FRAG-1, FRAG-2, FRAG-3, FRAG-4, FRAG-5, FRAG-6, FRAG-7, FRAG-10, FRAG-14, FRAG-15, FRAG-16, FRAG-17, FRAG-20, FRAG-21.

## MPLS-F-PPP

**On a PPP link a labeled packet travels with the PPP protocol 0281 hex, after the MPLS Control Protocol has opened.**

- **Sources** — RFC 3032 §4.
- **Level** — mandatory (reason: keyword). RFC3032-PPP-1 has the strength "must (lower case)".
- **Description** — The MPLS Control Protocol, protocol 8281 hex, runs after LCP in the Network-Layer Protocol phase, with Code values 1 to 7 and no options. A multicast labeled packet uses 0283 hex.
- **Checks** — core: RFC3032-PPP-1, PPP-11, PPP-12, PPP-13, PPP-16. Supporting: RFC3032-PPP-2, PPP-3, PPP-4, PPP-5, PPP-6, PPP-7, PPP-8, PPP-9, PPP-10, PPP-14, PPP-15.

## MPLS-F-LAN

**On a LAN a labeled packet travels in a frame of ethertype 8847 hex, with the label stack right after the link headers.**

- **Sources** — RFC 3032 §5.
- **Level** — mandatory (reason: only path). RFC 3032 §5 gives the one encoding of a labeled packet on a LAN.
- **Description** — A frame carries one labeled packet; a multicast labeled packet uses 8848 hex. Both work with the Ethernet and the LLC/SNAP encapsulation.
- **Checks** — core: RFC3032-LAN-1, LAN-2, LAN-3. Supporting: RFC3032-LAN-4, LAN-5.

## Coverage of the catalog entries

Every one of the 157 entries of the four catalogs is in at least one feature above, as core
or as supporting: RFC 3031 with 49, RFC 3032 with
81, RFC 3443 with
24 and RFC 5462 with 3.
