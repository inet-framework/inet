# IGMP — run results and model analysis

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../../protocol/igmp/checks.md), [features.md](../../protocol/igmp/features.md), [coverage.md](coverage.md)

Step 7 artifact of the standards test workflow for the IGMP level 2 pass. This is the first
document of the pass that may name the simulation model and reference code, and it does both:
the verdicts of the run, the class of every failure, and where the model implements, or fails
to implement, each checked behavior.

## Run record

- Date: 2026-09-24 16:03 +0200
- INET: branch `topic/standards-tests-igmp-mld-level2`, commit `bb20f0dd5e`, tree clean
- Trees: src `5c4f41c600`, tests/protocol `b925cca8a5`
- OMNeT++: 6.4.0, commit `cf58891643`
- Build: debug; the object files are a copy of a build of the same src tree `5c4f41c600`
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-34-generic x86_64
- Command: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/igmp$'`
- Suite: 42 tests, 25 PASS, 16 FAIL (unexpected), 1 FAIL (expected), so the suite reports FAIL

The `src/` tree `5c4f41c600` is the tree of `origin/master` at `7772a7e4ef`: this pass changes
no source file.

## Verdicts

One row per test. The check sections are in [`checks/`](../../protocol/igmp/checks); the
statements are those of the `Checks:` line of each section. Ten checks have two tests, so that
one failing observation does not hide the verdict of another: the Router Alert and the
precedence, the timer relations, the repetitions of a join, a change during the repetitions,
the answer to a General Query, source-specific forwarding, the S flag of each query kind, the
IGMPv2 mode of a host, and the return to IGMPv3.

| Test | Check | Verdict | Class |
| --- | --- | --- | --- |
| `Rfc9776ReportEncapsulation` | [Membership Report encapsulation](../../protocol/igmp/checks/message-format.md#membership-report-encapsulation) | PASS | — |
| `Rfc9776QueryEncapsulation` | [Membership Query encapsulation](../../protocol/igmp/checks/message-format.md#membership-query-encapsulation) | FAIL at observation 5 | defect, [gap 1](#gap-1-defect--the-queries-carry-no-qrv-and-no-qqic) |
| `Rfc9776RouterAlert` | [Router Alert and precedence on every message](../../protocol/igmp/checks/message-format.md#router-alert-and-precedence-on-every-message) | PASS | — |
| `Rfc9776Precedence` | [Router Alert and precedence on every message](../../protocol/igmp/checks/message-format.md#router-alert-and-precedence-on-every-message) | FAIL at observation 3 | defect, [gap 2](#gap-2-defect--a-report-leaves-with-precedence-0) |
| `Rfc9776QueryRobustness` | [Timer relations in the Query](../../protocol/igmp/checks/message-format.md#timer-relations-in-the-query) | FAIL at observation 2 | defect, [gap 1](#gap-1-defect--the-queries-carry-no-qrv-and-no-qqic) |
| `Rfc9776QueryTimerRelations` | [Timer relations in the Query](../../protocol/igmp/checks/message-format.md#timer-relations-in-the-query) | PASS | — |
| `Rfc9776JoinReport` | [Join reported at once](../../protocol/igmp/checks/host-reports.md#join-reported-at-once) | PASS | — |
| `Rfc9776JoinRepeated` | [Join repeated](../../protocol/igmp/checks/host-reports.md#join-repeated) | FAIL at observation 2 | defect, [gap 3](#gap-3-defect--two-default-intervals-have-the-values-of-older-documents) |
| `Rfc9776JoinRepeatedCount` | [Join repeated](../../protocol/igmp/checks/host-reports.md#join-repeated) | PASS | — |
| `Rfc9776LeaveReport` | [Leave reported](../../protocol/igmp/checks/host-reports.md#leave-reported) | PASS | — |
| `Rfc9776SourceListChange` | [Source list change](../../protocol/igmp/checks/host-reports.md#source-list-change) | PASS | — |
| `Rfc9776ExcludeChange` | [Change inside EXCLUDE mode](../../protocol/igmp/checks/host-reports.md#change-inside-exclude-mode) | PASS | — |
| `Rfc9776ChangeDuringRepetitions` | [A change during the repetitions](../../protocol/igmp/checks/host-reports.md#a-change-during-the-repetitions) | FAIL at observation 2 | defect, [gap 5](#gap-5-defect--a-second-change-replaces-the-pending-records-instead-of-a-merge) |
| `Rfc9776ChangeDuringRepetitionsTail` | [A change during the repetitions](../../protocol/igmp/checks/host-reports.md#a-change-during-the-repetitions) | PASS | — |
| `Rfc9776GeneralQueryResponse` | [Response to a General Query](../../protocol/igmp/checks/query-response.md#response-to-a-general-query) | PASS | — |
| `Rfc9776GeneralQueryNoState` | [Response to a General Query](../../protocol/igmp/checks/query-response.md#response-to-a-general-query) | FAIL at observation 5 | defect, [gap 8](#gap-8-defect--the-answer-to-a-general-query-holds-a-record-for-a-group-without-reception-state) |
| `Rfc9776GroupSpecificResponse` | [Response to a Group-Specific Query](../../protocol/igmp/checks/query-response.md#response-to-a-group-specific-query) | PASS | — |
| `Rfc9776GroupSourceResponse` | [Response to a Group-and-Source-Specific Query](../../protocol/igmp/checks/query-response.md#response-to-a-group-and-source-specific-query) | PASS | — |
| `Rfc9776StartupQueries` | [General Queries at startup and after it](../../protocol/igmp/checks/router-queries.md#general-queries-at-startup-and-after-it) | PASS | — |
| `Rfc9776QuerierElection` | [Querier election](../../protocol/igmp/checks/router-queries.md#querier-election) | PASS | — |
| `Rfc9776ForwardingAfterJoin` | [Forwarding after a join](../../protocol/igmp/checks/router-state.md#forwarding-after-a-join) | PASS | — |
| `Rfc9776SourceSpecificForwarding` | [Source-specific forwarding](../../protocol/igmp/checks/router-state.md#source-specific-forwarding) | PASS | — |
| `Rfc9776SourceSpecificBlocking` | [Source-specific forwarding](../../protocol/igmp/checks/router-state.md#source-specific-forwarding) | FAIL at observation 3 | defect, [gap 12](#gap-12-defect--the-forwarding-asks-for-a-listener-of-the-group-not-of-the-source) |
| `Rfc9776MembershipTimeout` | [Membership timeout without a leave](../../protocol/igmp/checks/router-state.md#membership-timeout-without-a-leave) | FAIL at observation 2 | defect, [gap 3](#gap-3-defect--two-default-intervals-have-the-values-of-older-documents) |
| `Rfc9776LastMemberQuery` | [Leave and the last member query](../../protocol/igmp/checks/router-state.md#leave-and-the-last-member-query) | PASS | — |
| `Rfc9776LastMemberQuerySFlag` | [Leave and the last member query](../../protocol/igmp/checks/router-state.md#leave-and-the-last-member-query) | FAIL at observation 6 | defect, [gap 9](#gap-9-defect--the-s-flag-of-a-group-specific-query-comes-from-the-timer-before-it-is-lowered) |
| `Rfc9776LeaveWithAnotherMember` | [Leave with another member](../../protocol/igmp/checks/router-state.md#leave-with-another-member) | PASS | — |
| `Rfc9776SourceBlockedOtherMember` | [Source blocked while another member wants it](../../protocol/igmp/checks/router-state.md#source-blocked-while-another-member-wants-it) | FAIL at observation 3 | defect, [gap 10](#gap-10-defect--a-report-cancels-the-retransmissions-of-the-queries) |
| `Rfc9776SourceQuerySFlag` | [Source blocked while another member wants it](../../protocol/igmp/checks/router-state.md#source-blocked-while-another-member-wants-it) | PASS | — |
| `Rfc9776SourceBlockedOnlyMember` | [Source blocked by its only member](../../protocol/igmp/checks/router-state.md#source-blocked-by-its-only-member) | FAIL at observation 2 | defect, [gap 11](#gap-11-defect--a-group-and-source-specific-query-does-not-lower-the-source-timers) |
| `Rfc9776RouterAfterJoin` | [A router that comes up after a join](../../protocol/igmp/checks/router-state.md#a-router-that-comes-up-after-a-join) | PASS | — |
| `Rfc9776HostV2Mode` | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | FAIL at observation 4 | defect, [gap 6](#gap-6-defect--the-igmpv2-mode-of-a-host-answers-at-once-and-sends-one-report) |
| `Rfc9776HostV2ModeRepeat` | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | FAIL at observation 3 | defect, [gap 6](#gap-6-defect--the-igmpv2-mode-of-a-host-answers-at-once-and-sends-one-report) |
| `Rfc2236ReportSuppression` | [Report suppression in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#report-suppression-in-igmpv2-mode) | FAIL at observation 2 | defect, [gap 6](#gap-6-defect--the-igmpv2-mode-of-a-host-answers-at-once-and-sends-one-report) |
| `Rfc9776LeaveV2Mode` | [Leave in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#leave-in-igmpv2-mode) | PASS | — |
| `Rfc2236LeaveAtV2Router` | [Leave at an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#leave-at-an-igmpv2-router) | PASS | — |
| `Rfc9776HostBackToV3` | [Host back to IGMPv3](../../protocol/igmp/checks/compatibility.md#host-back-to-igmpv3) | PASS | — |
| `Rfc9776OlderQuerierInterval` | [Host back to IGMPv3](../../protocol/igmp/checks/compatibility.md#host-back-to-igmpv3) | FAIL at observation 2 | defect, [gap 4](#gap-4-defect--the-igmpv2-mode-of-a-host-ends-after-the-other-querier-present-interval) |
| `Rfc9776RouterV2Member` | [Router with an IGMPv2 member](../../protocol/igmp/checks/compatibility.md#router-with-an-igmpv2-member) | PASS | — |
| `Rfc9776QuerierV2Router` | [Querier with an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#querier-with-an-igmpv2-router) | FAIL at observation 2, declared expected | missing feature, [gap 13](#gap-13-missing-feature--no-igmpv2-querier-mode-in-an-igmpv3-router) |
| `Rfc9776ModeChangeCancels` | [A mode change cancels the pending reports](../../protocol/igmp/checks/compatibility.md#a-mode-change-cancels-the-pending-reports) | FAIL at observation 2 | defect, [gap 7](#gap-7-defect--the-igmpv2-mode-keeps-the-pending-igmpv3-retransmissions) |
| `Rfc9776BlockInV2Mode` | [A BLOCK record for a group in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#a-block-record-for-a-group-in-igmpv2-mode) | PASS | — |

## The class of every failure

Fifteen failures are of the class **defect** of
[the guide](../../../guide/derive-tests-from-a-standard.md#the-class-of-a-failure-and-when-to-declare-it-expected):
the model has code for the behavior, and the code gets it wrong. None of them is declared
expected, because no limitation blocks a repair; each gap names the code that a repair would
change. One failure, `Rfc9776QuerierV2Router`, is an **unimplemented feature**: no code of the
IGMPv3 module sends an IGMPv2 Query, so the test declares `%# expected-result: FAIL`.

The first design of the pass had declared source-specific forwarding expected too. The code
exists in two halves that do not meet (gap 12), and the question of the guide, "does code exist
for this specific behavior?", makes it a defect; commit `bb20f0dd5e` removed the declaration.

No failure of this run is a test error or a misread of the specification. The earlier runs of
the pass found test errors — the order of the ini lines, two windows, a lifecycle stimulus that
the model cannot give — and one misread, the S flag of the second Group-and-Source-Specific
Query; the pass corrected each one before this run, and the plan records them.

The shapes of the defects: a field that the model never fills (gap 1), a line in a comment
(gap 2), a default or an interval with the value of another document (gaps 3 and 4), a state of
the host that the model does not keep (gaps 5, 6, 7 and 8), a timer that the router reads,
cancels or does not lower at the wrong moment (gaps 9, 10 and 11), and two halves of one
mechanism that do not meet (gap 12).

## The model gaps

### Gap 1 (defect) — the Queries carry no QRV and no QQIC

- **Tests**: `Rfc9776QueryEncapsulation`, observation 5 (QRV 0 and QQIC 0);
  `Rfc9776QueryRobustness`, observation 2 (QRV 0).
- **Statements**: RFC9776-QRY-12 (the QRV holds the Robustness Variable), QRY-15, QRY-16 (the
  QQIC holds the Query Interval); TIMER-3 (the Robustness Variable is not zero).
- **The code**: `Igmpv3::sendGeneralQuery` sets the type, the Max Resp Code and the length, and
  nothing else ([`Igmpv3.cc:1339-1346`](../../../../../src/inet/networklayer/ipv4/Igmpv3.cc)).
  The Group-Specific Queries (`sendGroupSpecificQuery`, line 1361), the
  Group-and-Source-Specific Queries (`sendGroupAndSourceSpecificQuery`, line 1420) and their
  retransmissions (`processRexmtTimer`, line 464) are built the same way. The two fields keep the
  value 0 of the message class.
- **Scope**: every Query of every IGMPv3 router. A QRV of 0 means a Robustness Variable above 7
  (RFC9776-QRY-13), and a QQIC of 0 a Query Interval of 0; a non-querier that adopts the values
  of the querier (RFC9776-TIMER-1, owed) would read both.

### Gap 2 (defect) — a Report leaves with precedence 0

- **Test**: `Rfc9776Precedence`, observation 3, at the first Report of A at 10 s.
- **Statement**: RFC9776-GEN-3 (every IGMP message has the precedence of Internetwork Control).
- **The code**: `Igmpv3::sendReportToIP` has the line that sets the DSCP in a comment, under
  "TODO set Type of Service to 0xc0"
  ([`Igmpv3.cc:1472-1473`](../../../../../src/inet/networklayer/ipv4/Igmpv3.cc));
  `Igmpv3::sendQueryToIP` sets it (line 1492), so the Queries of R hold.
- **Scope**: every Report and every Leave of an IGMPv3 host, also the IGMPv2 messages that the
  host sends in IGMPv2 mode, which go through the same function.

### Gap 3 (defect) — two default intervals have the values of older documents

- **Tests**: `Rfc9776JoinRepeated`, observation 2 (the repetition of the join comes 4.8 s after
  the first Report); `Rfc9776MembershipTimeout`, observation 2 (the last datagram of S1 leaves
  260 s after the last Report of A).
- **Statements**: RFC9776-TIMER-18, HOST-18 (the Unsolicited Report Interval is 1 second);
  TIMER-8, RREP-11 (the Group Membership Interval is [Robustness Variable] × [Query Interval]
  + 2 × [Query Response Interval], 270 s).
- **The code**: `unsolicitedReportInterval = default(10s)`
  ([`Igmpv3.ned:70`](../../../../../src/inet/networklayer/ipv4/Igmpv3.ned)), the value of RFC
  2236 §8.10; RFC 3376 and RFC 9776 give 1 second. `groupMembershipInterval =
  default((robustnessVariable * queryInterval) + queryResponseInterval)` (line 64), the formula
  of RFC 3376 §8.4, which the model claims; RFC 9776 §8.4 adds a second Query Response Interval.
  The level 1 look named the second one ([fact 4](#the-four-facts-of-the-level-1-look)).
- **Scope**: every host that keeps the default sends its repetitions up to 10 s after the first
  Report, where the standard asks for 1 s; every router that keeps the default ends a group 10 s
  early.

### Gap 4 (defect) — the IGMPv2 mode of a host ends after the Other Querier Present Interval

- **Test**: `Rfc9776OlderQuerierInterval`, observation 2: at 370 s, A sends a Version 3 Report.
- **Statement**: RFC9776-TIMER-22 (the Older Version Querier Present Interval is [Robustness
  Variable] × [Query Interval] + 10 × the Max Response Time of the last Query, 350 s).
- **The code**: `Igmpv3::processOlderVersionQuery` starts the Older Version Querier Present Timer
  with `otherQuerierPresentInterval`
  ([`Igmpv3.cc:1126`](../../../../../src/inet/networklayer/ipv4/Igmpv3.cc)), 255 s with the
  defaults ([`Igmpv3.ned:65`](../../../../../src/inet/networklayer/ipv4/Igmpv3.ned)): the
  interval of the querier election of a router. RFC 3376 §8.12 gave 260 s; RFC 9776 §8.12
  gives 350 s. A is in IGMPv3 mode again at 286.25 s.
- **Scope**: with Queries every 125 s, a pause between two Queries is 125, 250 or 375 s, and
  both 255 s and 350 s lie between the last two, so the gap shows only when the IGMPv2 querier
  stops: then the host goes back to IGMPv3 95 s early.

### Gap 5 (defect) — a second change replaces the pending records instead of a merge

- **Test**: `Rfc9776ChangeDuringRepetitions`, observation 2: the Report at 10.3 s holds
  ALLOW_NEW_SOURCES {S2}, without S1.
- **Statements**: RFC9776-HOST-15, HOST-16, HOST-25.
- **The code**: `Igmpv3::multicastSourceListChanged` computes the records of a change from the
  old and the new state, and replaces the pending records with them
  ([`Igmpv3.cc:264-275`](../../../../../src/inet/networklayer/ipv4/Igmpv3.cc)); the comment
  calls this a "merge by recomputation". RFC 9776 §5.1 keeps the retransmission state of each
  source: S1 belongs in the next Reports until [Robustness Variable] Reports have held it.
- **Scope**: a host that changes its filter twice within the repetitions of a Report sends the
  sources of the first change fewer times than the Robustness Variable; the loss of one Report
  can then lose a source.

### Gap 6 (defect) — the IGMPv2 mode of a host answers at once and sends one Report

- **Tests**: `Rfc9776HostV2Mode`, observation 4 (the answer leaves 12 microseconds after the
  Query); `Rfc9776HostV2ModeRepeat`, observation 3 (no second Report after a join);
  `Rfc2236ReportSuppression`, observation 2 (A and B both answer, at the same instant).
- **Statements**: RFC2236-HOST-21, HOST-28, HOST-31 (a report delay timer with a random value);
  HOST-22 (the repetition of the unsolicited Report); HOST-29 (the suppression of a Report that
  another member sent first).
- **The code**: `Igmpv3::processOlderVersionQuery` sends the IGMPv2 Reports in the event of the
  Query, for every joined group
  ([`Igmpv3.cc:1128-1133`](../../../../../src/inet/networklayer/ipv4/Igmpv3.cc) for a General
  Query, lines 1140-1145 for a Group-Specific Query). `Igmpv3::multicastSourceListChanged` sends
  one IGMPv2 Report for a join and returns (lines 198-216). The IGMPv2 mode of the host has no
  report delay timer, so it has no Delaying Member state and no suppression.
- **Scope**: every IGMPv3 host on a link with an IGMPv2 querier. All members of a group answer
  each Query at the same instant, and a lost unsolicited Report is not repeated.

### Gap 7 (defect) — the IGMPv2 mode keeps the pending IGMPv3 retransmissions

- **Test**: `Rfc9776ModeChangeCancels`, observation 2: A sends a Version 3 Report at 406.53 s,
  after the IGMPv2 Query of 406.25 s and its own IGMPv2 answer.
- **Statement**: RFC9776-COMPH-22 (a change of the compatibility mode cancels the pending
  response and retransmission timers).
- **The code**: `Igmpv3::processOlderVersionQuery` sets the compatibility mode and starts its
  timer ([`Igmpv3.cc:1125-1126`](../../../../../src/inet/networklayer/ipv4/Igmpv3.cc)) and
  cancels no timer. The retransmission timer of the group runs on and sends the pending
  Version 3 records (`processHostStateChangeTimer`, line 620).
- **Scope**: a host that changes its state just before it hears an IGMPv2 Query sends a
  Version 3 Report to an IGMPv2 router — which the IGMPv2 router of the model does not survive,
  see [Other findings](#other-findings).

### Gap 8 (defect) — the answer to a General Query holds a record for a group without reception state

- **Test**: `Rfc9776GeneralQueryNoState`, observation 5: a MODE_IS_INCLUDE record for K, with no
  sources, after A left K.
- **Statement**: RFC9776-HQRY-10 (one record for each address with reception state).
- **The code**: `Igmpv3::processHostGeneralQueryTimer` makes one record for each entry of the
  group table of the interface
  ([`Igmpv3.cc:557-563`](../../../../../src/inet/networklayer/ipv4/Igmpv3.cc)); a leave sets the
  entry to INCLUDE({}) and keeps it.
- **Scope**: a host answers every General Query with an empty record for each group that it has
  left, for as long as it runs. The run of `Rfc9776SourceBlockedOnlyMember` shows it too: A sends
  MODE_IS_INCLUDE {} for G at 163 s and 286 s.

### Gap 9 (defect) — the S flag of a Group-Specific Query comes from the timer before it is lowered

- **Test**: `Rfc9776LastMemberQuerySFlag`, observation 6: the first Group-Specific Query after the
  leave of the only member has the S flag set.
- **Statement**: RFC9776-RQRY-11.
- **The code**: `Igmpv3::sendGroupSpecificQuery` computes the flag from the Group Timer
  ([`Igmpv3.cc:1364`](../../../../../src/inet/networklayer/ipv4/Igmpv3.cc)) and then lowers the
  timer to the Last Member Query Time (line 1367); RFC 9776 §6.6.3.1 lowers the timer first, so
  the flag is clear unless a member answered. `Igmpv3::processRexmtTimer` sets the flag on every
  retransmission of a Group-Specific Query, with no look at the timer (line 514).
- **Scope**: every Group-Specific Query after a leave. A second router that honors the S flag
  (RFC9776-QRY-10, owed) would keep its timer, and would forward the group after the querier
  stops.

### Gap 10 (defect) — a Report cancels the retransmissions of the Queries

- **Test**: `Rfc9776SourceBlockedOtherMember`, observation 3: no second
  Group-and-Source-Specific Query follows the answer of A.
- **Statements**: RFC9776-RQRY-13; RREP-14 is not reached. The S flag of the second Query,
  RFC9776-RQRY-14, has no evidence: `Rfc9776SourceQuerySFlag` passes on the first Query alone.
- **The code**: `Igmpv3::processReport` cancels the retransmissions of a group when any record
  for the group arrives
  ([`Igmpv3.cc:851-859`](../../../../../src/inet/networklayer/ipv4/Igmpv3.cc)), "a new Report for
  this group supersedes any in-progress Last-Member Query retransmission". RFC 9776 §6.6.3 keeps
  the retransmission state of the group and of each source, and an answer changes only the
  timers, and so the S flag. The retransmission itself, `Igmpv3::processRexmtTimer`, queries only
  the sources whose timer is above the Last Member Query Time, and never sets the S flag of a
  Group-and-Source-Specific Query (lines 464-532); §6.6.3.2 sends those sources with the flag set,
  and the sources at or below it in a second Query with the flag clear.
- **Scope**: every Group-Specific and Group-and-Source-Specific Query that a member answers. The
  sequence stops at the first answer, so the loss of one answer can end the group or the source
  early.

### Gap 11 (defect) — a Group-and-Source-Specific Query does not lower the Source Timers

- **Test**: `Rfc9776SourceBlockedOnlyMember`, observation 2: R forwards a datagram of S1 at
  52.5 s, 2.5 s after the leave of its only member.
- **Statements**: RFC9776-RQRY-2; RQRY-12, RREP-13 (the lowering itself); FWD-4, RST-14 are not
  reached.
- **The code**: `Igmpv3::sendGroupAndSourceSpecificQuery` sends the Query and schedules its
  retransmission, and changes no Source Timer
  ([`Igmpv3.cc:1420-1456`](../../../../../src/inet/networklayer/ipv4/Igmpv3.cc)). The
  Group-Specific Query lowers the Group Timer (line 1367), so the leave in EXCLUDE mode ends the
  group in time: `Rfc9776LastMemberQuery` passes.
- **Scope**: a source that the last member of an INCLUDE-mode group blocks stays forwarded until
  its Source Timer ends, up to 260 s after the last Report that named it.

### Gap 12 (defect) — the forwarding asks for a listener of the group, not of the source

- **Test**: `Rfc9776SourceSpecificBlocking`, observation 3: the first datagram of S2 arrives on
  L1 at 10.00005 s, although A wants S1 only.
- **Statement**: RFC9776-FWD-5.
- **The code**: IPv4 forwards a multicast datagram onto a leaf interface when
  `Ipv4InterfaceData::hasMulticastListener(group)` holds
  ([`Ipv4.cc:534`](../../../../../src/inet/networklayer/ipv4/Ipv4.cc) and line 803).
  `Igmpv3::processReport` stores the forwarded sources of the group with `setMulticastListeners`
  ([`Igmpv3.cc:1093`](../../../../../src/inet/networklayer/ipv4/Igmpv3.cc)), and
  `hasMulticastListener(group, source)` exists
  ([`Ipv4InterfaceData.h:169`](../../../../../src/inet/networklayer/ipv4/Ipv4InterfaceData.h)),
  but the forwarding never asks it.
- **Scope**: every IGMPv3 router forwards all sources of a group that has a member. The source
  lists of INCLUDE and EXCLUDE mode have no effect on the data, only on the Queries.

### Gap 13 (missing feature) — no IGMPv2 querier mode in an IGMPv3 router

- **Test**: `Rfc9776QuerierV2Router`, observation 2, declared expected: after the IGMPv2 Query of
  R2, R1 sends a General Query of 12 octets at 31.25 s.
- **Statements**: RFC9776-RQRY-8, COMPR-1, COMPR-6, and COMPR-2 (the configuration option);
  COMPR-8 is not reached.
- **The code**: `Igmpv3` builds only `Igmpv3Query` messages
  ([`Igmpv3.cc`](../../../../../src/inet/networklayer/ipv4/Igmpv3.cc) lines 503, 1343, 1371 and
  1428), and has no parameter for a compatibility mode. An IGMPv2 General Query reaches the
  router half of the module only for the querier election (`processOlderVersionQuery`, line
  1151). The router half of RFC 9776 §7.3.1 is not in the model.
- **Scope**: a link with an IGMPv2 router and an IGMPv3 router has two versions of Queries. The
  test declares the failure expected; a repair is new code, not a correction.

## What the model does well

- **The Membership Report** leaves with IPv4 protocol 2, TTL 1, the Router Alert option, the
  destination 224.0.0.22 and a correct checksum; its length agrees with its records, its Reserved
  fields are zero, and its records carry no auxiliary data.
- **The reports of a host** follow §5.1: a join sends CHANGE_TO_EXCLUDE_MODE at once, a leave
  CHANGE_TO_INCLUDE_MODE, and a change of the source list ALLOW_NEW_SOURCES and
  BLOCK_OLD_SOURCES records without an empty record, in INCLUDE and in EXCLUDE mode; with the
  interval of the standard, each Report is sent twice and no more. No Report names 224.0.0.1.
- **The answers of a host** come within the Max Response Time and not at the instant of the
  Query: MODE_IS_EXCLUDE and MODE_IS_INCLUDE records for a General Query, an answer for the
  queried group only, and for a Group-and-Source-Specific Query only the queried sources that
  the host wants.
- **The querier** sends its General Queries at 0 and 31.25 s and then every 125 s, to 224.0.0.1
  with the Router Alert, precedence 6 and a Max Resp Code of 100; the election gives up the role
  to a lower address and takes it back exactly 255 s after the last Query of the other router.
- **The router state in EXCLUDE mode** is right: a join starts the forwarding within one
  datagram, a router that missed the join learns it from the next answer, a leave sends two
  Group-Specific Queries 1 s apart with Max Resp Code 10 and ends the forwarding within the Last
  Member Query Time, and the answer of another member keeps it.
- **The IGMPv2 side** holds its formats: an IGMPv3 host in IGMPv2 mode sends IGMPv2 Reports of 8
  octets to the group and Leaves to 224.0.0.2, returns to IGMPv3 when its timer ends, and the
  IGMPv3 router keeps a group with an IGMPv2 member in EXCLUDE mode, ignores a BLOCK record for
  it, and ends it after the Leave. The `Igmpv2` router follows its own Leave procedure of RFC 2236
  §7.

## Other findings

- **The IGMPv2 router stops the simulation on a Version 3 Report.** `Igmpv2` throws
  "Unhandled message type" on a type that it does not know
  ([`Igmpv2.cc:488`](../../../../../src/inet/networklayer/ipv4/Igmpv2.cc)); RFC 2236 §2 says
  "Unrecognized message types should be silently ignored". This is fact 1 of the level 1 look,
  and the run reaches it without a crafted message: an ordinary Version 3 Report of an IGMPv3
  host is enough. A link with an IGMPv2 router and an IGMPv3 host in IGMPv3 mode stops the run —
  after gap 4 ends the IGMPv2 mode of the host early, or with gap 7. RFC 2236 §2 is outside the
  in-scope set of this pass, so no check targets it; the check of the mode change keeps R off L1
  while A sends its Version 3 Report.
- **The IGMP modules have no lifecycle.** A router that is down at the start stops the
  simulation at initialization: `Igmpv3::initialize` reads the IPv4 data of each interface
  ([`Igmpv3.cc:117`](../../../../../src/inet/networklayer/ipv4/Igmpv3.cc)), which a node that is
  down does not have ("Tag 'inet::Ipv4InterfaceData' is absent"). A router that crashes keeps
  its timers, and its next Query stops the simulation ("Message 'Igmpv3 query' received when
  Ipv4 is down"). The checks never stop a node; a node joins or leaves L1 through its link.
- **A channel that is disabled at initialization never comes back.** `EthernetMacBase`
  subscribes to the parameters of its channel only when the channel is enabled at initialization
  ([`EthernetMacBase.cc:428`](../../../../../src/inet/linklayer/ethernet/base/EthernetMacBase.cc)),
  so a later enable does not reach the MAC, which drops every frame as "not connected". Every
  link of the mockups starts enabled.
- **The IGMPv2 router sends its Queries without the Router Alert option and with precedence 0.**
  `Igmpv2::sendToIP` has "TODO add Router Alert option" above it
  ([`Igmpv2.cc:728-729`](../../../../../src/inet/networklayer/ipv4/Igmpv2.cc)); the Reports and
  Leaves of `Igmpv2` carry the option (lines 703-705 and 721-723). The checks read the Router
  Alert of the IGMPv3 module only.
- **No multicast routing of the model forwards in the mockup with a source.**
  `Ipv4NetworkConfigurator` never marks a multicast route as leaf ("TODOisLeaf",
  [`Ipv4NetworkConfigurator.cc:1267`](../../../../../src/inet/networklayer/configurator/ipv4/Ipv4NetworkConfigurator.cc)
  and line 2031), so IPv4 forwards to every out interface and never asks IGMP; PIM-DM in the
  mockup did not forward ("source is not directly connected",
  [`PimDm.cc:1430`](../../../../../src/inet/routing/pim/modes/PimDm.cc)). The tests add one route
  with a test module, `MulticastLeafRoute` of `IgmpChecks.h`. Both are outside the in-scope set.

## The four facts of the level 1 look

The level 1 pass named four facts for this pass to check
([`conformance.md`](conformance.md#part-1--the-claims)):

1. **Unrecognized message types stop the simulation.** Confirmed for `Igmpv2`, on the normal
   path: see [Other findings](#other-findings). The same `throw` of `Igmpv3`
   ([`Igmpv3.cc:686`](../../../../../src/inet/networklayer/ipv4/Igmpv3.cc)) needs a crafted
   message, level 3.
2. **The Router Alert option is never attached.** No longer true for `Igmpv3`: the TODO comments
   stay, but the next lines attach the option, and `Rfc9776RouterAlert` passes. `Igmpv2` attaches
   it to its Reports and Leaves, and not to its Queries.
3. **The FIXME of `Igmpv3.cc:815` is stale.** The dispatcher sends the IGMPv1 and IGMPv2 Reports
   to `processOlderVersionReport` (lines 672-677), so `processReport` receives only Version 3
   Reports; the older Reports reach the router state, and `Rfc9776RouterV2Member` passes.
4. **The two intervals of RFC 9776 §8.** The Group Membership Interval is the RFC 3376 value,
   260 s, and the Older Version Querier Present Interval is 255 s, the value of no document:
   gaps 3 and 4.

## What the next pass owes

In the order I would do it:

1. **Level 3**, the crafted messages: the validation of every received message, the unknown
   types of both modules, and the IGMPv2 Group-Specific Query at a host in IGMPv3 mode. IGMP
   message validation is the one mandatory feature with no check at level 2.
2. **After a repair of gap 10**, run `Rfc9776SourceBlockedOtherMember` and
   `Rfc9776SourceQuerySFlag` again: RFC9776-RQRY-14 has no evidence for a retransmission until
   then, and `processRexmtTimer` (gap 10) will then decide it.
3. **After a repair of gap 13**, COMPR-8 gets its verdict from `Rfc9776QuerierV2Router`.
4. **The level 2 statements this pass left**: the closing list of
   [`checks.md`](../../protocol/igmp/checks.md#statements-this-pass-wrote-no-check-for) names what
   each needs, from a code value above the plain range to two IGMPv2 routers on one link.
