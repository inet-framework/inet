# MLD — model claims

> **Kind:** report · **Status:** snapshot 2026-09-23 · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/mld/standards.md), [coverage.md](coverage.md)

Step 8 artifact of the standards test workflow, part 1 only. A level 1 pass records what the
model intends: which standards it claims to implement, mapped onto the standards map. Part 2,
the conformance matrix, needs the feature map and the verdicts of a level 2 pass.
[`igmp/conformance.md`](../igmp/conformance.md) records the parallel claims for the IPv4 side;
`Mldv2` is a documented, same-month port of `Igmpv3` (`WHATSNEW:490-491`: "The IPv4 IGMPv3
model was brought to lockstep parity with MLDv2... so that the IPv4 and IPv6 multicast
membership implementations now mirror each other"), and the notes below say where the two
still diverge.

Read record — no build, no run:

- Date: 2026-09-23
- INET: branch `topic/standards-tests-wave0`, commit `e360ca980e`, tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`

## Part 1 — the claims

Every place in the model that names a document of the MLD family. Source:
`src/inet/networklayer/icmpv6/{Mldv1,Mldv2,IMld,MldMessage,Mldv2Message}.{cc,h,ned,msg}`,
`doc/src/users-guide/ch-ipv6.rst`, `WHATSNEW`. `Mldv1.ned` and `Mldv2.ned` both carry a 2026
copyright header — this family is much newer code than ND or IGMP.

| Claim | Where | What it says |
| --- | --- | --- |
| RFC 2710 | [`Mldv1.ned:12`](../../../../../src/inet/networklayer/icmpv6/Mldv1.ned) | "Implements MLDv1 (RFC 2710): Multicast Listener Discovery for IPv6." The module documentation comment. |
| RFC 3810 | [`Mldv2.ned:12`](../../../../../src/inet/networklayer/icmpv6/Mldv2.ned) | "Implements MLDv2 (RFC 3810): Multicast Listener Discovery version 2 for IPv6." The module documentation comment. |
| RFC 2710, RFC 2236 | [`IMld.ned:14`](../../../../../src/inet/networklayer/icmpv6/IMld.ned) | "MLD is the IPv6 equivalent of IGMP; MLDv1 (RFC 2710) parallels IGMPv2 (RFC 2236)." The shared module interface documentation. |
| RFC 2710 | [`Mldv1.ned:24,38,44`](../../../../../src/inet/networklayer/icmpv6/Mldv1.ned) | Section-numbered subheadings: "Host behavior (RFC 2710 Section 5)", "Router (querier) behavior (RFC 2710 Section 6)", and a mid-paragraph section reference. |
| RFC 2710 | [`Mldv1.cc:252,353,383,461,486,495,700`](../../../../../src/inet/networklayer/icmpv6/Mldv1.cc) | Seven inline comments citing an RFC 2710 section — the all-nodes exemption (§5), the response-delay encoding (§3.4), report suppression (§5), the hop limit (§3). |
| RFC 2710 | [`Mldv1.h:29,31,38,74,76`](../../../../../src/inet/networklayer/icmpv6/Mldv1.h) | Five comments naming the host state names (§5) and the router state names (§6). |
| RFC 2710 | [`MldMessage.msg:16,22,29,40,49`](../../../../../src/inet/networklayer/icmpv6/MldMessage.msg) | Five comments on the base MLDv1 message class and its three message types. |
| RFC 3810 | [`Mldv2.ned:28,41`](../../../../../src/inet/networklayer/icmpv6/Mldv2.ned) | Section-numbered subheadings: "Host behavior (RFC 3810 Section 5/6)", "Router (querier) behavior (RFC 3810 Section 7)". |
| RFC 3810 | [`Mldv2.h:36,40,80,99,150,158,214,313,330`](../../../../../src/inet/networklayer/icmpv6/Mldv2.h) | Nine comments naming a timer kind, a compatibility field, or the Robustness Variable. |
| RFC 3810 | [`Mldv2.cc`](../../../../../src/inet/networklayer/icmpv6/Mldv2.cc), 32 lines: `66,173,180,194,260,490,502,564,611,643,681,692,709,718,731,831,845,854,895,1093,1095,1185,1206,1270,1279,1308,1333,1367,1390,1419,1607,1764` | Inline comments citing an RFC 3810 section above the code that implements it — report generation, retransmission, MLDv1 interoperation (§8.2/§8.3), forwarding rules (§7.2). |
| RFC 3810 | [`Mldv2Message.msg:17,32,36,51,63`](../../../../../src/inet/networklayer/icmpv6/Mldv2Message.msg) | Five comments on the Multicast Address Record type and the query/report message layouts. |
| none | [`ch-ipv6.rst`](../../../../../doc/src/users-guide/ch-ipv6.rst) | The IPv6 user's guide never mentions MLD, Multicast Listener Discovery, `Mldv1` or `Mldv2` at all (`grep -ci 'mld\|listener discovery' ch-ipv6.rst` returns 0). Unlike IGMP, which at least gets an undocumented-RFC-number section, MLD gets no user-facing prose at all. |
| RFC 3810 | [`WHATSNEW:486`](../../../../../WHATSNEW) | "a new Mldv2 module implementing MLDv2 (RFC 3810) was added." |

No line anywhere in the tree names RFC 4604 or RFC 3590
(`grep -rn 'RFC ?4604\|RFC ?3590' src/inet/networklayer/icmpv6/` finds nothing).

Mapped onto the standards map, [`standards.md`](../../protocol/mld/standards.md#document-list):

| Document | In the in-scope set | Claimed | Note |
| --- | --- | --- | --- |
| RFC 9777 | yes, `base` | **no** | Nothing in the model names RFC 9777. Promoted to Internet Standard in March 2025, the same month as RFC 9776; nothing in this 2026-dated code has been touched since. |
| RFC 3810 | no, obsoleted | **yes**, in the module documentation of `Mldv2` and in 32 inline `.cc` section citations, plus `.h` and `.msg` comments | **The headline obsolete claim.** `Mldv2.ned:12` is the module reference's own description; RFC 9777 obsoleted RFC 3810 seven months before this read date. This is the same shape of finding [`igmp/conformance.md`](../igmp/conformance.md) records for `Igmpv3` against RFC 3376/RFC 9776 — the two protocols drifted from their governing documents together, as the brief for this pass expected. |
| RFC 2710 | yes, `companion` | **yes**, in the module documentation of `Mldv1`, the shared interface file, and 17 further comments across `.h`, `.cc` and `.msg` | Current and correctly cited; RFC 2710 is not obsoleted, so this claim is not a register gap — the same position RFC 2236 is in for IGMP. |
| RFC 4604 | no, level 5 | no | Not named anywhere in `src/inet/networklayer/icmpv6/`. No SSM-address-range check exists in either module. |
| RFC 3590 | no, level 5 / edge | no | Not named anywhere. No MLD source-address-selection logic exists to check it against. |

**The finding of the survey.** MLDv1 is claimed correctly and consistently, the same way
IGMPv2 is: RFC 2710 is not obsoleted, and every citation across five files names it. MLDv2 is
claimed only as RFC 3810, which RFC 9777 obsoleted on 2025-03-11; the claim appears in the
module reference (`Mldv2.ned:12`) and in 32 inline section citations, and it is never
corrected anywhere in the tree — not the NED file, not `WHATSNEW`, and the user's guide does
not mention MLD at all, which is a larger documentation gap than IGMPv3's (IGMPv3 at least
gets an unlabeled user-guide section; MLD gets none).

For the level 2 pass, four facts from the code (the guide permits this look to decide the
claim question, and the pass must check each one), kept parallel to the four facts in
[`igmp/conformance.md`](../igmp/conformance.md):

1. **`Mldv1` crashes on an unrecognized message type; `Mldv2` does not.** `Mldv1.cc:342`
   `throw cRuntimeError("Mldv1: Unknown MLD message type %d", ...)`, against RFC 9777's MUST
   (`rfc9777.txt:765`). `Mldv2.cc:703` instead logs a warning and drops the packet
   (`EV_WARN << "Mldv2: unexpected MLD message type "...; delete packet`) — already conformant.
   This is the opposite pattern from IGMP, where **both** `Igmpv2` and `Igmpv3` crash on the
   same case (see [`igmp/conformance.md`](../igmp/conformance.md)); the newer MLDv2 module was
   apparently written past this defect while the older MLDv1 module was not, and neither IGMP
   module was.
2. **No MLD message ever carries a Router Alert option, and no comment names the gap.**
   `grep -in 'router alert' Mldv1.cc Mldv2.cc` finds nothing at all — not even a TODO — against
   a requirement RFC 9777 states as a formal MUST (`rfc9777.txt:738-741`) and RFC 2710 already
   stated descriptively (`rfc2710.txt:83-85`). Both modules do set the Hop Limit to 1 correctly
   (`Mldv1.cc:495`, `Mldv2.cc:1429`). Unlike IGMP's three `// TODO fill Router Alert option`
   comments (see [`igmp/conformance.md`](../igmp/conformance.md)), this is a silent gap — the
   model does not admit it anywhere, which the guide's TODO table treats no differently from a
   named one: the code exists (option-free packet construction), so this is still a claim, and
   still a defect if a level 2 check reaches it.
3. **The `Mldv2.ned` "Parity gaps" comment (`Mldv2.ned:56-61`) is stale on all three of its own
   points, not just the one the survey lead flagged.** It reads: "state-change Reports are sent
   once, not retransmitted [Robustness Variable]-1 more times; Multicast-Address-Specific
   Queries after a leave are not retransmitted `lastMemberQueryCount` times; and MLDv1<->MLDv2
   interoperation... is not implemented." `git blame` dates this comment to the first commit of
   the file, `c4b596331a7` (2026-06-17 10:44), when all three gaps were true. Two later commits
   the same family closed all three: `c325ab8338f` (2026-06-17 21:42) added both retransmission
   loops (`Mldv2.cc:270`: `groupData->retransmitCount = robustnessVariable - 1`; `Mldv2.cc:1337,
   1397`: `groupData->rexmtCount = lastMemberQueryCount - 1`, each guarded by `if (...count > 0)
   startTimer(...)`), and `16c75c1a3c` (2026-06-18, "MLDv1 <-> MLDv2 interop") wired
   `processOlderVersionQuery`/`processOlderVersionReport`/`processOlderVersionDone` into the
   live dispatch (`Mldv2.cc:683,695,699`, inside `processMldMessage`) — confirmed reachable, not
   dead code. Both commits are
   ancestors of the commit this pass reads (`git merge-base --is-ancestor` confirms it). The NED
   comment was never updated after either fix. This is the mirror image of the level 2 pass's
   usual finding: here the code is more capable than the documentation admits, on all three
   named points at once.
4. **The Multicast Address Listening Interval default in the NED file has not been checked
   against RFC 9777's changed text** (see the override table of
   [`standards.md`](../../protocol/mld/standards.md#override-table)). `Mldv1.ned:71` and
   `Mldv2.ned:76` both default `multicastListenerInterval`/`groupMembershipInterval` to
   `(robustnessVariable * queryInterval) + queryResponseInterval` — one Query Response Interval,
   matching RFC 2710/RFC 3810's superseded formula, not RFC 9777's "plus 2 times" text. The same
   fact holds for IGMP's `groupMembershipInterval` default, confirming the two protocols share
   this gap as well as the formula it is measured against.
