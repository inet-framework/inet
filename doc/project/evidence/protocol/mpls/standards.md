# MPLS — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around Multiprotocol Label Switching, records which document governs each contested clause,
and pins the set that the next pass tests against. MPLS forwarding rests on two base
documents: RFC 3031 defines the architecture (the label, the LSR, label swapping, the label
stack), and RFC 3032 defines the wire encoding of the label stack entry. The two signaling
protocols that install label bindings, LDP and RSVP-TE, and the traffic-engineering database
that feeds them, are separate protocols. They get their own protocol folder when they get a
pass; this document does not map their families.

The relationships come from the RFC-editor metadata
(`https://www.rfc-editor.org/rfc/rfcNNNN.json`), retrieved 2026-09-23.

## Target level

**Level 1 — Survey** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
This pass maps the family, pins the in-scope set that a level 2 pass needs, and records the
claims of the model. It writes no catalog, no feature map and no test.

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 2, Core | RFC 3031 §3.1, §3.9 to §3.13, §3.16, §3.23; RFC 3032 §2.1, §2.2, §2.4 | the normal path: what a label is, the label stack, the Incoming Label Map and the NHLFE, label swapping, penultimate hop popping, the Time-to-Live rule, the label stack entry encoding (label, Exp/TC, S, TTL), and how a receiver finds the network-layer protocol under the stack |
| level 3, Edge | RFC 3031 §3.18, §3.22; RFC 3032 §2.3 | in scope now, through the override table. §3.18 and §3.22 hold the negative cases (an invalid incoming label, no outgoing label for a FEC); §2.3 holds the ICMP report for a labeled IP packet that cannot be delivered |
| level 4, Dynamics | nothing new | RFC 3031 states no timer or control loop of its own; the label-distribution protocols that do (LDP, RSVP-TE) are out of scope of this protocol |
| level 5, Complete | RFC 3031 §3.20, §3.26, §3.27 (aggregation, label merging, tunnels and hierarchy); RFC 3270, RFC 5129, RFC 5332, RFC 5586, RFC 6790, RFC 7274, RFC 9017 | optional and compound mechanisms: DiffServ-over-MPLS, ECN marking, multicast encapsulation, the generic associated channel, entropy labels, and the special-purpose label registry |

What a pass actually reached is not recorded here. It is in
[`model/mpls/coverage.md`](../../model/mpls/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Downloaded |
| --- | --- | --- | --- | --- | --- |
| RFC 3031 | Multiprotocol Label Switching Architecture | January 2001 | Proposed Standard | `base`, architecture | [`standards/RFC/rfc3031.txt`](../../../../../../standards/RFC/rfc3031.txt), 2026-09-23 |
| RFC 3032 | MPLS Label Stack Encoding | January 2001 | Proposed Standard | `base`, wire encoding | [`standards/RFC/rfc3032.txt`](../../../../../../standards/RFC/rfc3032.txt), 2026-09-23 |
| RFC 3443 | Time To Live (TTL) Processing in Multi-Protocol Label Switching (MPLS) Networks | January 2003 | Proposed Standard | `updates` RFC 3032 | [`standards/RFC/rfc3443.txt`](../../../../../../standards/RFC/rfc3443.txt), 2026-09-23 |
| RFC 5462 | MPLS Label Stack Entry: "EXP" Field Renamed to "Traffic Class" Field | February 2009 | Proposed Standard | `updates` RFC 3032 (and RFC 3443, among others) | [`standards/RFC/rfc5462.txt`](../../../../../../standards/RFC/rfc5462.txt), 2026-09-23 |
| RFC 6178 | Label Edge Router Forwarding of IPv4 Option Packets | March 2011 | Proposed Standard | `updates` RFC 3031 | no |
| RFC 4182 | Removing a Restriction on the use of MPLS Explicit NULL | September 2005 | Proposed Standard | `updates` RFC 3032 (and RFC 5462, RFC 7274 update it in turn) | no |
| RFC 7274 | Allocating and Retiring Special-Purpose MPLS Labels | June 2014 | Proposed Standard | `updates` RFC 3032, RFC 3209, and others | no |
| RFC 3270 | Multi-Protocol Label Switching (MPLS) Support of Differentiated Services | May 2002 | Proposed Standard | `updates` RFC 3032 | no |
| RFC 5129 | Explicit Congestion Marking in MPLS | January 2008 | Proposed Standard | `updates` RFC 3032 | no |
| RFC 5332 | MPLS Multicast Encapsulations | August 2008 | Proposed Standard | `updates` RFC 3032, RFC 4023 | no |
| RFC 5586 | MPLS Generic Associated Channel | June 2009 | Proposed Standard | `updates` RFC 3032, and others | no |
| RFC 6790 | The Use of Entropy Labels in MPLS Forwarding | November 2012 | Proposed Standard | `updates` RFC 3031, RFC 3107, RFC 3209, RFC 5036 | no |
| RFC 9017 | Special-Purpose Label Terminology | April 2021 | Proposed Standard | `updates` RFC 3032, RFC 7274 | no |
| RFC 5036 | LDP Specification | October 2007 | Proposed Standard | `companion`; the base document of a separate protocol, out of scope | no |
| RFC 2205 | Resource ReSerVation Protocol (RSVP) | September 1997 | Proposed Standard | `companion`; the base document of a separate protocol, out of scope | no |
| RFC 3209 | RSVP-TE: Extensions to RSVP for LSP Tunnels | December 2001 | Proposed Standard | `companion`; the base document of a separate protocol, out of scope | no |

Source of the texts:

- `rfc3031.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc3031.txt) —
  <https://www.rfc-editor.org/rfc/rfc3031.txt>, downloaded 2026-09-23.
- `rfc3032.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc3032.txt) —
  <https://www.rfc-editor.org/rfc/rfc3032.txt>, downloaded 2026-09-23.
- `rfc3443.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc3443.txt) —
  <https://www.rfc-editor.org/rfc/rfc3443.txt>, downloaded 2026-09-23.
- `rfc5462.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc5462.txt) —
  <https://www.rfc-editor.org/rfc/rfc5462.txt>, downloaded 2026-09-23.

Neither base document uses the keywords of RFC 2119 heavily. RFC 3031 has "MUST" on 10
lines, "SHOULD" on 6 and "MAY" on 2 (`grep -c`); RFC 3032 has "MUST" on 16 lines, "SHOULD"
on 3 and "MAY" on 5. Most of RFC 3031 is a definition of terms (`rfc3031.txt:37-90` lists
28 subsections of §3, one term each), so the level of a feature will mostly come from the
"only path" rule of the guide and not from a keyword. RFC 3443 and RFC 5462 each use each
keyword once, in the boilerplate RFC 2119 reference of their introduction, and are
themselves short amendments (10 and 9 pages).

## Override table

| Area | Base clause | Later clause | Governs | In scope now |
| --- | --- | --- | --- | --- |
| Time-to-Live processing beyond the default case | RFC 3032 §2.4, `rfc3032.txt:424-467`: defines the incoming/outgoing TTL and states the rule for what this document calls the (implicit) Uniform Model only | RFC 3443 §2.1, `rfc3443.txt:76-81`: "[MPLS-ENCAPS] only covers the Uniform Model and does NOT address the Pipe Model or the Short Pipe Model. This document addresses [it]" | RFC 3443, for a network configured for the Pipe or the Short Pipe Model; RFC 3032 §2.4 for the Uniform Model, the default | yes |
| The name and the use of the 3-bit field after the label | RFC 3032 §2.1, `rfc3032.txt:159,196`: "Exp: Experimental Use, 3 bits" ... "This three-bit field is reserved for experimental use" | RFC 5462, `rfc5462.txt:65-66`: "This document changes the name of the field to the 'Traffic Class field' ('TC field')" | RFC 5462 for the field's name; RFC 3032 for its position and width in the label stack entry | yes |
| Forwarding of an IPv4 packet that carries IP options | RFC 3031 does not mention IP options; §3.22 (`rfc3031.txt:73`) covers only the case of no outgoing label at all | RFC 6178: a Label Edge Router MUST NOT label-switch an IPv4 packet with an unrecognized option, and must forward it unlabeled or drop it | RFC 6178 | no; level 3, a corner case of the ingress classification of one packet type, not the label-swap operation itself |
| Explicit NULL label placement | RFC 3032 §2.1 restricts the IPv4 and IPv6 Explicit NULL labels to the bottom of the label stack | RFC 4182: removes that restriction; an Explicit NULL label may appear at another position | RFC 4182 | no; level 5, a placement rule for one reserved label value, not the general swap/push/pop operation |
| The meaning of the reserved label values 4-15 | RFC 3032 §2.1: values 4-15 of the 20-bit label field are "reserved for future use" | RFC 7274: turns the reserved block into an IANA-managed "special-purpose label" registry with a defined allocation procedure | RFC 7274 | no; level 5, a registry-management mechanism, not a data-plane rule; the four label values RFC 3032 itself already defines (0-3) are unchanged |
| The use of the Exp/TC field for a DiffServ PHB | RFC 3032 §2.1 leaves the use of the Exp field undefined ("reserved for experimental use") | RFC 3270: assigns the field a DiffServ Code-Point mapping for MPLS-aware DiffServ | RFC 3270 | no; level 5, a separate feature that combines MPLS with DiffServ; nothing in this family claims it |

## In-scope set

The set that a level 2 pass tests against:

| Document | Version | Catalog file |
| --- | --- | --- |
| RFC 3031 | January 2001, Proposed Standard; §3.1, §3.9 to §3.13, §3.16, §3.18, §3.22, §3.23 | none yet; level 2 writes `standard/rfc3031/catalog.md` |
| RFC 3032 | January 2001, Proposed Standard; §2.1, §2.2, §2.3, §2.4, as amended below | none yet; level 2 writes `standard/rfc3032/catalog.md` |
| RFC 3443 | January 2003, Proposed Standard; §2.1, §3 (the Pipe and Short Pipe Model TTL rules) | none yet |
| RFC 5462 | February 2009, Proposed Standard; the field rename of §2.1 | none yet |

Out of scope, with the reason:

| Document | Reason |
| --- | --- |
| RFC 6178 | Level 3. A corner case of ingress classification for one IPv4 packet type (a packet with options), not the label-swap operation. |
| RFC 4182 | Level 5. A placement restriction on one reserved label value (Explicit NULL). |
| RFC 7274 | Level 5. Turns the reserved label block into a registry; a management mechanism, not a data-plane rule. |
| RFC 3270 | Level 5. Combines MPLS with DiffServ, a second protocol family; nothing in the model claims the combination. |
| RFC 5129 | Level 5. Optional congestion marking on the Exp/TC field, an experimental use of the same field RFC 5462 renames. |
| RFC 5332 | Level 5. A separate encapsulation for multicast traffic; the base documents cover unicast forwarding. |
| RFC 5586 | Level 5. A separate operations-and-maintenance channel identified by a reserved label; not a forwarding rule. |
| RFC 6790 | Level 5. An optional label type for load-balancing (entropy labels); it updates RFC 3031, not the label stack encoding of RFC 3032. |
| RFC 9017 | Terminology only. It restates the vocabulary of RFC 7274 and RFC 3032; no rule changes. |
| RFC 5036 (LDP) | A separate protocol: the label-distribution signaling that installs the bindings this document's LSR consults. It gets its own protocol folder. |
| RFC 2205, RFC 3209 (RSVP, RSVP-TE) | A separate protocol: the other label-distribution signaling family. It gets its own protocol folder. |
| RFC 3630, RFC 5305 (OSPF-TE, ISIS-TE) | The traffic-engineering topology distribution that feeds RSVP-TE; out of scope with RSVP-TE. |

RFC 3031 and RFC 3032 are companions, not a base-and-successor pair: neither obsoletes the
other, both are dated January 2001, and each is `updated_by` its own separate family (RFC
3031 by RFC 6178 and RFC 6790, the architecture-level updates; RFC 3032 by nine documents,
the encoding-level updates). No override exists between the two base documents themselves.
