# PIM — model claims

> **Kind:** report · **Status:** snapshot 2026-09-23 · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/pim/standards.md), [coverage.md](coverage.md)

Step 8 artifact of the standards test workflow, part 1 only. A level 1 pass records what the
model intends: which standards it claims to implement, mapped onto the standards map. Part 2,
the conformance matrix, needs the feature map and the verdicts of a level 2 pass.

Read record — no build, no run:

- Date: 2026-09-23
- INET: branch `topic/standards-tests-wave0`, commit `e360ca980e`, tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`

## Part 1 — the claims

Every place in the model that names a standard of the PIM family:

| Claim | Where | What it says |
| --- | --- | --- |
| RFC 3973 | [`PimDm.ned:15`](../../../../../src/inet/routing/pim/modes/PimDm.ned); [`PimDm.h:25`](../../../../../src/inet/routing/pim/modes/PimDm.h) | "Implementation of PIM-DM protocol (RFC 3973)." — the documentation comment of the module and the matching Doxygen comment, word for word. |
| RFC 4601, obsolete | [`PimSm.ned:15`](../../../../../src/inet/routing/pim/modes/PimSm.ned); [`PimSm.h:23`](../../../../../src/inet/routing/pim/modes/PimSm.h); [`Pim.h:31`](../../../../../src/inet/routing/pim/Pim.h) | "Implementation of PIM-SM protocol (RFC 4601)." and, for the compound module, "Compound module for PIM protocol (RFC 4601)." RFC 4601 is obsoleted by RFC 7761; see the finding below. |
| RFC 4601, the time constants | [`PimSm.ned:37`](../../../../../src/inet/routing/pim/modes/PimSm.ned) | "Other parameters set the time constants to the values specified in RFC 4601. They should not be changed except for testing." |
| RFC 7761 | [`PimBase.cc:164`](../../../../../src/inet/routing/pim/modes/PimBase.cc); [`PimBase.cc:260`](../../../../../src/inet/routing/pim/modes/PimBase.cc); [`PimDm.cc:1723`](../../../../../src/inet/routing/pim/modes/PimDm.cc) | Three source comments on the IPv6 link-local addressing rule: "RFC 7761: PIM-over-IPv6 messages are sourced from the interface link-local address" and "RFC 7761/RFC 3973 over IPv6: PIM uses the interface's link-local address". These name the current document where the module documentation of the same mechanism names the obsolete one. |
| RFC 4601, the wire format | [`PimBase.h:186`](../../../../../src/inet/routing/pim/modes/PimBase.h); [`PimPacketSerializer.cc:27`](../../../../../src/inet/routing/pim/PimPacketSerializer.cc) | "RFC 4601 4.9.1 Encoded-Address field lengths" and "Encoded-Address forms (RFC 4601 §4.9.1)" above the address-encoding code. |
| RFC 3973 and RFC 4601, a TODO | [`PimNeighborTable.h:27`](../../../../../src/inet/routing/pim/tables/PimNeighborTable.h) | "TODO add fields for options received in Hello Messages (RFC 3973 4.7.5, RFC 4601 4.9.2)." |
| RFC 3973, by clause | [`PimDm.cc:412`](../../../../../src/inet/routing/pim/modes/PimDm.cc); [`PimDm.cc:1401`](../../../../../src/inet/routing/pim/modes/PimDm.cc) | "See RFC 3973 4.4.2.2" above the Prune-Pending Timer expiry handler, and "RFC 3973 4.5.2.2" above the data-packet-received handler of State Refresh. |
| RFC 3973, State Refresh | [`PimPacket.msg:37`](../../../../../src/inet/routing/pim/PimPacket.msg) | "StateRefresh = 9; // in RFC 3973" in the `PimPacketType` enum. |
| RFC 3956, embedded-RP | [`PimSm.cc:87,90,1952`](../../../../../src/inet/routing/pim/modes/PimSm.cc) | "embedded-RP groups (RFC 3956) carry their RP in the group address", a matching warning message, and "RFC 3956 embedded-RP: a group in the FF7x::/12 range carries its RP in the group address itself." |
| RFC 3956, the release notes | [`WHATSNEW:500-501`](../../../../../WHATSNEW) | "PIM-SM can also derive the rendezvous point per group from embedded-RP IPv6 addresses (RFC 3956)." |
| RFC 4607, source-specific multicast | [`PimSm.cc:1214,1224,1230`](../../../../../src/inet/routing/pim/modes/PimSm.cc); [`PimBase.cc:387`](../../../../../src/inet/routing/pim/modes/PimBase.cc) | "Source-specific multicast (RFC 4607): only SSM-range groups are served", "SSM is INCLUDE-only (RFC 4607)", a matching warning message, and "RFC 4607 source-specific multicast range: IPv4 232.0.0.0/8, IPv6 FF3x::/32." |
| RFC 4607, the release notes | [`WHATSNEW:501-505`](../../../../../WHATSNEW) | "Source-specific multicast (SSM, RFC 4607) is now fully supported on both address families ... using INCLUDE-only semantics and requiring no rendezvous point." |
| RFC 2460, obsolete | [`PimPacket.msg:130`](../../../../../src/inet/routing/pim/PimPacket.msg); [`Pim.cc:45`](../../../../../src/inet/routing/pim/Pim.cc) | Two identical comments: "For IPv6, the checksum also includes the IPv6 'pseudo-header', as specified in RFC 2460, Section 8.1 [5]." RFC 2460 is obsoleted by RFC 8200; see the finding below. This document is not part of the PIM family, it is cited only for the IPv6 pseudo-header layout. |
| a stated limitation | [`PimSm.ned:40-44`](../../../../../src/inet/routing/pim/modes/PimSm.ned) | "Limitations: Only one global RP is supported and it must be specified statically. PIM Bootstrap and RP discovery are not yet implemented. Switchover to the shortest path tree is not supported. Source specific excludes in the shared tree are not supported." Three items, not two; see the facts below. |

No comment, NED documentation, or line of the user's guide names RFC 8736, RFC 9436 or RFC 5059:
a tree-wide search for each number returns nothing outside this table. No line of
`doc/src/users-guide/*.rst` names PIM by an RFC number at all; the one PIM mention in the user's
guide is a bare cross-reference to the protocol name (`ch-networks.rst:58`).

Mapped onto the standards map, [`standards.md`](../../protocol/pim/standards.md#document-list):

| Document | In the in-scope set | Claimed | Note |
| --- | --- | --- | --- |
| RFC 7761 | yes, `base` | **yes**, in source comments only | Three comments name it correctly for the IPv6 link-local addressing rule. No NED documentation comment names it; the module documentation of the same module names the obsolete RFC 4601 instead. |
| RFC 4601 | no, obsolete | **yes**, in the published documentation | **An obsolete claim.** `PimSm.ned:15` and `PimSm.h:23`, the two places a reader of the module reference looks first, name RFC 4601; `Pim.h:31` names it for the compound module too. RFC 7761 obsoleted it in 2016. |
| RFC 3973 | yes, `base` | **yes**, in the published documentation and at the level of clauses | `PimDm.ned:15`, `PimDm.h:25`, and comments that name §4.4.2.2 and §4.5.2.2. PIM-DM has no successor document, so this claim is not obsolete. |
| RFC 3956 | yes, `companion` | **yes**, in source comments and the release notes | No NED documentation comment names it; `PimSm.cc` and `WHATSNEW` do. |
| RFC 4607 | yes, `companion` | **yes**, in source comments and the release notes | Same pattern as RFC 3956. |
| RFC 9436, RFC 8736 | no, level 3 | no | — |
| RFC 5059 | no, level 5 | **declined, in the published documentation** | `PimSm.ned:41`: "PIM Bootstrap and RP discovery are not yet implemented." A stated limitation, not a bare TODO; see the facts below. |
| RFC 2460 | no, not part of the family, obsolete | **yes**, twice, for one clause | **A second obsolete claim.** Both citations name RFC 2460 §8.1 for the IPv6 pseudo-header; RFC 8200 obsoleted it in 2017 without renumbering that clause. |
| RFC 8200 | no, not part of the family | no | Nothing names the current document for the same clause. |

**The finding of the survey.** The model claims both base documents, and it claims them
inconsistently within the same source tree. The two places a reader of the module reference
looks first — `PimSm.ned`'s documentation comment and `PimSm.h`'s Doxygen comment, plus the
compound module's own comment in `Pim.h` — name RFC 4601, obsoleted by RFC 7761 in 2016. Three
source comments inside `PimBase.cc` and `PimDm.cc`, which a reader of the generated
documentation does not see, correctly name RFC 7761 for the same protocol. PIM-DM's claim has no
such split: `PimDm.ned` and `PimDm.h` both name RFC 3973, which has no successor.

A second, unrelated obsolete claim sits in the checksum code: two identical comments cite
RFC 2460 for the IPv6 pseudo-header layout, and RFC 2460 was obsoleted by RFC 8200 in 2017. This
citation is outside the PIM family — it is a general IPv6 citation borrowed for a checksum
procedure — but it is still a citation of a superseded document, the same shape of finding as the
RFC 4601 claim, so this survey counts two obsolete claims for PIM, not one.

## Facts for the level 2 pass

1. **`PimSm.ned`'s "Limitations:" section states three omissions, not two.** Besides the two the
   survey lead named — no switch to the shortest-path tree, and no source-specific excludes on
   the shared tree — a third sentence precedes them: "Only one global RP is supported and it
   must be specified statically. PIM Bootstrap and RP discovery are not yet implemented."
   (`PimSm.ned:40-41`). All three are stated limitations in the module's own published
   documentation, not code comments, so all three are declarable absences by the third
   principle of the guide: a check of any of them fails, and the failure is expected.
2. **The Bootstrap and Candidate-RP-Advertisement message types have a reserved type code and a
   receive path, but no message class.** `PimPacket.msg:32,36` reserves `Bootstrap = 4` and
   `CandidateRPAdvertisement = 8` in the `PimPacketType` enum, and `PimSm.cc:249-254` has a
   `case` for each: `case Bootstrap: delete pk; break;` and the same for
   `CandidateRPAdvertisement`. Neither type has a dedicated message class in `PimPacket.msg`
   (compare `PimHello`, `PimJoinPrune`, `PimAssert`, `PimGraft`, `PimStateRefresh`,
   `PimRegister`, `PimRegisterStop`, all of which do). A packet of either type is received,
   recognized by its type code, and discarded unconditionally, with no parse. This matches the
   stated limitation of fact 1 — "PIM Bootstrap and RP discovery are not yet implemented" — so a
   level 2 or later check that sends a Bootstrap message and observes silence tests a declared
   absence, and one that expects a Candidate-RP-Advertisement reply has no message class to
   build the stimulus with: an **untestable claim** only if the module documentation had not
   already declared the absence; here it is a plain declared absence instead.
3. **RFC 7761 §4.7 makes static RP configuration the one mandatory group-to-RP mapping
   mechanism** ("A PIM router MUST support the static configuration of group-to-RP mappings",
   `rfc7761.txt:5477-5478`) and lists Bootstrap, embedded-RP and Cisco's Auto-RP as three
   alternatives it does not mandate. The model implements the mandatory one (a single global
   `RP` parameter) and one of the three alternatives (embedded-RP); this is consistent scope,
   not a partial claim.
4. **RFC 2460 is not a PIM-family document**, so it does not appear in
   [`standards.md`](../../protocol/pim/standards.md)'s in-scope or out-of-scope tables by that
   test alone; it is recorded here because the citation names a superseded document, which is
   the level 1 headline this pass looks for regardless of which family the document belongs to.
