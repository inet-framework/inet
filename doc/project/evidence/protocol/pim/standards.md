# PIM — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around Protocol Independent Multicast, records which document governs each contested clause,
and pins the set that the next pass tests against. PIM has two independent base documents for
its two modes: Sparse Mode and Dense Mode. They describe different protocols that share a
message header and an address-encoding format, not the same protocol for two address families.

The relationships come from the RFC-editor metadata
(`https://www.rfc-editor.org/rfc/rfcNNNN.json`), retrieved 2026-09-23.

## Target level

**Level 1 — Survey** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
This pass maps the family, pins the in-scope set that a level 2 pass needs, and records the
claims of the model. It writes no catalog, no feature map and no test.

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 2, Core | RFC 7761 §3, §4.1 to §4.6, §4.7 (static configuration only), §4.8, §4.9.1 to §4.9.6; RFC 3973 §3, §4.1 to §4.7; RFC 3956; RFC 4607 | the normal message exchange of both modes — Hello, Join/Prune, Register, Register-Stop and Assert for Sparse Mode; Hello, Prune, Join, Graft, Graft-Ack, State-Refresh and Assert for Dense Mode — the one group-to-RP mapping mechanism RFC 7761 mandates (static configuration), and the two mechanisms that the model uses by default in its place (embedded-RP and source-specific multicast) |
| level 3, Edge | RFC 9436 | it redefines the Reserved field of the common PIM header as a per-type Flag Bits field and reserves three new message types; the field carries no meaning yet for the eight message types of the level 2 set, so a check needs a crafted, non-zero value |
| level 4, Dynamics | RFC 7761 §4.10 and §4.11; RFC 3973 §4.8 | the Sparse Mode and Dense Mode timers: Join/Prune, Hello, Assert, Register-Suppression, and their Dense Mode counterparts, Prune, Graft-Retry, Source-Active and State-Refresh |
| level 5, Complete | RFC 5059 | the Bootstrap Router mechanism, one of the three dynamic alternatives to static RP configuration that RFC 7761 §4.7 lists and does not mandate |

`RFC 7761` Appendix A already removed the `(*,*,RP)` state, the PIM Multicast Border Router
feature, and IPsec-based authentication from the base document; none of the three is testable
material for any level of this protocol, current or future.

What a pass actually reached is not recorded here. It is in
[`model/pim/coverage.md`](../../model/pim/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Downloaded |
| --- | --- | --- | --- | --- | --- |
| RFC 7761 | Protocol Independent Multicast - Sparse Mode (PIM-SM): Protocol Specification (Revised) | March 2016 | Internet Standard (STD 83) | `base`, Sparse Mode; obsoletes RFC 4601 | [`standards/RFC/rfc7761.txt`](../../../../../../standards/RFC/rfc7761.txt), 2026-09-23 |
| RFC 4601 | Protocol Independent Multicast - Sparse Mode (PIM-SM): Protocol Specification (Revised) | August 2006 | Proposed Standard | `obsoleted by` RFC 7761 | no |
| RFC 3973 | Protocol Independent Multicast - Dense Mode (PIM-DM): Protocol Specification (Revised) | January 2005 | Experimental | `base`, Dense Mode; no obsoletes, no obsoleted by | [`standards/RFC/rfc3973.txt`](../../../../../../standards/RFC/rfc3973.txt), 2026-09-23 |
| RFC 9436 | PIM Message Type Space Extension and Reserved Bits | August 2023 | Proposed Standard | `updates` RFC 7761, RFC 3973, and others; obsoletes RFC 8736 | no |
| RFC 8736 | PIM Message Type Space Extension and Reserved Bits | February 2020 | Proposed Standard | `updates` RFC 7761, RFC 3973, and others; `obsoleted by` RFC 9436 | no |
| RFC 3956 | Embedding the Rendezvous Point (RP) Address in an IPv6 Multicast Address | November 2004 | Proposed Standard | `companion`; updates RFC 3306 | [`standards/RFC/rfc3956.txt`](../../../../../../standards/RFC/rfc3956.txt), 2026-09-23 |
| RFC 4607 | Source-Specific Multicast for IP | August 2006 | Proposed Standard | `companion` | [`standards/RFC/rfc4607.txt`](../../../../../../standards/RFC/rfc4607.txt), 2026-09-23 |
| RFC 5059 | Bootstrap Router (BSR) Mechanism for Protocol Independent Multicast (PIM) | January 2008 | Proposed Standard | `companion`; updates RFC 4601; obsoletes RFC 2362 | no |
| RFC 8200 | Internet Protocol, Version 6 (IPv6) Specification | July 2017 | Internet Standard | `companion`, checksum pseudo-header only; obsoletes RFC 2460 | no |
| RFC 2460 | Internet Protocol, Version 6 (IPv6) Specification | December 1998 | Draft Standard | `obsoleted by` RFC 8200 | no |

Source of the texts:

- `rfc7761.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc7761.txt) —
  <https://www.rfc-editor.org/rfc/rfc7761.txt>, downloaded 2026-09-23.
- `rfc3973.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc3973.txt) —
  <https://www.rfc-editor.org/rfc/rfc3973.txt>, downloaded 2026-09-23.
- `rfc3956.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc3956.txt) —
  <https://www.rfc-editor.org/rfc/rfc3956.txt>, downloaded 2026-09-23.
- `rfc4607.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc4607.txt) —
  <https://www.rfc-editor.org/rfc/rfc4607.txt>, downloaded 2026-09-23.

All four downloaded documents use the keywords of RFC 2119. Counts of whole-word matches: RFC 7761
has `MUST` on 51 lines and `SHOULD` on 43; RFC 3973 has `MUST` on 129 lines and `SHOULD` on 37;
RFC 3956 has `MUST` on 14 lines and `SHOULD` on 4; RFC 4607 has `MUST` on 18 lines and
`SHOULD` on 9.

RFC 3973 §4.7, `rfc3973.txt:2099-2101`, states its own deference to the Sparse Mode document:
"All PIM-DM packets use the same format as PIM-SM packets. In the event of a discrepancy,
PIM-SM ... should be considered the definitive specification." Keep one catalog for each
document even so, because the two modes are different protocols with different message sets:
PIM-DM has no Register, Register-Stop, or Bootstrap message, and PIM-SM has no Graft or
Graft-Ack message.

| Mechanism | RFC 7761 (PIM-SM) | RFC 3973 (PIM-DM) |
| --- | --- | --- |
| protocol overview | §3 | §3 |
| protocol state | §4.1 | §4.1 |
| data-packet forwarding | §4.2 | §4.2 |
| Hello messages, neighbors | §4.3 | §4.3 |
| join and prune | §4.5 | §4.4 (combined with graft) |
| assert | §4.6 | §4.6 |
| RP discovery, register | §4.4, §4.7 | not applicable, no RP |
| state refresh | not applicable | §4.5 |
| common header and message formats | §4.9 | §4.7 |
| timers | §4.10, §4.11 | §4.8 |

## Override table

| Area | Base clause | Later clause | Governs | In scope now |
| --- | --- | --- | --- | --- |
| The PIM-SM base specification | RFC 4601 (whole document), obsolete | RFC 7761, `rfc7761.txt:35-38`: "obsoletes RFC 4601 by replacing it ... removes the optional (*,*,RP), PIM Multicast Border Router features and authentication using IPsec"; the removed items are listed in Appendix A, `rfc7761.txt:7567-7578` | RFC 7761 | yes; RFC 7761 is the base document of the in-scope set |
| The Reserved field of the common PIM header | RFC 7761 §4.9, `rfc7761.txt:5831-5832`, and RFC 3973 §4.7.1, `rfc3973.txt:2148-2149`: "Reserved / Set to zero on transmission. Ignored upon receipt." | RFC 9436 §4, renames the field "Flag Bits field" and assigns per-type meaning to it for message type 4 (Bootstrap, RFC 5059), type 10 (DF Election, RFC 5015) and type 12 (PIM Flooding Mechanism, RFC 8364); RFC 9436 §5 extends the type space with types 13 to 15 | RFC 9436, for the four affected types; RFC 7761 and RFC 3973 still govern the eight types of the in-scope set, unchanged | no; level 3, see the target-level table |
| Group-to-RP mapping | RFC 7761 §4.7, `rfc7761.txt:5471-5473`: "This specification does not mandate the use of a single mechanism ... Currently, four mechanisms are possible"; `rfc7761.txt:5477-5478`: "A PIM router MUST support the static configuration of group-to-RP mappings" | RFC 5059 defines one of the four alternatives, the Bootstrap Router mechanism, `rfc7761.txt:5503-5511` names it and points to it; RFC 3956 defines a second alternative, embedded-RP, `rfc7761.txt:5481-5484` | RFC 7761 for the mandatory baseline (static configuration); RFC 3956 and RFC 5059 for the two dynamic alternatives, neither of which the base document requires | RFC 3956 yes, from level 2 (the model's other default path); RFC 5059 no, level 5 |

## In-scope set

The set that a level 2 pass tests against:

| Document | Version | Catalog file |
| --- | --- | --- |
| RFC 7761 | March 2016, Internet Standard (STD 83); §3, §4.1 to §4.6, §4.7 (static configuration), §4.8, §4.9.1 to §4.9.6 | none yet; level 2 writes `standard/rfc7761/catalog.md` |
| RFC 3973 | January 2005, Experimental; §3, §4.1 to §4.7 | none yet; level 2 writes `standard/rfc3973/catalog.md` |
| RFC 3956 | November 2004, Proposed Standard; in full | none yet; level 2 writes `standard/rfc3956/catalog.md` |
| RFC 4607 | August 2006, Proposed Standard; in full | none yet; level 2 writes `standard/rfc4607/catalog.md` |

Out of scope, with the reason:

| Document | Reason |
| --- | --- |
| RFC 4601 | Obsoleted by RFC 7761. |
| RFC 8736 | Obsoleted by RFC 9436. |
| RFC 9436 | Level 3. It only assigns meaning to the header's flag bits for three message types outside the level 2 set (Bootstrap, DF Election, PIM Flooding Mechanism) and reserves three further, currently unused, message types. |
| RFC 5059 | Level 5. RFC 7761 §4.7 lists the Bootstrap Router mechanism as one of three dynamic alternatives to the one mechanism it mandates, static configuration; it is not the only path to a group-to-RP mapping. |
| RFC 2460 | Obsoleted by RFC 8200. |
| RFC 8200 | Not part of the PIM family. It supplies the IPv6 pseudo-header layout that a PIM checksum computation over IPv6 uses; PIM's own checksum procedure is stated directly in RFC 7761 §4.9 and RFC 3973 §4.7. |
