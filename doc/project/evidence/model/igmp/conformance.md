# IGMP — model claims and conformance matrix

> **Kind:** report · **Status:** snapshot 2026-09-24 · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/igmp/standards.md), [features.md](../../protocol/igmp/features.md), [coverage.md](coverage.md)

Step 8 artifact of the standards test workflow. Part 1 records what the model intends: which
standards it claims to implement, mapped onto the standards map. Part 2 crosses the claims
with the feature support of the ledger, feature by feature.
[`mld/conformance.md`](../mld/conformance.md) records the parallel claims for the IPv6 side;
the two modules share code lineage (the MLDv2 module is a documented port of `Igmpv3`), and
the notes below say where the two diverge.

Run record of the ledger state that part 2 comes from:

- Date: 2026-09-29 18:33 +0200
- INET: branch `topic/standards-tests-igmp-mld-level2-fixes`, commit `ddea7a391b`, tree clean
- Trees: src `e76d92f3af`, tests/protocol `36c0e0673d`
- OMNeT++: 6.4.0, commit `cf58891643`
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-34-generic x86_64
- Command: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/igmp$'`

Part 1 was first read by the level 1 pass, at src `16dc528e10`, commit `e360ca980e` of the wave 0
branch. The IGMP code is the same in `5c4f41c600`: the two trees differ only in seven files of
IEEE 802.11. The line numbers of part 1 therefore hold for both.
The repairs of this branch change the IGMP code, so part 1 describes the code before them.

## Part 1 — the claims

Every place in the model that names a document of the IGMP family. Source:
`src/inet/networklayer/ipv4/{Igmpv2,Igmpv3,IIgmp,IgmpMessage}.{cc,h,ned,msg}`,
`doc/src/users-guide/ch-ipv4.rst`, `WHATSNEW`.

| Claim | Where | What it says |
| --- | --- | --- |
| RFC 2236 | [`Igmpv2.ned:26`](../../../../../src/inet/networklayer/ipv4/Igmpv2.ned) | "This module implements both IGMPv2 host and router logic as specified in RFC 2236." The module documentation comment. |
| RFC 3376 | [`Igmpv3.ned:13`](../../../../../src/inet/networklayer/ipv4/Igmpv3.ned) | "Implements IGMPv3 (RFC 3376), the source-filtering version of the Internet Group Management Protocol for IPv4." The module documentation comment. |
| RFC 1112, RFC 2236, RFC 3376 | [`IIgmp.ned:15-16`](../../../../../src/inet/networklayer/ipv4/IIgmp.ned) | "Currently, there are 3 versions specified in RFC 1112 (IGMPv1), RFC 2236 (IGMPv2), and RFC 3376 (IGMPv3)." The shared module interface documentation — the one place all three versions are named together. |
| RFC 2236 | [`Igmpv2.cc:30,63`](../../../../../src/inet/networklayer/ipv4/Igmpv2.cc) | "RFC 2236, Section 6: Host State Diagram" and "...Section 7: Router State Diagram", above the two state machines. |
| RFC 2236 | [`Igmpv2.h:137-146`](../../../../../src/inet/networklayer/ipv4/Igmpv2.h) | Ten timer-field comments, one per RFC 2236 §8.2 to §8.11 (the last field is commented out — the timer itself is unimplemented, only the citation remains). |
| RFC 3376 | [`Igmpv3.h`](../../../../../src/inet/networklayer/ipv4/Igmpv3.h), 9 lines: `73,77,80,98,118,170,178,234,326` | Nine comments citing an RFC 3376 section for a timer kind, a compatibility-level field, or the Robustness Variable. |
| RFC 3376 | [`Igmpv3.cc`](../../../../../src/inet/networklayer/ipv4/Igmpv3.cc), 30 lines: `73,175,194,263,461,472,534,585,617,670,690,697,838,852,861,902,1101,1103,1212,1237,1253,1293,1318,1327,1356,1357,1385,1419,1442,1670` | Inline comments citing an RFC 3376 section above the code that implements it — report generation, retransmission, older-version compatibility (§7.2/§7.3), forwarding rules (§6.3). |
| RFC 3488 (a different protocol, RGMP) | [`IgmpMessage.msg:30,130`](../../../../../src/inet/networklayer/ipv4/IgmpMessage.msg) | The `RGMP_HELLO` message type constant and its comment name RFC 3488, which shares the IGMP wire format but is a separate protocol the model does not implement. |
| RFC 2236 | [`ch-ipv4.rst:218-219`](../../../../../doc/src/users-guide/ch-ipv4.rst) | "The `Igmpv2` module implements version 2 of the IGMP protocol (RFC 2236)." |
| none | [`ch-ipv4.rst:194`](../../../../../doc/src/users-guide/ch-ipv4.rst) | "The `Igmpv3` module implements the Internet Group Management Protocol (IGMP)." The user's guide section that introduces the module a reader is most likely to configure names **no document at all** — the only RFC number for IGMPv3 anywhere in the prose documentation is the NED comment. |
| none | [`WHATSNEW`](../../../../../WHATSNEW) | No line in `WHATSNEW` names an IGMP RFC number by grep (`grep -n -iE 'IGMP.*RFC\|RFC.*IGMP' WHATSNEW` finds nothing); lines 5500-5503 describe an external-router mode for IGMPv2 without a citation. |

No line anywhere in the tree names RFC 1112 or RFC 4604 directly (`grep -rn 'RFC ?1112\|RFC
?4604' src/inet/networklayer/ipv4/` finds nothing); `IIgmp.ned:15` names RFC 1112 only as part
of the three-version list quoted above.

Mapped onto the standards map, [`standards.md`](../../protocol/igmp/standards.md#document-list):

| Document | In the in-scope set | Claimed | Note |
| --- | --- | --- | --- |
| RFC 9776 | yes, `base` | **no** | Nothing in the model names RFC 9776. Promoted to Internet Standard in March 2025; nothing in this tree has been touched since. |
| RFC 3376 | no, obsoleted | **yes**, in the module documentation of `Igmpv3` and in about 24 inline section citations | **The headline obsolete claim.** `Igmpv3.ned:13` is the module reference's own description of the largest file in the family (1976 lines), and it names a document RFC 9776 obsoleted seven months before this read date. |
| RFC 2236 | yes, `companion` | **yes**, in the module documentation of `Igmpv2`, the user's guide, and ten timer-field comments | Current and correctly cited; RFC 2236 is not obsoleted, so this claim is not a register gap. |
| RFC 1112 | no, level 5 | named only inside the three-version list of `IIgmp.ned:15`, never as an implementation claim | No module implements it; `IgmpMessage.msg`'s `Igmpv1Query`/`Igmpv1Report` are message classes only, confirmed by the absence of an `Igmpv1` `.cc`/`.h` pair in the directory listing. |
| RFC 4604 | no, level 5 | no | Not named anywhere in `src/inet/networklayer/ipv4/`. No SSM-address-range check exists in the router forwarding or report-reception code. |
| RFC 3488 | no, not a family relative | named, for a shared wire format, declined | `IgmpMessage.msg:30,130` names it once for the reserved `RGMP_HELLO` type; nothing processes or sends it. |

**The finding of the survey.** IGMPv2 is claimed correctly and consistently: RFC 2236 is not
obsoleted, and every citation — the NED file, the user's guide, the ten timer comments — names
it. IGMPv3 is claimed only as RFC 3376, which RFC 9776 obsoleted on 2025-03-11, six months
before this read date; the claim appears in the one place the module reference publishes
(`Igmpv3.ned:13`) and in roughly 24 inline section citations, and it is never once corrected to
the current document anywhere in the tree — not the NED file, not `WHATSNEW`, not the user's
guide, which for IGMPv3 names no RFC number at all. This is the same shape of finding
[`mld/conformance.md`](../mld/conformance.md) records for MLDv2 against RFC 3810/RFC 9777,
discovered independently and confirming the two protocols drifted from their governing
documents together.

For the level 2 pass, four facts from the code (the guide permits this look to decide the
claim question, and the pass must check each one):

1. **Unrecognized message types stop the simulation instead of being silently ignored.**
   `Igmpv2.cc:488` and `Igmpv3.cc:686` both `throw cRuntimeError` on an unhandled type. RFC 9776
   states this as a MUST (`rfc9776.txt:482`); RFC 2236 already stated it, as a lower-case
   "should" (`rfc2236.txt:123`). The code exists and runs on the normal receive path — this is a
   claim, and a check that reaches it is a defect, not a declared expected failure.
2. **The Router Alert IP option is never attached to an outgoing message.** Three separate `//
   TODO fill Router Alert option` comments name the gap (`Igmpv2.cc:703,721,728`;
   `Igmpv3.cc:1469,1488`), against a requirement RFC 9776 states for every IGMP message
   (`rfc9776.txt:447-450`) and RFC 2236 already required (`rfc2236.txt:1272`, "The IGMPv2 spec
   requires the presence of the IP Router Alert option"). A bare TODO is a claim; unlike the
   MLD side (see [`mld/conformance.md`](../mld/conformance.md)), the model at least names this
   gap in a comment at each site.
3. **`Igmpv3.cc:815`'s "FIXME also accept Igmpv1Report and Igmpv2Report" looks stale.** The
   comment sits inside `processReport()`, but the dispatcher (`Igmpv3.cc:666-680`) already
   routes `IGMPV1_MEMBERSHIP_REPORT` and `IGMPV2_MEMBERSHIP_REPORT` to a separate
   `processOlderVersionReport()` before `processReport()` ever runs — `processReport()` can only
   receive an `IGMPV3_MEMBERSHIP_REPORT` in the reachable path. A level 2 pass should read
   whether this comment describes a real gap or is left over from before the dispatch split.
4. **The Group Membership Interval and Older Version Querier Present Interval formulas in the
   code have not been checked against RFC 9776's changed text** (see the override table of
   [`standards.md`](../../protocol/igmp/standards.md#override-table)). `Igmpv2.ned:105` and
   `Igmpv3.ned:64` both default `groupMembershipInterval` to `(robustnessVariable *
   queryInterval) + queryResponseInterval` — one Query Response Interval, matching RFC 2236/RFC
   3376's superseded formula, not RFC 9776's "plus 2 *" text. This is a NED default, not a
   catalog verdict; a level 2 pass decides whether the default itself is the statement under
   test or only the code path that computes the deadline.

The level 2 pass checked the four facts; [`results.md`](results.md#the-four-facts-of-the-level-1-look)
has the outcome of each.

## Part 2 — the conformance matrix

A feature is `claimed` when the claims of part 1 cover its governing source document. The
model claims RFC 3376 for IGMPv3, and RFC 9776 is the same protocol raised to Internet Standard,
with the changes of the [override table](../../protocol/igmp/standards.md#override-table); the
claim of RFC 3376 therefore covers every feature that RFC 3376 already had, and the two changed
intervals were gaps of their own, which the repairs closed, [gap 3](results.md#gap-3-defect--two-default-intervals-have-the-values-of-older-documents)
and [gap 4](results.md#gap-4-defect--the-igmpv2-mode-of-a-host-ends-after-the-other-querier-present-interval).
The model claims RFC 2236 as a whole, so the two version 2 features are claimed. One feature is
not claimed: IGMP-F-SSM-AWARE comes from the SSM text of RFC 4604, which RFC 9776 took over, and
the model names RFC 4604 nowhere. The support comes from [`coverage.md`](coverage.md#feature-support).

| Feature | Level | Claimed | Support | Verdict |
| --- | --- | --- | --- | --- |
| [IGMP-F-MESSAGE-FORMAT](../../protocol/igmp/features.md#igmp-f-message-format) | mandatory | yes | supported | `confirmed` |
| [IGMP-F-QUERY-FORMAT](../../protocol/igmp/features.md#igmp-f-query-format) | mandatory | yes | partial | `partial` — no core check fails; RFC9776-QRY-4, QRY-13, QRY-17 are owed |
| [IGMP-F-REPORT-FORMAT](../../protocol/igmp/features.md#igmp-f-report-format) | mandatory | yes | supported | `confirmed` |
| [IGMP-F-MESSAGE-VALIDATION](../../protocol/igmp/features.md#igmp-f-message-validation) | mandatory | yes | untested | `unverified` — level 3 |
| [IGMP-F-STATE-CHANGE-REPORT](../../protocol/igmp/features.md#igmp-f-state-change-report) | mandatory | yes | supported | `confirmed` |
| [IGMP-F-REPORT-RETRANSMISSION](../../protocol/igmp/features.md#igmp-f-report-retransmission) | mandatory | yes | supported | `confirmed` |
| [IGMP-F-QUERY-RESPONSE](../../protocol/igmp/features.md#igmp-f-query-response) | mandatory | yes | partial | `partial` — no core check fails; RFC9776-HQRY-16 is owed |
| [IGMP-F-GENERAL-QUERY](../../protocol/igmp/features.md#igmp-f-general-query) | mandatory | yes | supported | `confirmed` |
| [IGMP-F-QUERIER-ELECTION](../../protocol/igmp/features.md#igmp-f-querier-election) | mandatory | yes | supported | `confirmed` |
| [IGMP-F-GROUP-MEMBERSHIP](../../protocol/igmp/features.md#igmp-f-group-membership) | mandatory | yes | partial | `partial` — no core check fails; RFC9776-RREP-8, RREP-10 are owed |
| [IGMP-F-FORWARDING](../../protocol/igmp/features.md#igmp-f-forwarding) | mandatory | yes | partial | `partial` — no core check fails; RFC9776-FWD-6, FWD-7 are owed |
| [IGMP-F-STATE-CHANGE-PROCESSING](../../protocol/igmp/features.md#igmp-f-state-change-processing) | mandatory | yes | partial | `partial` — no core check fails; RFC9776-RREP-24, RREP-25, RREP-26 are owed; RFC9776-RREP-23 is later |
| [IGMP-F-SPECIFIC-QUERIES](../../protocol/igmp/features.md#igmp-f-specific-queries) | mandatory | yes | supported | `confirmed` |
| [IGMP-F-QUERY-TIMER-UPDATES](../../protocol/igmp/features.md#igmp-f-query-timer-updates) | mandatory | yes | partial | `partial` — no core check fails; RFC9776-RQRY-4, QRY-10 are owed |
| [IGMP-F-HOST-COMPATIBILITY](../../protocol/igmp/features.md#igmp-f-host-compatibility) | mandatory | yes | partial | `partial` — no core check fails; RFC9776-VER-1, COMPH-5, COMPH-8, COMPH-11, COMPH-20 are later |
| [IGMP-F-VERSION-2-HOST](../../protocol/igmp/features.md#igmp-f-version-2-host) | mandatory | yes | partial | `partial` — no core check fails; RFC2236-HOST-30 is owed |
| [IGMP-F-ROUTER-COMPATIBILITY](../../protocol/igmp/features.md#igmp-f-router-compatibility) | mandatory | yes | partial | `partial` — no core check fails; RFC9776-COMPR-20, COMPR-30, TIMER-25 are owed; RFC9776-GEN-7, COMPR-16, COMPR-19, COMPR-31, COMPR-32, COMPR-33 are later |
| [IGMP-F-VERSION-2-ROUTER](../../protocol/igmp/features.md#igmp-f-version-2-router) | mandatory | yes | partial | `partial` — no core check fails; RFC2236-ROUTER-37, ROUTER-41 are owed; RFC2236-ROUTER-36, ROUTER-40, ROUTER-42, ROUTER-45, ROUTER-46, ROUTER-47 are later |
| [IGMP-F-SSM-AWARE](../../protocol/igmp/features.md#igmp-f-ssm-aware) | optional | no | untested | `out of claim` — level 5 |
| [IGMP-F-TIMER-CONFIGURATION](../../protocol/igmp/features.md#igmp-f-timer-configuration) | mandatory | yes | partial | `partial` — no core check fails; RFC9776-TIMER-1 is owed |

Seven features are `confirmed`, eleven `partial`, one `unverified` and one `out of claim`. In the
level 2 run, five were `confirmed` and thirteen `partial`.

### How to read the matrix

- **No feature is a `defect`, and since the repairs no core check fails.** Every `partial`
  verdict comes from a core statement whose check is owed or needs a later level. In the level 2
  run the thirteen gaps of [`results.md`](results.md#the-model-gaps) were statement-level defects
  inside eleven partial features.
- **Gap 13 was the one missing feature**, inside IGMP-F-ROUTER-COMPATIBILITY: the router half of
  the IGMPv2 mode. The repairs added it, with the IGMPv1 mode, as the parameter `routerVersion`
  of `Igmpv3`; the feature stays partial because the Older Host Present Interval and some
  IGMPv1 statements have no check.
- **The one `unverified`**, IGMP-F-MESSAGE-VALIDATION, is level 3 work, not a verdict on the
  model; the level 1 look and the run both show that an unknown type stops the simulation
  ([results.md](results.md#other-findings)), so its check will fail when it exists.

## Headlines for the next pass

1. **IGMP-F-MESSAGE-VALIDATION is `unverified` and mandatory**, and its first check will fail:
   the IGMPv2 router stops the simulation on a Version 3 Report already on the normal path.
2. **The thirteen gaps** of [`results.md`](results.md#the-model-gaps) are repaired, and the suite
   passes. The next level 2 work is the debt of
   [`coverage.md`](coverage.md#the-coverage-debt-the-checks-this-pass-owes), 49 owed statements.
3. **The obsolete claim of level 1 stands**: `Igmpv3.ned:13` and the inline comments name RFC
   3376. Two of the gaps were values of the documents that RFC 9776 replaced; the repairs changed
   the values, and not the claim.
