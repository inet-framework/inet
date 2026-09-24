# OSPFv3 — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around OSPF for IPv6 (OSPFv3), records which document governs each contested clause, and
pins the set that a level 2 pass tests against. RFC 5340 states its own relationship to OSPF
version 2 in its own words, `rfc5340.txt:698-699`: "the basic OSPF mechanisms remain
unchanged from those documented in [OSPFV2]. These mechanisms are briefly outlined in
Section 4 of [OSPFV2]." A large share of the in-scope set is therefore RFC 2328 clauses that
RFC 5340 points at, read from the same text as [`ospfv2/standards.md`](../ospfv2/standards.md).

The relationships come from the RFC-editor metadata
(`https://www.rfc-editor.org/rfc/rfcNNNN.json`), retrieved 2026-09-23.

## Target level

**Level 1 — Survey** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
This pass maps the family, pins the in-scope set that a level 2 pass needs, and records the
claims of the model. It writes no catalog, no feature map and no test.

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 2, Core | RFC 5340 §4.1 to §4.1.3 (the area, interface and neighbor data structures), §4.2 to §4.2.2.1 (sending and receiving Hello and Database Description packets), §4.3 and §4.3.1 (the routing table structure), §4.4.1 to §4.4.3.3 and §4.4.3.8, §4.4.3.9 and §4.4.4 (the LSA header, the link-state database, LSA options, Router-LSAs, Network-LSAs, Link-LSAs, Intra-Area-Prefix-LSAs, and future LSA validation), §4.5 to §4.5.3 and §4.6 (flooding and self-originated LSAs), §4.8, §4.8.1 and §4.8.2 (the routing table calculation for one area and the next-hop calculation), Appendix A.1 to A.3 (encapsulation, the Options field, the packet formats) and A.4.1 to A.4.4, A.4.9 and A.4.10 (the IPv6 prefix representation, the LSA header, and the Router-LSA, Network-LSA, Link-LSA and Intra-Area-Prefix-LSA formats), together with the RFC 2328 clauses this set points at (see the mapping table below) | the normal exchange within a single area: the Hello protocol, forming an adjacency, the database exchange, flooding, Designated Router and Backup Designated Router election, the Router-LSA, Network-LSA, Link-LSA and Intra-Area-Prefix-LSA formats, and the routing table from the shortest-path tree of that area |
| level 3, Edge | nothing new | the same clauses hold the validity checks of a received packet or LSA |
| level 4, Dynamics | RFC 5340 §4.2.1.1 (sending Hello packets) and the timers RFC 2328 §9.5, §13.3, §13.6, §14 and §14.1 govern unchanged (see the mapping table) | the periodic Hello, the retransmission of an unacknowledged LSA, and the aging of the database are timers |
| level 5, Complete | RFC 5340 §4.4.3.4 to §4.4.3.7 (Inter-Area-Prefix-LSAs, Inter-Area-Router-LSAs, AS-External-LSAs, NSSA-LSAs), §4.7 (virtual links), §4.8.3 to §4.8.5 (inter-area, transit-area and AS external/NSSA route calculation), §4.9 and §4.9.1 (multiple interfaces to one link, the Standby interface state); RFC 6845 (the hybrid broadcast and point-to-multipoint interface type); RFC 6860 (hiding transit-only networks); RFC 7503 (OSPFv3 autoconfiguration); RFC 8362 (the Extended LSA TLV encoding); RFC 5838 (address families); RFC 9454 (terminology) | more than one area, external and NSSA route redistribution, a second interface to the same link, and the optional extensions |

What a pass actually reached is not recorded here. It is in
[`model/ospfv3/coverage.md`](../../model/ospfv3/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Downloaded |
| --- | --- | --- | --- | --- | --- |
| RFC 5340 | OSPF for IPv6 | July 2008 | Proposed Standard | `base`; obsoletes RFC 2740 | [`standards/RFC/rfc5340.txt`](../../../../../../standards/RFC/rfc5340.txt), 2026-09-23 |
| RFC 2740 | OSPF for IPv6 | December 1999 | Proposed Standard | obsoleted by RFC 5340 | no |
| RFC 2328 | OSPF Version 2 | April 1998 | Internet Standard (STD 54) | `companion`; RFC 5340 inherits its procedures, see the mapping table below | [`standards/RFC/rfc2328.txt`](../../../../../../standards/RFC/rfc2328.txt), 2026-09-23; the same file [`ospfv2/standards.md`](../ospfv2/standards.md) uses |
| RFC 6845 | OSPF Hybrid Broadcast and Point-to-Multipoint Interface Type | January 2013 | Proposed Standard | `updates` RFC 2328 and RFC 5340 | no |
| RFC 6860 | Hiding Transit-Only Networks in OSPF | January 2013 | Proposed Standard | `updates` RFC 2328 and RFC 5340 | no |
| RFC 7503 | OSPFv3 Autoconfiguration | April 2015 | Proposed Standard | `updates` RFC 5340 | no |
| RFC 8362 | OSPFv3 Link State Advertisement (LSA) Extensibility | April 2018 | Proposed Standard | `updates` RFC 5340 and RFC 5838 | no |
| RFC 9454 | Update to OSPF Terminology | August 2023 | Proposed Standard | `updates` RFC 5340 and six other RFCs | no |
| RFC 5838 | Support of Address Families in OSPFv3 | April 2010 | Proposed Standard | `companion`; `updated by` RFC 6969, RFC 7949, RFC 8362, RFC 9454 | no |

Sources of the texts:

- `rfc5340.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc5340.txt) —
  <https://www.rfc-editor.org/rfc/rfc5340.txt>, downloaded 2026-09-23. 94 pages, 5267 lines.
- `rfc2328.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc2328.txt), downloaded
  2026-09-23 for [`ospfv2/standards.md`](../ospfv2/standards.md); reused here, not
  re-downloaded, per the guide's rule that one document serves every protocol that uses it.

RFC 5340 uses the RFC 2119 convention (§1.1, "Requirements Notation"): upper-case "MUST" on
29 lines, "SHOULD" on 14, "MAY" on 13, "SHALL" and "REQUIRED" once each, "RECOMMENDED" once.
It also carries lower-case "must" (36 lines), "should" (69) and "may" (53) in descriptive
prose, mostly where it paraphrases RFC 2328, which itself has no upper-case keyword at all
(see [`ospfv2/standards.md`](../ospfv2/standards.md#document-list)).

RFC 5340 §2.6, `rfc5340.txt:379-388`, removes authentication from the protocol itself: "the
'AuType' and 'Authentication' fields have been removed from the OSPF packet header... OSPF
relies on the IP Authentication Header... and the IP Encapsulating Security Payload." OSPFv3
has no authentication mechanism of its own to bring into scope at any level; the mechanism
that protects an OSPFv3 exchange belongs to IPv6's own evidence tree, not this one.

### Which RFC 2328 clauses this pass inherits

RFC 5340 §4, `rfc5340.txt:715-762`, names a list of RFC 2328 clauses that it states are
"completely unchanged" or "remain unchanged" for IPv6. §4.5, `rfc5340.txt:2214-2223`, adds
five more that are "exactly the same." §4.8.1, `rfc5340.txt:2505-2506`, states its
shortest-path calculation is "identical" to RFC 2328's, with named exceptions.

| Mechanism | RFC 5340 | Inherits from RFC 2328 |
| --- | --- | --- |
| Bringing up adjacencies: the Hello protocol, database synchronization, the Designated Router, the Backup Designated Router, the adjacency graph | "remains completely unchanged," `rfc5340.txt:740-744` | §7, §7.1 to §7.5 |
| Interface state machine and Designated Router election | "remain unchanged," `rfc5340.txt:750-753` | §9.1 to §9.4 |
| Neighbor state machine | "remains unchanged," `rfc5340.txt:755-757` | §10.1 to §10.4 |
| Aging and premature aging | "remains unchanged," `rfc5340.txt:759-761` | §14, §14.1 |
| Flooding: which LSA instance is newer, responding to self-originated updates, acknowledgments, retransmission, receiving acknowledgments | "exactly the same," `rfc5340.txt:2214-2223` | §13.1, §13.4, §13.5, §13.6, §13.7 |
| Flooding: receiving and sending Link State Updates, installing LSAs in the database | modified for flooding scope and the IPv6 LSA reorganization, `rfc5340.txt:2224-2231` | §13 and §13.3 (governing text is now RFC 5340 §4.5.1 and §4.5.2); §13.2 (governing text is now RFC 5340 §4.5.3) |
| Shortest-path tree for one area | "identical...with the following exceptions," `rfc5340.txt:2505-2506` | §16.1 (governing text is now RFC 5340 §4.8.1, which states the exceptions) |

RFC 9454 renames the same "master/slave" and "MS bit" terms this table's Database
Description row inherits to "Leader/Follower" and the "L bit," inside RFC 5340 as well as
RFC 2328 ([`ospfv2/standards.md`](../ospfv2/standards.md#document-list)). The update states
the operation is not modified.

## Override table

| Area | Base clause | Later clause | Governs | In scope now |
| --- | --- | --- | --- | --- |
| LSA wire encoding | RFC 5340 Appendix A.4, `rfc5340.txt:3792-3797`: "This document defines eight distinct types of LSAs. Each LSA begins with a standard 20-byte LSA header" (A.4.1 to A.4.10, one subsection per LSA type) | RFC 8362 §4.1 to §4.8, "OSPFv3 Extended LSAs": a TLV-based encoding (E-Router-LSA, E-Network-LSA, and so on) for the same information | RFC 8362 when the Extended LSA capability is negotiated; RFC 5340 Appendix A.4 otherwise | no; level 5 |
| Advertising a transit-only network's prefix | RFC 5340 §4.4.3.9, "Intra-Area-Prefix-LSAs": every attached prefix is advertised | RFC 6860 §3 and §3.1, "Hiding IPv6 Transit-Only Networks in OSPFv3" | RFC 6860 when hiding is configured; RFC 5340 §4.4.3.9 otherwise | no; level 5 |
| The Intra-Area-Prefix-LSA of a broadcast network | RFC 5340 §4.4.3.2 and §4.4.3.9, Appendix A.4.10 (`rfc5340.txt:4668`) | RFC 6845 §4.6.2, "OSPFv3 Intra-Area-Prefix-LSA": the hybrid broadcast/point-to-multipoint representation | RFC 6845 when the hybrid mode is configured; RFC 5340 otherwise | no; level 5 |
| HelloInterval/RouterDeadInterval matching, and receiving a self-originated LSA | RFC 5340 §4.2.1.1 ("compare to Section 9.5 of [OSPFV2]") and §4.5 ("exactly the same" as RFC 2328 §13.4) | RFC 7503 §3, "OSPFv3 HelloInterval/RouterDeadInterval Flexibility," and §7.4, "Change to RFC 2328, Section 13.4" | RFC 7503 in an autoconfiguring deployment; RFC 5340/RFC 2328 §13.4 otherwise | no; level 5 |

## In-scope set

The set that a level 2 pass tests against:

| Document | Version | Catalog file |
| --- | --- | --- |
| RFC 5340 | July 2008, Proposed Standard; §4.1 to §4.1.3, §4.2 to §4.2.2.1, §4.3, §4.3.1, §4.4.1 to §4.4.3.3, §4.4.3.8, §4.4.3.9, §4.4.4, §4.5 to §4.5.3, §4.6, §4.8, §4.8.1, §4.8.2, Appendix A.1 to A.3, A.4.1 to A.4.4, A.4.9, A.4.10 | none yet; level 2 writes `standard/rfc5340/catalog.md` |
| RFC 2328 | April 1998, Internet Standard (STD 54); the clauses of the mapping table above | none yet; shares `standard/rfc2328/catalog.md` with [`ospfv2/standards.md`](../ospfv2/standards.md#in-scope-set); OSPFv3 owns no new area of it |

Out of scope, with the reason:

| Document / area | Reason |
| --- | --- |
| RFC 2740 | Obsoleted by RFC 5340. |
| RFC 5340 §4.4.3.4, §4.4.3.5 (Inter-Area-Prefix-LSAs, Inter-Area-Router-LSAs), Appendix A.4.5, A.4.6, §4.8.3, §4.8.4 | Level 5. More than one area. The level-2 slice tests the shortest-path tree of a single area. |
| RFC 5340 §4.4.3.6, §4.4.3.7 (AS-External-LSAs, NSSA-LSAs), Appendix A.4.7, A.4.8, §4.8.5 | Level 5. External and NSSA routes are redistributed from outside the area or the Autonomous System; the level-2 slice has no AS boundary router and no NSSA area. |
| RFC 5340 §4.7 | Level 5. A virtual link exists only to carry the backbone through a second, transit area. |
| RFC 5340 §4.9, §4.9.1 | Level 5. A second interface to the same link, and the Standby interface state it adds, is additional topology richness on top of the one-interface-per-link scenario of the level-2 slice. |
| RFC 5340 §2.6 (authentication removed) | Not a scope decision at any level: RFC 5340 defines no authentication mechanism of its own. The protection of an OSPFv3 exchange is IPv6's Authentication Header and Encapsulating Security Payload, a different protocol's evidence tree. |
| RFC 6845 | Level 5. The hybrid broadcast and point-to-multipoint interface type is an alternative LSA representation for a network that already works under the plain broadcast model of the level-2 slice. |
| RFC 6860 | Level 5. Hiding transit-only networks is an optional routing-table installation choice on top of the normal shortest-path calculation. |
| RFC 7503 | Level 5. Autoconfiguration relaxes and extends the normal exchange for a specific deployment profile; the level-2 slice tests the normal exchange without it. |
| RFC 8362 | Level 5. The Extended LSA encoding is a negotiated alternative to the LSA formats of the level-2 slice. |
| RFC 5838 | Level 5, companion. Running a second address family's routes inside one OSPFv3 instance needs more than the single, one-area instance of the level-2 slice. |
| RFC 9454 | Editorial. It renames terms inside clauses of the level-2 slice; by its own words (see [`ospfv2/standards.md`](../ospfv2/standards.md#document-list)), the operation is not modified. |
| RFC 2328, the clauses outside the mapping table (§3, §4.1, §4.2, §12.4.3, §12.4.4, §15, §16.2 to §16.4, Appendix D) | Out of scope for the reasons [`ospfv2/standards.md`](../ospfv2/standards.md#in-scope-set) gives; RFC 5340 does not point at these clauses for IPv6 either. |
