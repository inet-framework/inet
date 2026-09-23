# OSPFv3 — model claims

> **Kind:** report · **Status:** snapshot 2026-09-23 · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/ospfv3/standards.md), [coverage.md](coverage.md)

Step 8 artifact of the standards test workflow, part 1 only. A level 1 pass records what the
model intends: which standards it claims to implement, mapped onto the standards map. Part 2,
the conformance matrix, needs the feature map and the verdicts of a level 2 pass.

Read record — no build, no run:

- Date: 2026-09-23
- INET: branch `topic/standards-tests-wave0`, commit `e360ca980e`, tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`

## Part 1 — the claims

Every place in the model that names a standard of the OSPFv3 family, found with
`grep -rn -E 'RFC ?[0-9]{3,4}|draft-' src/inet/routing/ospfv3 src/inet/node/ospfv3
--include=*.ned --include=*.cc --include=*.h --include=*.msg`, excluding generated
`*_m.cc`/`*_m.h`, plus the users-guide:

| Claim | Where | What it says |
| --- | --- | --- |
| none | [`Ospfv3.ned:13-14`](../../../../../src/inet/routing/ospfv3/Ospfv3.ned) | "Implements the OSPFv3 (Open Shortest Path First version 3) routing protocol for IPv4 and IPv6 networks." The module documentation comment names no document. |
| RFC 5340 §2.5 | [`Ospfv3Checksum.h:20`](../../../../../src/inet/routing/ospfv3/Ospfv3Checksum.h) | "upper-layer packet length, next header = OSPF) followed by the entire packet, per RFC 5340 2.5." |
| RFC 2328 §12.1.7 | [`Ospfv3Checksum.h:27`](../../../../../src/inet/routing/ospfv3/Ospfv3Checksum.h) | "the Fletcher checksum over the LSA contents excepting the LS Age field, per RFC 2328 12.1.7." |
| RFC 5340 §2.5 | [`Ospfv3Checksum.cc:34`](../../../../../src/inet/routing/ospfv3/Ospfv3Checksum.cc) | "RFC 5340 2.5: the checksum is the standard Internet checksum over the IPv6 [pseudo-header and packet]." |
| RFC 2328 §12.1.7 | [`Ospfv3Checksum.cc:64`](../../../../../src/inet/routing/ospfv3/Ospfv3Checksum.cc) | "RFC 2328 12.1.7: Fletcher checksum of the LSA contents excepting the LS Age field." |
| RFC 5340 Appendix A | [`Ospfv3PacketSerializer.h:24`](../../../../../src/inet/routing/ospfv3/Ospfv3PacketSerializer.h) | "(RFC 5340 Appendix A)" above the serializer class. |
| RFC 5340, §A.4.1 | [`Ospfv3PacketSerializer.h:38,44,48,73`](../../../../../src/inet/routing/ospfv3/Ospfv3PacketSerializer.h) | Four comments on the 128-bit IPv6 address encoding, the address-prefix encoding (twice), and "the RFC 2328 / RFC 5340 LS Checksum (Fletcher)". |
| RFC 5340 §A.3.1, §A.2, §A.4.2, §A.4.1, §A.4.3 to §A.4.9 | [`Ospfv3PacketSerializer.cc:25,59,87,89,130,154,186,248`](../../../../../src/inet/routing/ospfv3/Ospfv3PacketSerializer.cc) | Eight comments that cite an appendix clause of RFC 5340 above the packet header, the Options field, the LSA header, and the address-prefix and LSA-body encodings; each sentence continues onto the next one or two lines (`89-92`, `130-132`, `154-156`). |
| RFC 5340 | [`Ospfv3Packet.msg:23`](../../../../../src/inet/routing/ospfv3/Ospfv3Packet.msg) | "RFC 5340 A.3.1. The OSPF Packet Header: Checksum:" |
| RFC 5340 §2.5, RFC 2328 §12.1.7 | [`process/Ospfv3Process.ned:22`](../../../../../src/inet/routing/ospfv3/process/Ospfv3Process.ned) | The `checksumMode` parameter: "'computed' yields RFC 5340 2.5 / RFC 2328 12.1.7 checksums, e.g. for pcap/interop". |
| RFC 5340 | [`interface/Ospfv3Interface.cc:952,1307,1352`](../../../../../src/inet/routing/ospfv3/interface/Ospfv3Interface.cc) | Three comments: "RFC 5340 lets a router pack [LSAs] in any order" inside a Link State Update, and twice "RFC 5340: OSPF header Packet Length" above a length-field assignment. |
| RFC 2328 §13 | [`interface/Ospfv3Interface.cc:1182`](../../../../../src/inet/routing/ospfv3/interface/Ospfv3Interface.cc) | "RFC 2328 Section 13, step 7: the received LSA is not newer than the database". |
| RFC 5340 §4.5.2 | [`interface/Ospfv3Interface.cc:1185`](../../../../../src/inet/routing/ospfv3/interface/Ospfv3Interface.cc) | "procedure from OSPFv2 per RFC 5340 Section 4.5.2." above the acknowledgment logic. |
| RFC 2328 §10.6 | [`process/Ospfv3Area.cc:145`](../../../../../src/inet/routing/ospfv3/process/Ospfv3Area.cc) | A comment above `findLSA` explains why every LSA type is searched, citing the Database Description request-list rule of "RFC 2328 Section 10.6". |
| RFC 5340 Appendix A.4.1 (URL) | [`process/Ospfv3Area.cc:1421`](../../../../../src/inet/routing/ospfv3/process/Ospfv3Area.cc) | A TODO that links `https://tools.ietf.org/html/rfc5340#appendix-A.4.1` for the address-prefix word-alignment rule. |
| RFC 5340 | [`neighbor/Ospfv3Neighbor.cc:275,429,446`](../../../../../src/inet/routing/ospfv3/neighbor/Ospfv3Neighbor.cc) | Three comments: the header Packet Length field, "LSA order within a Link State Update is not significant (RFC 5340...)", and the LSA-count field. |
| RFC 5340 §2.5 | [`process/Ospfv3Process.cc:769`](../../../../../src/inet/routing/ospfv3/process/Ospfv3Process.cc) | "RFC 5340 2.5 checksum over the IPv6 pseudo-header + packet, which needs the addresses known". |

`ch-routing.rst` and the other `doc/src/users-guide/*.rst` chapters do not name OSPFv3
directly; grepping them for `RFC ?[0-9]{3,4}` finds no OSPFv3-specific line, and `WHATSNEW`
records only the historical fact that "a new OSPF protocol implementation has been added
which implements version 3" (`WHATSNEW:2622`), naming no RFC number.

Mapped onto the standards map, [`standards.md`](../../protocol/ospfv3/standards.md#document-list):

| Document | In the in-scope set | Claimed | Note |
| --- | --- | --- | --- |
| RFC 5340 | yes, `base` | **yes**, in source comments at the level of an appendix clause | 25 lines cite "RFC 5340" or "RFC5340" by clause; no NED documentation comment names it, only "Implements the OSPFv3 ... routing protocol for IPv4 and IPv6 networks." |
| RFC 2328 | yes, `companion`, the clauses of the mapping table | **yes**, twice, for the LSA checksum algorithm (§12.1.7) and the Database Description request-list rule (§10.6) | Both citations point into clauses the mapping table of `standards.md` lists as inherited; neither is a stray or obsolete reference. |
| RFC 2740 | no, obsoleted | no | — |
| RFC 6845, RFC 6860, RFC 7503, RFC 8362, RFC 5838, RFC 9454 | no, level 5 | no | No comment, NED documentation line, or users-guide chapter names any of these six documents. |

**The finding of the survey.** The model claims RFC 5340 precisely, in 25 source-comment
lines that cite an appendix clause next to a packet or LSA field, and it claims the two RFC
2328 clauses it reuses correctly, at the right section numbers. Unlike OSPFv2, no claim here
is obsolete or misattributed: OSPFv3 has one base document (no RFC 1583-style predecessor
chain to confuse), and every RFC 2328 citation lands on a clause the standards map's
inheritance table also lists. The gap is elsewhere: RFC 5340 was itself updated by five
documents (RFC 6845, RFC 6860, RFC 7503, RFC 8362, RFC 9454) and has one companion (RFC
5838), and the model claims none of them, even where its own code shape suggests an
attempt — the NSSA area type and the dual IPv4/IPv6 instance split exist structurally
without naming RFC 5838, RFC 6845, or RFC 8362 anywhere.

For the level 2 pass, five facts from the code (the guide permits this look to decide the
claim question, and the pass must check each one):

1. LSA checksum validation is a stub that always accepts, in every LSA class:
   `validateLSChecksum` "return true" with the comment "not implemented", eight times, one
   per LSA type (`process/Ospfv3Lsa.h:67,81,95,109,123,137,151,165`). Router-LSA, Network-LSA,
   Link-LSA and Intra-Area-Prefix-LSA are in the level-2 slice's flooding procedure (RFC 5340
   §4.5); a check that floods one of these with a wrong checksum and expects it discarded
   meets a claimed behavior with a placeholder result: a defect, by the same reasoning
   [`ospfv2/conformance.md`](../ospfv2/conformance.md#part-1--the-claims) records for OSPFv2.
2. The Database Description exchange uses "master"/"slave" (`Ospfv3Neighbor::MASTER`,
   `Ospfv3Neighbor::SLAVE` and their uses in
   [`interface/Ospfv3Interface.cc:614,621,653,682,684,692,694,719,720,749`](../../../../../src/inet/routing/ospfv3/interface/Ospfv3Interface.cc)),
   the same naming RFC 9454 replaces with "Leader"/"Follower" without changing the
   operation. A naming gap for the model's documentation, not a defect.
3. `Ospfv3Process.cc:427` reads "multiarea configuration is not supported" directly above a
   loop, `Ospfv3Process.cc:428-437`, that iterates every `<Area>` element the XML
   configuration lists and adds each one to the instance. The comment and the code disagree;
   a level 2 or later pass should treat the comment as stale rather than as a scope
   statement, since the loop is not gated on a single-area check anywhere nearby.
4. Two named gaps sit inside the routing-table calculation, and both are outside the level-2
   slice already: step 4, rechecking transit areas' summary-LSAs, is commented out
   (`Ospfv3Process.cc:951-961`, "this part of protocol is not supported yet"), and step 5,
   calculating AS external and NSSA routes, is commented out too
   (`Ospfv3Process.cc:963-964`, same wording, with the call to
   `calculateASExternalRoutes(...)` itself inside the comment). Step 3, `calculateInterAreaRoutes`
   for the backbone or a single non-backbone area (`Ospfv3Process.cc:938-949`), is **not**
   commented out and does run; only the transit-area recheck (step 4) and the external/NSSA
   step (step 5) are stubbed. Both stubbed steps belong to RFC 5340 §4.8.4 and §4.8.5,
   already out of the level-2 slice by
   [`standards.md`](../../protocol/ospfv3/standards.md#target-level).
5. The NSSA area type is structurally present — `enum Ospfv3AreaType { NORMAL, STUB,
   TOTALLY_STUBBY, NSSA, NSSA_TOTALLY_STUB }` (`process/Ospfv3Area.h:19-25`), an `NSSA_LSA`
   function code (`Ospfv3Packet.msg:110`) and an `nBit` field on the wire
   (`Ospfv3Packet.msg:46`) — but the
   Type-7-to-Type-5 translation this area type depends on is exactly the stubbed external-route
   step of fact 4. The area type is a claim (code exists: the enum, the wire bit, the LSA
   type) whose dependency is unimplemented; a level 5 pass should read this as a defect in
   the NSSA feature, not as "NSSA not claimed."
