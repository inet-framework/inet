# OSPFv2 — model claims

> **Kind:** report · **Status:** snapshot 2026-09-23 · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/ospfv2/standards.md), [coverage.md](coverage.md)

Step 8 artifact of the standards test workflow, part 1 only. A level 1 pass records what the
model intends: which standards it claims to implement, mapped onto the standards map. Part 2,
the conformance matrix, needs the feature map and the verdicts of a level 2 pass.

Read record — no build, no run:

- Date: 2026-09-23
- INET: branch `topic/standards-tests-wave0`, commit `e360ca980e`, tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`

## Part 1 — the claims

Every place in the model that names a standard of the OSPFv2 family, found with
`grep -rn -E 'RFC ?[0-9]{3,4}|draft-' src/inet/routing/ospfv2 src/inet/routing/ospf_common
src/inet/node/ospfv2 --include=*.ned --include=*.cc --include=*.h --include=*.msg`, excluding
generated `*_m.cc`/`*_m.h`, plus the users-guide:

| Claim | Where | What it says |
| --- | --- | --- |
| none | [`Ospfv2.ned:14`](../../../../../src/inet/routing/ospfv2/Ospfv2.ned) | "Implements the OSPFv2 routing protocol." This is the documentation comment of the module, which the module reference publishes, and it names no document. |
| RFC 3101 | [`Ospfv2.ned:41`](../../../../../src/inet/routing/ospfv2/Ospfv2.ned) | "`@RFC1583Compatible` - optional true or false value (defined in RFC 3101)", in the XML configuration documentation. |
| RFC 3101 | [`Ospfv2.ned:159`](../../../../../src/inet/routing/ospfv2/Ospfv2.ned) | The `RFC1583Compatible` module parameter: "If 'false', prune the set of routing table entries for the ASBR (RFC 3101)". |
| RFC 1583 (named) | [`Ospfv2Router.h:44`](../../../../../src/inet/routing/ospfv2/router/Ospfv2Router.h) | The `rfc1583Compatibility` field: "Decides whether to handle the preferred routing table entry to an AS boundary router as defined in RFC1583 or not." |
| RFC 2328 | [`Ospfv2Checksum.cc:30`](../../../../../src/inet/routing/ospfv2/Ospfv2Checksum.cc) | "RFC 2328: OSPF checksum is calculated over the entire OSPF packet, excluding the 64-bit authentication field." |
| RFC 1583 §A.4.2 | [`Ospfv2Packet.msg:120,128,141`](../../../../../src/inet/routing/ospfv2/Ospfv2Packet.msg) | Three comments: "(RFC 1583 Section A.4.2.)" above the TOS-data structure, the Router-LSA link section, and the Router-LSA class itself. |
| RFC 2328 | [`Ospfv2Router.h:28`](../../../../../src/inet/routing/ospfv2/router/Ospfv2Router.h) | "Represents the full OSPF data structure as laid out in RFC2328." |
| RFC 2328 §14, §13.3, §11.1, §16, §16.4, Appendix E, §12.4 | [`Ospfv2Router.h:136,163,194,203,253,265,313,326,336,358,361`](../../../../../src/inet/routing/ospfv2/router/Ospfv2Router.h) | Eleven `@sa`/prose comments that cite a clause of RFC 2328 above a routing-table-calculation method. |
| RFC 2328 §13 | [`LinkStateUpdateHandler.cc:37`](../../../../../src/inet/routing/ospfv2/messagehandler/LinkStateUpdateHandler.cc) | "@see RFC2328 Section 13." above the Link State Update handler. |
| RFC 2328 §13.3 | [`Ospfv2Interface.cc:335`](../../../../../src/inet/routing/ospfv2/interface/Ospfv2Interface.cc) | "@see RFC2328 Section 13.3." |
| RFC 2328 §8.2 | [`MessageHandler.cc:187`](../../../../../src/inet/routing/ospfv2/messagehandler/MessageHandler.cc) | "// see RFC 2328 8.2" |
| RFC 2328 §12.4.4.1, §3.6, §16.3, §16.4, §16.7 | [`Ospfv2Router.cc:153,420,887,941,1130`](../../../../../src/inet/routing/ospfv2/router/Ospfv2Router.cc) | Five comments that cite a clause of RFC 2328 above the routing-table calculation and the stub-area check. |
| RFC 2328 §16.2 | [`Ospfv2Area.cc:2264`](../../../../../src/inet/routing/ospfv2/router/Ospfv2Area.cc) | "@see RFC 2328 Section 16.2." |

The `RFC1583Compatible` parameter carries two different citations for the same flag, five
lines and one file apart: the NED documentation names RFC 3101 twice
(`Ospfv2.ned:41,159`), and the C++ field comment names RFC 1583
(`Ospfv2Router.h:44`). Neither is the document that actually governs the flag: by
[`standards.md`](../../protocol/ospfv2/standards.md#document-list), the parameter is defined
in RFC 2328 Appendix C.1 and Appendix G, as a choice between the path-preference rule of
RFC 1583 and the loop-preventing rule RFC 2328 §16.4.1 adds. RFC 3101 defines the NSSA area
type, an unrelated mechanism; RFC 1583 is Historic, and its text is folded into RFC 2328.
The C++ comment names the right family member but not the document that governs; the NED
comment names the wrong document outright.

No claim in the model, the NED files, or [`ch-routing.rst`](../../../../../doc/src/users-guide/ch-routing.rst)
names RFC 6549, RFC 6845, RFC 6860, RFC 7474, RFC 8042, RFC 9355, RFC 9454, RFC 5709, or
RFC 3101 for anything other than the `RFC1583Compatible` misattribution above.
`ch-routing.rst:107` states "The `Ospfv2` module implements OSPF protocol version 2" and
names no RFC number; the only RFC-shaped text in that chapter's OSPF section is the
`RFC1583Compatible` XML attribute name in the example, which repeats the parameter and
claims nothing new.

Mapped onto the standards map, [`standards.md`](../../protocol/ospfv2/standards.md#document-list):

| Document | In the in-scope set | Claimed | Note |
| --- | --- | --- | --- |
| RFC 2328 | yes, `base` | **yes**, in source comments at the level of clauses | 22 lines cite "RFC2328" or "RFC 2328" by section; no NED documentation comment names it, only the module doc string "Implements the OSPFv2 routing protocol." |
| RFC 1583 | no, obsoleted | named, twice | **An obsolete claim**, in two different and inconsistent ways: `Ospfv2Packet.msg` names it for the Router-LSA wire format that RFC 2328 §A.4.2 defines (RFC 2328 restates the same layout unchanged), and `Ospfv2Router.h:44` names it for the `RFC1583Compatible` flag, which RFC 2328 Appendix C.1 defines and Appendix G explains. |
| RFC 2178 | no, obsoleted | no | — |
| RFC 3101 | no, level 5, companion | named, **wrongly** | `Ospfv2.ned:41,159` cites RFC 3101 for the `RFC1583Compatible` parameter. RFC 3101 defines the NSSA area type; the model has no NSSA area type at all (0 hits for "nssa", case-insensitive, in `src/inet/routing/ospfv2`). The citation is unrelated to what the flag does and unrelated to any code in the file. |
| RFC 5709, RFC 7474, RFC 6549, RFC 6845, RFC 6860, RFC 8042, RFC 9355, RFC 9454 | no, level 5 | no | — |

**The finding of the survey.** The model claims RFC 2328 precisely and often, in source
comments that cite a clause number next to a routing-table or a flooding method — 22 such
lines. The one place a reader looks first, the module's NED documentation comment, names no
RFC number at all. Two claims are obsolete or wrong. `Ospfv2Packet.msg` names RFC 1583 for a
Router-LSA layout that RFC 2328 restates unchanged in the same appendix number (A.4.2), so
the citation is stale but not incorrect in substance. The `RFC1583Compatible` parameter is
the sharper case: its own NED documentation cites RFC 3101, a document about a different
area type that the parameter has nothing to do with, while the C++ field that backs the same
parameter correctly associates it with RFC 1583's name, one file and a few lines away from
the wrong citation.

For the level 2 pass, four facts from the code (the guide permits this look to decide the
claim question, and the pass must check each one):

1. Packet-level authentication is a stub that always accepts: `authenticatePacket` "return
   true" (`MessageHandler.h:51-52`, marked "Authentication not implemented"), called from
   the receive path (`MessageHandler.cc:242`). RFC 2328 Appendix D exists on the wire
   (`authenticationType` and an 8-byte `authentication` field, `Ospfv2Packet.msg:53-54`,
   verified) and in the field, so a check that crafts a failing authentication
   type meets code that claims the behavior and gets it wrong: a defect, not an unimplemented
   feature, once a level 3 or later pass reaches it.
2. LSA checksum validation is the same kind of stub, in four places, one per LSA class in
   scope and out of scope alike: `RouterLsa::validateLSChecksum`, `NetworkLsa::validateLSChecksum`,
   `SummaryLsa::validateLSChecksum`, and `AsExternalLsa::validateLSChecksum` all "return true"
   with the comment "not implemented" (`Lsa.h:83,99,121,143`). `LinkStateUpdateHandler.h:31`
   has the same stub at the handler level. The Router-LSA and Network-LSA classes are in the
   level-2 slice's flooding procedure (RFC 2328 §13), so a check that floods an LSA with a
   wrong checksum and expects it discarded meets a claimed behavior with a placeholder
   result: a defect.
3. The Database Description exchange still uses the "master"/"slave" names RFC 2328 uses
   (`Ospfv2Neighbor.h:61-62`, `Ospfv2Neighbor.cc:216`, `DatabaseDescriptionHandler.cc:54,85,112,114,121,123,144,164,221`).
   RFC 9454 renames these to "Leader"/"Follower" without changing the operation
   ([`standards.md`](../../protocol/ospfv2/standards.md#document-list)); this is a naming
   gap for the model's documentation, not a defect, and a level 2 check may use either name
   for the same relationship.
4. The `RFC1583Compatible` parameter (`Ospfv2ConfigReader.cc:73-74`) is read from the XML
   configuration and forwarded to `Ospfv2Router::setRFC1583Compatibility`, and
   `Ospfv2Router.cc:910,1008` branch on it inside the AS-external route preference
   calculation (RFC 2328 §16.4). The flag itself is implemented; only its two citations are
   wrong or stale. Since §16.4 is out of the level-2 slice
   ([`standards.md`](../../protocol/ospfv2/standards.md#target-level)), a check of this flag
   is level 5 work, not level 2.
