# MLD — model claims and conformance matrix

> **Kind:** report · **Status:** snapshot 2026-09-24 · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/mld/standards.md), [features.md](../../protocol/mld/features.md), [coverage.md](coverage.md)

Step 8 artifact of the standards test workflow. Part 1 records what the model intends: which
standards it claims to implement, mapped onto the standards map. Part 2 crosses the claims
with the feature support of the ledger, feature by feature.
[`igmp/conformance.md`](../igmp/conformance.md) records the parallel claims for the IPv4 side.

Run record of the ledger state that part 2 comes from:

- Date: 2026-09-29 18:33 +0200
- INET: branch `topic/standards-tests-igmp-mld-level2-fixes`, commit `ddea7a391b`, tree clean
- Trees: src `e76d92f3af`, tests/protocol `36c0e0673d`
- OMNeT++: 6.4.0, commit `cf58891643`
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-34-generic x86_64
- Command: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/mld$'`

Part 1 was first read by the level 1 pass, at src `16dc528e10`, commit `e360ca980e` of the wave 0
branch. The MLD code is the same in `5c4f41c600`: the two trees differ only in seven files of
IEEE 802.11. The line numbers of part 1 therefore hold for both.
The repairs of this branch change the MLD code, so part 1 describes the code before them.

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

The level 2 pass checked the four facts; [`results.md`](results.md#the-four-facts-of-the-level-1-look)
has the outcome of each.

## Part 2 — the conformance matrix

A feature is `claimed` when the claims of part 1 cover its governing source document. The
model claims RFC 3810 for MLDv2, and RFC 9777 is the same protocol raised to Internet Standard,
with the changes of the [override table](../../protocol/mld/standards.md#override-table); the
claim of RFC 3810 therefore covers every feature that RFC 3810 already had, and the changed
interval was a gap of its own, which the repairs closed, [gap 6](results.md#gap-6-defect--two-default-intervals-have-the-values-of-older-documents). The model claims RFC 2710 as a whole, so the two version 1
features are claimed. One feature is not claimed: MLD-F-SSM-AWARE comes from the SSM text of RFC
4604, and the model names RFC 4604 nowhere. The support comes from
[`coverage.md`](coverage.md#feature-support).

| Feature | Level | Claimed | Support | Verdict |
| --- | --- | --- | --- | --- |
| [MLD-F-MESSAGE-FORMAT](../../protocol/mld/features.md#mld-f-message-format) | mandatory | yes | supported | `confirmed` |
| [MLD-F-QUERY-FORMAT](../../protocol/mld/features.md#mld-f-query-format) | mandatory | yes | supported | `confirmed` |
| [MLD-F-REPORT-FORMAT](../../protocol/mld/features.md#mld-f-report-format) | mandatory | yes | supported | `confirmed` |
| [MLD-F-MESSAGE-VALIDATION](../../protocol/mld/features.md#mld-f-message-validation) | mandatory | yes | partial | `partial` — no core check fails; RFC9777-GEN-8, QRY-6, QRY-20, QRY-21, QRY-27, QRY-30, REP-6, REP-16, REP-17, REP-18, REP-33, REP-35, REP-42, VER-2, LQRY-1, RREP-1, RQRY-1, RFC2710-NODE-6, ROUTER-2, ROUTER-13, ROUTER-14, ROUTER-16 are later |
| [MLD-F-STATE-CHANGE-REPORT](../../protocol/mld/features.md#mld-f-state-change-report) | mandatory | yes | supported | `confirmed` |
| [MLD-F-REPORT-RETRANSMISSION](../../protocol/mld/features.md#mld-f-report-retransmission) | mandatory | yes | partial | `partial` — no core check fails; RFC9777-LSN-20, LSN-21 are owed |
| [MLD-F-QUERY-RESPONSE](../../protocol/mld/features.md#mld-f-query-response) | mandatory | yes | partial | `partial` — no core check fails; RFC9777-LQRY-5, LTIM-9 are owed |
| [MLD-F-GENERAL-QUERY](../../protocol/mld/features.md#mld-f-general-query) | mandatory | yes | supported | `confirmed` |
| [MLD-F-QUERIER-ELECTION](../../protocol/mld/features.md#mld-f-querier-election) | mandatory | yes | supported | `confirmed` |
| [MLD-F-LISTENING-STATE](../../protocol/mld/features.md#mld-f-listening-state) | mandatory | yes | partial | `partial` — no core check fails; RFC9777-RREP-18, RSW-2 are owed |
| [MLD-F-FORWARDING](../../protocol/mld/features.md#mld-f-forwarding) | mandatory | yes | partial | `partial` — no core check fails; RFC9777-FWD-6, FWD-7 are owed |
| [MLD-F-STATE-CHANGE-PROCESSING](../../protocol/mld/features.md#mld-f-state-change-processing) | mandatory | yes | partial | `partial` — no core check fails; RFC9777-RREP-24, RREP-25, RREP-26 are owed; RFC9777-RREP-23 is later |
| [MLD-F-SPECIFIC-QUERIES](../../protocol/mld/features.md#mld-f-specific-queries) | mandatory | yes | supported | `confirmed` |
| [MLD-F-QUERY-TIMER-UPDATES](../../protocol/mld/features.md#mld-f-query-timer-updates) | mandatory | yes | partial | `partial` — no core check fails; RFC9777-RQRY-5, QRY-12 are owed |
| [MLD-F-LISTENER-COMPATIBILITY](../../protocol/mld/features.md#mld-f-listener-compatibility) | mandatory | yes | supported | `confirmed` |
| [MLD-F-VERSION-1-LISTENER](../../protocol/mld/features.md#mld-f-version-1-listener) | mandatory | yes | partial | `partial` — no core check fails; RFC2710-NODE-18, NODE-23 are owed |
| [MLD-F-ROUTER-COMPATIBILITY](../../protocol/mld/features.md#mld-f-router-compatibility) | mandatory | yes | partial | `partial` — no core check fails; RFC9777-COMPR-17, TIMER-19 are owed |
| [MLD-F-VERSION-1-ROUTER](../../protocol/mld/features.md#mld-f-version-1-router) | mandatory | yes | partial | `partial` — no core check fails; RFC2710-ROUTER-27, ROUTER-30, ROUTER-32, ROUTER-34, ROUTER-35, ROUTER-36, ROUTER-37, ROUTER-38, ROUTER-39 are owed |
| [MLD-F-SSM-AWARE](../../protocol/mld/features.md#mld-f-ssm-aware) | optional | no | untested | `out of claim` — level 5 |
| [MLD-F-TIMER-CONFIGURATION](../../protocol/mld/features.md#mld-f-timer-configuration) | mandatory | yes | partial | `partial` — no core check fails; RFC9777-TIMER-1, RFC2710-TIMER-1 are owed; RFC2710-TIMER-4 is later |

Eight features are `confirmed`, eleven `partial` and one `out of claim`. In the level 2 run, two
were `confirmed`, fifteen `partial` and two `defect`.

### How to read the matrix

- **The two `defect` verdicts of the level 2 run are gone.** MLD-F-MESSAGE-VALIDATION failed
  its one core statement on the normal path, RFC2710-NODE-2, because the MLDv1 node could not
  read an MLDv2 Query ([gap 16](results.md#gap-16-defect--the-mldv1-node-stops-the-simulation-on-an-mldv2-query)).
  MLD-F-ROUTER-COMPATIBILITY had its querier half missing ([gap 17](results.md#gap-17-missing-feature--no-mldv1-mode-in-an-mldv2-router))
  and its other half unreached, because gap 16 stopped every run with an MLDv1 host. After the
  repairs both are `partial`: every core statement that runs passes, and the others are owed or
  need crafted messages.
- **The eleven `partial` verdicts** fail no core check; each has a core statement that is owed
  or needs a later level. In the level 2 run, thirteen partial features held a failing core
  check, and the IPv6-specific gaps 2 to 5 reached every MLD message.
- **The twins of IGMP**: every IGMP gap except gap 2, the precedence, for which MLD has no rule,
  had a twin here, with the same code shape in `Mldv2`; the MLD module is a port of `Igmpv3`,
  the port carried the defects with it, and the repairs are twins too.

## Headlines for the next pass

1. **The receiver rules are the next MLD checks.** Since the repairs of gaps 2 and 3, every MLD
   message has the source, the Hop Limit and the Router Alert option that a receiver checks; the
   checks of those rules need crafted messages, level 3, and MLD-F-MESSAGE-VALIDATION stays
   partial until then.
2. **The IPv6-specific gaps 2 to 5 are repaired**: a link-local source, the Router Alert option,
   the checksum over the pseudo-header, and the Reports for the solicited-node addresses. The
   repair of the checksum reaches every ICMPv6 message of the model.
3. **The obsolete claim of level 1 stands**: `Mldv2.ned:12` names RFC 3810, and the stale
   "parity gaps" comment of the same file still says that three working mechanisms are missing.
