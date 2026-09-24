# OSPFv2 — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around Open Shortest Path First version 2, records which document governs each contested
clause, and pins the set that a level 2 pass tests against. RFC 2328 is a large document
(244 pages); this pass pins a clause slice of it for level 2 and defers the rest.

The relationships come from the RFC-editor metadata
(`https://www.rfc-editor.org/rfc/rfcNNNN.json`), retrieved 2026-09-23.

## Target level

**Level 1 — Survey** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
This pass maps the family, pins the in-scope set that a level 2 pass needs, and records the
claims of the model. It writes no catalog, no feature map and no test.

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 2, Core | RFC 2328 §5 to §10 (the protocol data structures, packet processing, interface data structure and Designated Router election, and the neighbor data structure and state machine), §12.1, §12.2, §12.4.1 and §12.4.2 (the LSA header, and the Router-LSA and Network-LSA formats), §13 (the flooding procedure), §16.1 (the shortest-path tree for one area), and Appendix A (the packet and LSA wire formats of A.3 and A.4.1 to A.4.3) | the normal exchange within a single area: the Hello protocol, forming an adjacency, the database exchange, flooding, Designated Router and Backup Designated Router election, the Router-LSA and Network-LSA formats, and the routing table from the shortest-path tree of that area |
| level 3, Edge | nothing new | the same clauses hold the validity checks of a received packet or LSA: a bad checksum, a Hello with a mismatched HelloInterval or RouterDeadInterval, a Database Description packet out of sequence, an unexpected LSA type |
| level 4, Dynamics | RFC 2328 §9.5 and §9.5.1 (sending Hello packets), §13.3 and §13.6 (retransmission), §14 and §14.1 (aging and premature aging), and the timers of Appendix C.3 (HelloInterval, RouterDeadInterval, RxmtInterval) | the periodic Hello, the retransmission of an unacknowledged LSA, and the aging of the database are timers |
| level 5, Complete | RFC 2328 §3 (splitting the AS into areas), §4.1, §4.2, §12.4.3 (Summary-LSAs), §12.4.4 (AS-external-LSAs), §15 (virtual links), §16.2 to §16.4 (inter-area and AS external routes), Appendix D (authentication: Null, Simple, Cryptographic); RFC 3101 (the NSSA option); RFC 5709 and RFC 7474 (cryptographic and manual-key authentication); RFC 6549 (multiple instances on one interface); RFC 6845 (the hybrid broadcast and point-to-multipoint interface type); RFC 6860 (hiding transit-only networks); RFC 8042 (the two-part metric); RFC 9355 (BFD strict-mode) | more than one area, the backbone-through-a-transit-area topology of a virtual link, external route redistribution, the authentication types, and the optional extensions |

What a pass actually reached is not recorded here. It is in
[`model/ospfv2/coverage.md`](../../model/ospfv2/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Downloaded |
| --- | --- | --- | --- | --- | --- |
| RFC 2328 | OSPF Version 2 | April 1998 | Internet Standard (STD 54) | `base`; obsoletes RFC 2178 | [`standards/RFC/rfc2328.txt`](../../../../../../standards/RFC/rfc2328.txt), 2026-09-23 |
| RFC 2178 | OSPF Version 2 | July 1997 | Draft Standard | obsoleted by RFC 2328; obsoletes RFC 1583 | no |
| RFC 1583 | OSPF Version 2 | March 1994 | Draft Standard | obsoleted by RFC 2178 | no |
| RFC 5709 | OSPFv2 HMAC-SHA Cryptographic Authentication | October 2009 | Proposed Standard | `updates` RFC 2328; `updated by` RFC 7474 | no |
| RFC 6549 | OSPFv2 Multi-Instance Extensions | March 2012 | Proposed Standard | `updates` RFC 2328 | no |
| RFC 6845 | OSPF Hybrid Broadcast and Point-to-Multipoint Interface Type | January 2013 | Proposed Standard | `updates` RFC 2328 and RFC 5340 | no |
| RFC 6860 | Hiding Transit-Only Networks in OSPF | January 2013 | Proposed Standard | `updates` RFC 2328 and RFC 5340 | no |
| RFC 7474 | Security Extension for OSPFv2 When Using Manual Key Management | April 2015 | Proposed Standard | `updates` RFC 2328 and RFC 5709 | no |
| RFC 8042 | OSPF Two-Part Metric | December 2016 | Proposed Standard | `updates` RFC 2328 | no |
| RFC 9355 | OSPF Bidirectional Forwarding Detection (BFD) Strict-Mode | February 2023 | Proposed Standard | `updates` RFC 2328 | no |
| RFC 9454 | Update to OSPF Terminology | August 2023 | Proposed Standard | `updates` RFC 2328 and six other RFCs, one of them RFC 5340 | no |
| RFC 3101 | The OSPF Not-So-Stubby Area (NSSA) Option | January 2003 | Proposed Standard | `companion`; obsoletes RFC 1587 | no |

Source of the text:

- `rfc2328.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc2328.txt) —
  <https://www.rfc-editor.org/rfc/rfc2328.txt>, downloaded 2026-09-23. 244 pages, 12201 lines.

RFC 2328 predates the common use of the RFC 2119 convention: it carries no upper-case
keyword. It uses "must" on 163 lines, "should" on 197, "may" on 135, "required" on 21 and
"recommended" on 2, all lower case. The catalog of a later pass records the words as
written; most statements have the strength `description` or a lower-case keyword, not the
formal RFC 2119 word.

RFC 2328 Appendix C.1 defines a configurable parameter, `RFC1583Compatibility`
(`rfc2328.txt:10941-10964`), that selects between two rules for choosing among multiple
AS-external-LSAs that advertise the same destination: the rule of RFC 1583, or the
loop-preventing rule of RFC 2328 §16.4.1. Appendix G, "Differences from RFC 2178"
(`rfc2328.txt:11955-11957`), documents this and the other differences from the document RFC
2328 obsoletes directly. The parameter is about AS-external route preference, §16.4; it has
no connection to RFC 3101, the NSSA option, which is a companion document about a different
area type.

RFC 9454 renames the Database Description exchange's "master/slave" relationship and the
"master (MS) bit" to "Leader/Follower" and the "Leader (L) bit" inside RFC 2328 §7.2, §10,
§10.1 to §10.3, §10.6, §10.8, §10.10 and Appendix A.3.3 — clauses of the level-2 slice above.
The document states this in its own words: "The operation of OSPFv2 is not modified." It is
a naming change for a later catalog to use, not an override of any clause's substance.

## Override table

| Area | Base clause | Later clause | Governs | In scope now |
| --- | --- | --- | --- | --- |
| The algorithm behind OSPFv2 cryptographic authentication | RFC 2328 Appendix D.3, `rfc2328.txt:11374-11387`: "This specification completely defines the use of OSPF Cryptographic authentication when the MD5 algorithm is used" | RFC 5709 §3, "Cryptographic Authentication with NIST SHS in HMAC Mode"; RFC 7474 §3 and §5 add an anti-replay sequence number and cover the IP header | RFC 5709 and RFC 7474 for the algorithm and the anti-replay sequence, when configured; RFC 2328 Appendix D for the Null and Simple types and the message layout | no; level 5 |
| The Router-LSA link description of a broadcast interface | RFC 2328 §12.4.1.2, `rfc2328.txt:6492-6511`: a Type 2 (transit network) or Type 3 (stub network) link, one cost | RFC 6845 §4.6.1, "Stub Links in OSPFv2 Router-LSA": the same interface represented as a set of point-to-multipoint links | RFC 6845 when the hybrid mode is configured; RFC 2328 §12.4.1.2 otherwise | no; level 5 |
| Installing a transit network's prefix in the routing table | RFC 2328 §16.1, `rfc2328.txt:8027-8035`: a stub-network leaf is added to the shortest-path tree for every attached prefix | RFC 6860 §2, "Hiding IPv4 Transit-Only Networks in OSPFv2" | RFC 6860 when hiding is configured; RFC 2328 §16.1 otherwise | no; level 5 |
| The cost of a multi-access interface in a Router-LSA | RFC 2328 §12.4.1.2, `rfc2328.txt:6492-6511`: one cost value for the whole link | RFC 8042 §3.2, "Advertising Network-to-Router Metric in OSPFv2": a separate network-to-router part | RFC 8042 when the two-part metric is configured; RFC 2328 §12.4.1.2 otherwise | no; level 5 |
| The 2-WayReceived event in neighbor state Init | RFC 2328 §10.2, `rfc2328.txt:4484-4494`: the event alone decides whether the neighbor state becomes 2-Way or ExStart | RFC 9355 §4, "Procedures": adjacency formation past 2-Way is blocked until a BFD session is up, when strict-mode is advertised | RFC 9355 when BFD strict-mode is negotiated; RFC 2328 §10.2 otherwise | no; level 5 |

## In-scope set

The set that a level 2 pass tests against:

| Document | Version | Catalog file |
| --- | --- | --- |
| RFC 2328 | April 1998, Internet Standard (STD 54); §5 to §10, §12.1, §12.2, §12.4.1, §12.4.2, §13, §16.1, Appendix A.3 and A.4.1 to A.4.3 | none yet; level 2 writes `standard/rfc2328/catalog.md` |

Out of scope, with the reason:

| Document / area | Reason |
| --- | --- |
| RFC 2178, RFC 1583 | Obsoleted. RFC 2328 Appendix G lists the differences from RFC 2178, which itself obsoletes RFC 1583; the current text governs through RFC 2328. |
| RFC 2328 §3, §4.1, §12.4.3 (Summary-LSAs), §16.2, §16.3 | Level 5. More than one area. The level-2 slice tests the shortest-path tree of a single area; Summary-LSAs and inter-area route calculation need an area border router and a second area. |
| RFC 2328 §4.2, §12.4.4 (AS-external-LSAs), §16.4 | Level 5. External routes are redistributed from outside the Autonomous System; the level-2 slice has no AS boundary router. |
| RFC 2328 §15 (virtual links) | Level 5. A virtual link exists only to carry the backbone through a second, transit area, which needs the multi-area topology above. |
| RFC 2328 §14, §14.1, and the RxmtInterval and HelloInterval/RouterDeadInterval timers of Appendix C.3 | Level 4. The database age, the LSA retransmission, and the periodic Hello are timers, with a tolerance to state. |
| RFC 2328 Appendix D (authentication: Null, Simple, Cryptographic) | Level 5. Section 4.5 names one base optional capability, the stub-area E-bit; the authentication type is a further per-interface configuration choice, and the normal exchange of the level-2 slice does not need a non-null type. |
| RFC 5709, RFC 7474 | Level 5, with RFC 2328 Appendix D: additional cryptographic algorithms and anti-replay protection for the authentication type above. |
| RFC 6549 | Level 5. Multiple OSPFv2 instances on one interface is an additional capability on top of the normal, single-instance exchange. |
| RFC 6845 | Level 5. The hybrid broadcast and point-to-multipoint interface type is an alternative Router-LSA representation for a network that already works under the plain broadcast model of the level-2 slice. |
| RFC 6860 | Level 5. Hiding transit-only networks is an optional routing-table installation choice on top of the normal shortest-path calculation. |
| RFC 8042 | Level 5. The two-part metric is an optional alternative to the single interface cost the level-2 slice uses. |
| RFC 9355 | Level 5. BFD strict-mode is a negotiated precondition on top of the normal neighbor state machine. |
| RFC 9454 | Editorial. It renames terms inside clauses of the level-2 slice; by its own words, "the operation of OSPFv2 is not modified." It changes no in-scope behavior. |
| RFC 3101 | Level 5, companion. The NSSA option imports external routes into a stub area in a limited way; it needs both the stub-area mechanism and the external-route mechanism, neither in scope yet. |
