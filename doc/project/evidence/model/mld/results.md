# MLD — run results and model analysis

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../../protocol/mld/checks.md), [features.md](../../protocol/mld/features.md), [coverage.md](coverage.md)

Step 7 artifact of the standards test workflow for the MLD level 2 pass. This is the first
document of the pass that may name the simulation model and reference code, and it does both:
the verdicts of the run, the class of every failure, and where the model implements, or fails
to implement, each checked behavior. The MLD checks are the twins of the IGMP checks, so most
gaps have a twin in [`igmp/results.md`](../igmp/results.md#the-model-gaps), and each gap below
that has one names it.

The level 2 pass ran on 2026-09-24 at `29aed12310` (src `5c4f41c600`): 47 tests, 21 PASS, 25
FAIL, 1 FAIL declared expected, and seventeen gaps. The repairs of 2026-09-25 to 2026-09-28, on
`topic/standards-tests-igmp-mld-level2-fixes`, repaired all seventeen; the run below is theirs.
Each gap keeps its description of the code of the level 2 run, and says how it was repaired.

## Run record

- Date: 2026-09-29 18:33 +0200
- INET: branch `topic/standards-tests-igmp-mld-level2-fixes`, commit `ddea7a391b`, tree clean
- Trees: src `e76d92f3af`, tests/protocol `36c0e0673d`
- OMNeT++: 6.4.0, commit `cf58891643`
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-34-generic x86_64
- Command: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/mld$'`
- Suite: 47 tests, 47 PASS, so the suite reports PASS

The `src/` tree of the level 2 run, `5c4f41c600`, is the tree of `origin/master` at
`7772a7e4ef`: the pass changed no source file. The repairs change `Mldv2`, `Mldv1`, `Icmpv6`,
`Ipv6`, `Ipv6InterfaceData` and the IPv6 extension headers for MLD; the IGMP half of the branch
changes `Igmpv3`, `Ipv4` and `Ipv4InterfaceData`.

## Verdicts

One row per test. The check sections are in [`checks/`](../../protocol/mld/checks); the
statements are those of the `Checks:` line of each section. Twelve checks have two or three
tests, so that one failing observation does not hide the verdict of another: the source
address and the checksum of the two messages, the Router Alert and the Hop Limit, the timer
relations, the solicited-node addresses, the repetitions of a start of listening, a change
during the repetitions, the answer to a General Query, source-specific forwarding, the S flag
of each query kind, the MLDv1 mode of a host, and the return to MLDv2.

| Test | Check | Verdict | Class |
| --- | --- | --- | --- |
| `Rfc9777ReportEncapsulation` | [Multicast Listener Report encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-report-encapsulation) | PASS | — |
| `Rfc9777ReportSource` | [Multicast Listener Report encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-report-encapsulation) | PASS | repaired, [gap 2](#gap-2-defect--an-mld-message-leaves-from-a-global-address) |
| `Rfc9777ReportChecksum` | [Multicast Listener Report encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-report-encapsulation) | PASS | repaired, [gap 4](#gap-4-defect--the-icmpv6-checksum-has-no-pseudo-header) |
| `Rfc9777QueryEncapsulation` | [Multicast Listener Query encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-query-encapsulation) | PASS | repaired, [gap 1](#gap-1-defect--the-queries-carry-no-qrv-and-no-qqic) |
| `Rfc9777QuerySource` | [Multicast Listener Query encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-query-encapsulation) | PASS | repaired, [gap 2](#gap-2-defect--an-mld-message-leaves-from-a-global-address) |
| `Rfc9777QueryChecksum` | [Multicast Listener Query encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-query-encapsulation) | PASS | repaired, [gap 4](#gap-4-defect--the-icmpv6-checksum-has-no-pseudo-header) |
| `Rfc9777RouterAlert` | [Router Alert on every message](../../protocol/mld/checks/message-format.md#router-alert-on-every-message) | PASS | repaired, [gap 3](#gap-3-defect--an-mld-message-carries-no-router-alert-option) |
| `Rfc9777HopLimitAndSource` | [Router Alert on every message](../../protocol/mld/checks/message-format.md#router-alert-on-every-message) | PASS | repaired, [gap 2](#gap-2-defect--an-mld-message-leaves-from-a-global-address) |
| `Rfc9777QueryRobustness` | [Timer relations in the Query](../../protocol/mld/checks/message-format.md#timer-relations-in-the-query) | PASS | repaired, [gap 1](#gap-1-defect--the-queries-carry-no-qrv-and-no-qqic) |
| `Rfc9777QueryTimerRelations` | [Timer relations in the Query](../../protocol/mld/checks/message-format.md#timer-relations-in-the-query) | PASS | — |
| `Rfc9777StartReport` | [Start of listening reported at once](../../protocol/mld/checks/listener-reports.md#start-of-listening-reported-at-once) | PASS | — |
| `Rfc9777SolicitedNodeReport` | [Start of listening reported at once](../../protocol/mld/checks/listener-reports.md#start-of-listening-reported-at-once) | PASS | repaired, [gap 5](#gap-5-defect--a-node-does-not-report-its-solicited-node-addresses) |
| `Rfc9777StartRepeated` | [Start of listening repeated](../../protocol/mld/checks/listener-reports.md#start-of-listening-repeated) | PASS | repaired, [gap 6](#gap-6-defect--two-default-intervals-have-the-values-of-older-documents) |
| `Rfc9777StartRepeatedCount` | [Start of listening repeated](../../protocol/mld/checks/listener-reports.md#start-of-listening-repeated) | PASS | — |
| `Rfc9777StopReport` | [Stop of listening reported](../../protocol/mld/checks/listener-reports.md#stop-of-listening-reported) | PASS | — |
| `Rfc9777SourceListChange` | [Source list change](../../protocol/mld/checks/listener-reports.md#source-list-change) | PASS | — |
| `Rfc9777ExcludeChange` | [Change inside EXCLUDE mode](../../protocol/mld/checks/listener-reports.md#change-inside-exclude-mode) | PASS | — |
| `Rfc9777ChangeDuringRepetitions` | [A change during the repetitions](../../protocol/mld/checks/listener-reports.md#a-change-during-the-repetitions) | PASS | repaired, [gap 8](#gap-8-defect--a-second-change-replaces-the-pending-records-instead-of-a-merge) |
| `Rfc9777ChangeDuringRepetitionsTail` | [A change during the repetitions](../../protocol/mld/checks/listener-reports.md#a-change-during-the-repetitions) | PASS | — |
| `Rfc9777GeneralQueryResponse` | [Response to a General Query](../../protocol/mld/checks/query-response.md#response-to-a-general-query) | PASS | — |
| `Rfc9777GeneralQueryNoState` | [Response to a General Query](../../protocol/mld/checks/query-response.md#response-to-a-general-query) | PASS | repaired, [gap 11](#gap-11-defect--an-answer-to-a-query-holds-a-record-that-the-standard-leaves-out) |
| `Rfc9777AddressSpecificResponse` | [Response to a Multicast Address Specific Query](../../protocol/mld/checks/query-response.md#response-to-a-multicast-address-specific-query) | PASS | — |
| `Rfc9777AddressSourceResponse` | [Response to a Multicast Address and Source Specific Query](../../protocol/mld/checks/query-response.md#response-to-a-multicast-address-and-source-specific-query) | PASS | repaired, [gap 11](#gap-11-defect--an-answer-to-a-query-holds-a-record-that-the-standard-leaves-out) |
| `Rfc9777StartupQueries` | [General Queries at startup and after it](../../protocol/mld/checks/router-queries.md#general-queries-at-startup-and-after-it) | PASS | — |
| `Rfc9777QuerierElection` | [Querier election](../../protocol/mld/checks/router-queries.md#querier-election) | PASS | — |
| `Rfc9777ForwardingAfterStart` | [Forwarding after a start of listening](../../protocol/mld/checks/router-state.md#forwarding-after-a-start-of-listening) | PASS | — |
| `Rfc9777SourceSpecificForwarding` | [Source-specific forwarding](../../protocol/mld/checks/router-state.md#source-specific-forwarding) | PASS | — |
| `Rfc9777SourceSpecificBlocking` | [Source-specific forwarding](../../protocol/mld/checks/router-state.md#source-specific-forwarding) | PASS | repaired, [gap 15](#gap-15-defect--the-forwarding-asks-for-a-listener-of-the-address-not-of-the-source) |
| `Rfc9777ListeningTimeout` | [Listening timeout without a stop](../../protocol/mld/checks/router-state.md#listening-timeout-without-a-stop) | PASS | repaired, [gap 6](#gap-6-defect--two-default-intervals-have-the-values-of-older-documents) |
| `Rfc9777LastListenerQuery` | [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | PASS | — |
| `Rfc9777LastListenerQuerySFlag` | [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | PASS | repaired, [gap 12](#gap-12-defect--the-s-flag-of-a-multicast-address-specific-query-comes-from-the-timer-before-it-is-lowered) |
| `Rfc9777StopWithAnotherListener` | [Stop of listening with another listener](../../protocol/mld/checks/router-state.md#stop-of-listening-with-another-listener) | PASS | — |
| `Rfc9777SourceBlockedOtherListener` | [Source blocked while another listener wants it](../../protocol/mld/checks/router-state.md#source-blocked-while-another-listener-wants-it) | PASS | repaired, [gap 13](#gap-13-defect--a-report-cancels-the-retransmissions-of-the-queries) |
| `Rfc9777SourceQuerySFlag` | [Source blocked while another listener wants it](../../protocol/mld/checks/router-state.md#source-blocked-while-another-listener-wants-it) | PASS | — |
| `Rfc9777SourceBlockedOnlyListener` | [Source blocked by its only listener](../../protocol/mld/checks/router-state.md#source-blocked-by-its-only-listener) | PASS | repaired, [gap 14](#gap-14-defect--a-multicast-address-and-source-specific-query-does-not-lower-the-source-timers) |
| `Rfc9777RouterAfterStart` | [A router that comes up after a start of listening](../../protocol/mld/checks/router-state.md#a-router-that-comes-up-after-a-start-of-listening) | PASS | — |
| `Rfc9777ListenerV1Mode` | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | PASS | repaired, [gap 9](#gap-9-defect--the-mldv1-mode-of-a-host-answers-at-once-and-sends-one-report) |
| `Rfc9777ListenerV1ModeRepeat` | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | PASS | repaired, [gap 9](#gap-9-defect--the-mldv1-mode-of-a-host-answers-at-once-and-sends-one-report) |
| `Rfc2710ReportSuppression` | [Report suppression in MLDv1 mode](../../protocol/mld/checks/compatibility.md#report-suppression-in-mldv1-mode) | PASS | repaired, [gap 9](#gap-9-defect--the-mldv1-mode-of-a-host-answers-at-once-and-sends-one-report) |
| `Rfc2710DoneInV1Mode` | [Done in MLDv1 mode](../../protocol/mld/checks/compatibility.md#done-in-mldv1-mode) | PASS | — |
| `Rfc2710DoneAtV1Router` | [Done at an MLDv1 router](../../protocol/mld/checks/compatibility.md#done-at-an-mldv1-router) | PASS | — |
| `Rfc9777ListenerBackToV2` | [Listener back to MLDv2](../../protocol/mld/checks/compatibility.md#listener-back-to-mldv2) | PASS | — |
| `Rfc9777OlderQuerierInterval` | [Listener back to MLDv2](../../protocol/mld/checks/compatibility.md#listener-back-to-mldv2) | PASS | repaired, [gap 7](#gap-7-defect--the-mldv1-mode-of-a-host-ends-after-the-other-querier-present-interval) |
| `Rfc9777RouterV1Listener` | [Router with an MLDv1 listener](../../protocol/mld/checks/compatibility.md#router-with-an-mldv1-listener) | PASS | repaired, [gap 16](#gap-16-defect--the-mldv1-node-stops-the-simulation-on-an-mldv2-query) |
| `Rfc9777QuerierConfiguredV1` | [Querier configured for an MLDv1 router](../../protocol/mld/checks/compatibility.md#querier-configured-for-an-mldv1-router) | PASS | repaired, [gap 17](#gap-17-missing-feature--no-mldv1-mode-in-an-mldv2-router) |
| `Rfc9777ModeChangeCancels` | [A mode change cancels the pending reports](../../protocol/mld/checks/compatibility.md#a-mode-change-cancels-the-pending-reports) | PASS | repaired, [gap 10](#gap-10-defect--the-mldv1-mode-keeps-the-pending-mldv2-retransmissions) |
| `Rfc9777BlockInV1Mode` | [A BLOCK record for an address in MLDv1 mode](../../protocol/mld/checks/compatibility.md#a-block-record-for-an-address-in-mldv1-mode) | PASS | repaired, [gap 16](#gap-16-defect--the-mldv1-node-stops-the-simulation-on-an-mldv2-query) |

All 47 tests pass. In the level 2 run, 21 of the 47 tests passed, and the seventeen gaps below
held every failure.

## The class of every failure

**After the repairs** no failure is left. Five tests had premises that the repairs made wrong,
and each has a `tests:` commit of its own before the repair that showed it (see the gaps 2, 5, 7,
9 and 16 below, and the plan): the first Query of a router that waits for its link-local
address; the MLDv1 router of the checks, which is now up at the start; and three steps that took
any Report where the check names the Report for G, since the nodes report their solicited-node
groups too. The repairs also made the failure of a test move to an earlier step while its
verdict stayed FAIL; the plan now compares the reason of every failure with this document after
each gap. The rest of this section classes the failures of the level 2 run.

Twenty-five failures were of the class **defect** of
[the guide](../../../guide/derive-tests-from-a-standard.md#the-class-of-a-failure-and-when-to-declare-it-expected):
the model had code for the behavior, and the code got it wrong. None of them was declared
expected, because no limitation blocked a repair; each gap names the code that a repair would
change. Two of them, `Rfc9777RouterV1Listener` and `Rfc9777BlockInV1Mode`, failed on a defect of
the MLDv1 host of the mockup, not of the MLDv2 router that the checks are about: the run stopped
at the start, and the statements of the router had no verdict until the repair of gap 16. One
failure, `Rfc9777QuerierConfiguredV1`, was an **unimplemented feature**: no code of the MLDv2
module sent an MLDv1 Query, so the test declared `%# expected-result: FAIL` until the feature
came.

No failure of that run was a test error or a misread of the specification. The earlier runs of
the pass found one test error — a repetition step that took a Current-State answer for the
repetition — and two weak checks, which the pass made stronger before this run: the answer to
a source-specific Query could never be empty in its scenario (in both protocols), and the
return to MLDv2 bracketed its interval too loosely to see a difference of 5 seconds. The plan
records them.

The shapes of the defects are the IGMP shapes — a field never filled (gap 1), a default or an
interval of another document (gaps 6 and 7), a state of the host that the model does not keep
or a record that it sends where the standard sends none (gaps 8 to 11), a timer that the router
reads, cancels or does not lower at the wrong moment (gaps 12 to 14), two halves of one
mechanism that do not meet (gap 15) — and four that IPv6 adds: the source address (gap 2), the
Hop-by-Hop Options header (gap 3), the pseudo-header of the checksum (gap 4), and the
solicited-node addresses (gap 5).

## The model gaps

### Gap 1 (defect) — the Queries carry no QRV and no QQIC

- **Tests**: `Rfc9777QueryEncapsulation`, observation 5 (QRV 0 and QQIC 0);
  `Rfc9777QueryRobustness`, observation 2 (QRV 0).
- **Statements**: RFC9777-QRY-13 (the QRV holds the Robustness Variable), QRY-15 (the QQIC holds
  the Query Interval); TIMER-3 (the Robustness Variable is not zero).
- **The code**: `Mldv2::sendGeneralQuery` sets the Maximum Response Code and the length, and
  nothing else ([`Mldv2.cc:1291-1298`](../../../../../src/inet/networklayer/icmpv6/Mldv2.cc));
  the specific Queries are built the same way.
- **Scope**: every Query of every MLDv2 router. The twin of IGMP gap 1.
- **Repaired** on 2026-09-25 (`f0d21017fa`): every Query of an MLDv2 router carries the Robustness
  Variable in the QRV and the 8-bit code of the Query Interval in the QQIC, from the new
  `Mldv2::codeQqic`.

### Gap 2 (defect) — an MLD message leaves from a global address

- **Tests**: `Rfc9777ReportSource`, observation 2 (the Report of A from 2001:db8:1::10);
  `Rfc9777QuerySource`, observation 3 (the Query of R from 2001:db8:1::1);
  `Rfc9777HopLimitAndSource`, observation 3 (the first Query of R, at the start).
- **Statements**: RFC9777-GEN-2, REP-34 (every MLDv2 message, and every Report, from a
  link-local address); QRY-26, RQRY-10 (every Query from a link-local address of fe80::/64).
- **The code**: `Mldv2::sendToIPv6` gives the IPv6 layer the destination, the interface and the
  Hop Limit, and no source address
  ([`Mldv2.cc:1420-1430`](../../../../../src/inet/networklayer/icmpv6/Mldv2.cc)), so IPv6 takes
  the global address of the interface. The MLDv1 module has the same shape: its Reports and
  Queries in the runs leave from global addresses too.
- **Scope**: every MLD message of every node with a global address. A receiver that checks the
  source, as RFC 9777 §6.2 and §7.4 require (level 3), drops every Query and every Report.
- **Repaired** on 2026-09-25 (`6be09e8a02`): both modules give the tested link-local address as the
  source. Before the address is tested the querier waits, a Query is not sent, and a Report or a
  Done waits in the module: the IPv6 layer of the model replaces an unspecified source, which RFC
  3810 §5.2.13 would use. `Ipv6InterfaceData` now emits `interfaceIpv6ConfigChanged` when an address
  becomes tentative or valid. A test repair comes first (`f45e6c99f0`).

### Gap 3 (defect) — an MLD message carries no Router Alert option

- **Test**: `Rfc9777RouterAlert`, observation 2, at the first Query of R.
- **Statement**: RFC9777-GEN-4.
- **The code**: `Mldv2::sendToIPv6` asks for no extension header
  ([`Mldv2.cc:1420-1430`](../../../../../src/inet/networklayer/icmpv6/Mldv2.cc)), and the model
  has no Router Alert option for IPv6: the option types of
  [`Ipv6ExtensionHeaders.msg`](../../../../../src/inet/networklayer/ipv6/Ipv6ExtensionHeaders.msg)
  do not hold type 5 of RFC 2711. No comment names the gap; the level 1 look found it by grep
  ([fact 2](#the-four-facts-of-the-level-1-look)).
- **Scope**: every MLD message of both modules.
- **Repaired** on 2026-09-25 (`a7c86550c8`): every MLD message asks for a Hop-by-Hop Options header
  with the new `Ipv6RouterAlertOption` and a PadN option, and a multicast router delivers a
  multicast datagram locally when it carries the option, instead of every ICMPv6 multicast datagram.
  The Payload Length of `Ipv6::encapsulate` did not count the requested extension headers, and a
  commit before repairs it (`24271dbfb5`).

### Gap 4 (defect) — the ICMPv6 checksum has no pseudo-header

- **Tests**: `Rfc9777ReportChecksum`, observation 5; `Rfc9777QueryChecksum`, observation 6.
- **Statements**: RFC9777-REP-4, QRY-4.
- **The code**: `Icmpv6::insertChecksum` sums the serialized ICMPv6 message alone
  ([`Icmpv6.cc:452-475`](../../../../../src/inet/networklayer/icmpv6/Icmpv6.cc)), and
  `Icmpv6::verifyChecksum` checks the same sum; RFC 9777 §5.1.2 and §5.2.2 take the pseudo-header
  of RFC 8200 §8.1 into the checksum. `mldChecksumOk` of
  [`MldChecks.h`](../../../../../tests/protocol/mld/MldChecks.h) sums the pseudo-header and the
  message; the sum of the message alone is correct in every message of the runs.
- **Scope**: every ICMPv6 message of the model, Neighbor Discovery too. Two nodes of the model
  agree with each other, so no run shows it; a real node, or a capture read by a tool, rejects
  every message.
- **Repaired** on 2026-09-25 (`7e95321201`): a computed checksum waits for the final source address:
  `Ipv6::fragmentPostRouting` computes it over the pseudo-header, with the final destination of a
  Routing header and the home address of a Home Address option, and `Icmpv6` checks it with the
  addresses of `L3AddressInd`.

### Gap 5 (defect) — a node does not report its solicited-node addresses

- **Test**: `Rfc9777SolicitedNodeReport`, observation 5: no Report of A names ff02::1:ff00:a or
  ff02::1:ff00:10 before 10 seconds.
- **Statement**: RFC9777-LSN-3 (MLD messages for every address that a node listens to, except
  ff02::1 and the addresses of scope 0 and 1); the half about ff02::1 holds in
  `Rfc9777StartReport`.
- **The code**: a node accepts a packet to a solicited-node address by a match with its own
  addresses
  ([`Ipv6InterfaceData.cc:375`](../../../../../src/inet/networklayer/ipv6/Ipv6InterfaceData.cc))
  and never joins the group, so MLD never reports it. The ND level 2 pass found the same cause
  from the side of Neighbor Discovery (its gap 9, on the branch `topic/standards-tests-nd-level2`).
- **Scope**: every solicited-node address of every node. A switch that snoops MLD would drop the
  solicitations of address resolution and of Duplicate Address Detection.
- **Repaired** on 2026-09-25 (`f8c5ddfff8`): the change of ND gap 9, applied again: an interface
  joins the solicited-node group of each unicast address, one membership for all addresses of one
  group. Two commits come first: `Mldv1` ignores an MLDv2 Report, which the MLDv2 hosts now send
  before they hear an MLDv1 Query (`367940bc9b`), and a test repair (`8fd55d9ae8`).

### Gap 6 (defect) — two default intervals have the values of older documents

- **Tests**: `Rfc9777StartRepeated`, observation 2 (the repetition comes later than 1 second
  after the first Report); `Rfc9777ListeningTimeout`, observation 2 (the forwarding stops 260 s
  after the last Report of A).
- **Statements**: RFC9777-TIMER-15, LSN-13 (the Unsolicited Report Interval is 1 second);
  TIMER-7, RREP-19 (the Multicast Address Listening Interval is [Robustness Variable] × [Query
  Interval] + 2 × [Query Response Interval], 270 s).
- **The code**: `unsolicitedReportInterval = default(10s)`
  ([`Mldv2.ned:82`](../../../../../src/inet/networklayer/icmpv6/Mldv2.ned)), the value of RFC
  2710 §7.10; RFC 9777 gives 1 second. `groupMembershipInterval =
  default((robustnessVariable * queryInterval) + queryResponseInterval)` (line 76), the formula
  of RFC 3810, which the model claims; RFC 9777 §9.4 adds a second Query Response Interval. The
  level 1 look named the second one ([fact 4](#the-four-facts-of-the-level-1-look)).
- **Scope**: the twin of IGMP gap 3.
- **Repaired** on 2026-09-25 (`0c691f71f5`): the defaults are 1 s and [Robustness Variable] × [Query
  Interval] + 2 × [Query Response Interval]; the Older-Version-Host-Present Timer has its own
  parameter, `olderVersionHostPresentInterval`, and keeps 260 s (RFC 9777 §9.13).

### Gap 7 (defect) — the MLDv1 mode of a host ends after the Other Querier Present Interval

- **Test**: `Rfc9777OlderQuerierInterval`, observation 2: at 290 s, A sends a Version 2 Report.
- **Statement**: RFC9777-TIMER-17 (the Older Version Querier Present Interval is [Robustness
  Variable] × [Query Interval] of the last Query + [Query Response Interval], 260 s).
- **The code**: `Mldv2::processOlderVersionQuery` starts the Older-Version-Querier-Present Timer
  with `otherQuerierPresentInterval`
  ([`Mldv2.cc:1116`](../../../../../src/inet/networklayer/icmpv6/Mldv2.cc)), 255 s with the
  defaults ([`Mldv2.ned:77`](../../../../../src/inet/networklayer/icmpv6/Mldv2.ned)). A is in
  MLDv2 mode again at 286.25 s.
- **Scope**: the twin of IGMP gap 4, with a smaller error: 5 s where IGMP has 95 s. The gap shows
  only when the MLDv1 querier stops.
- **Repaired** on 2026-09-25 (`a9988d5521`): the timer runs for [Robustness Variable] × [Query
  Interval] + [Query Response Interval]. A test repair comes first (`e9f5512b96`): the MLDv1 router
  of the checks is up at the start, with no Duplicate Address Detection.

### Gap 8 (defect) — a second change replaces the pending records instead of a merge

- **Test**: `Rfc9777ChangeDuringRepetitions`, observation 2: the Report at 10.3 s holds
  ALLOW_NEW_SOURCES {S2}, without S1.
- **Statements**: RFC9777-LSN-15, LSN-16, LSN-24.
- **The code**: `Mldv2::multicastSourceListChanged` computes the records of a change from the
  old and the new state, and replaces the pending records with them
  ([`Mldv2.cc:264-275`](../../../../../src/inet/networklayer/icmpv6/Mldv2.cc)).
- **Scope**: the twin of IGMP gap 5.
- **Repaired** on 2026-09-25 (`e0da0ca7d0`): as IGMP gap 5, by Table 2 of RFC 9777 §6.1.

### Gap 9 (defect) — the MLDv1 mode of a host answers at once and sends one Report

- **Tests**: `Rfc9777ListenerV1Mode`, observation 4 (the answer leaves at the instant of the
  Query); `Rfc9777ListenerV1ModeRepeat`, observation 3 (no second Report after a start of
  listening); `Rfc2710ReportSuppression`, observation 2 (A and B both answer).
- **Statements**: RFC2710-NODE-13, NODE-20, NODE-22 (a report timer with a random value);
  NODE-14 (the repetition of the unsolicited Report); NODE-21 (the suppression).
- **The code**: `Mldv2::processOlderVersionQuery` sends the MLDv1 Reports in the event of the
  Query ([`Mldv2.cc:1115-1136`](../../../../../src/inet/networklayer/icmpv6/Mldv2.cc)), and
  `Mldv2::multicastSourceListChanged` sends one MLDv1 Report for a start of listening and
  returns (lines 198-210).
- **Scope**: the twin of IGMP gap 6.
- **Repaired** on 2026-09-25 (`d34f87d387`): the MLDv1 mode follows the node state machine of RFC
  2710 §5, as IGMP gap 6. A test repair comes first (`f4269c2dd4`).

### Gap 10 (defect) — the MLDv1 mode keeps the pending MLDv2 retransmissions

- **Test**: `Rfc9777ModeChangeCancels`, observation 2: A sends a Version 2 Report at 407.19 s,
  after the MLDv1 Query of 406.25 s.
- **Statement**: RFC9777-COMPL-11.
- **The code**: `Mldv2::processOlderVersionQuery` sets the compatibility mode and starts its
  timer ([`Mldv2.cc:1115-1116`](../../../../../src/inet/networklayer/icmpv6/Mldv2.cc)) and
  cancels no timer.
- **Scope**: the twin of IGMP gap 7. The MLDv1 router of the model stops the simulation on the
  Version 2 Report that follows ([fact 1](#the-four-facts-of-the-level-1-look)).
- **Repaired** on 2026-09-25 (`b316802099`): as IGMP gap 7; the change also deletes the Reports of
  the old mode that wait for the link-local address.

### Gap 11 (defect) — an answer to a Query holds a record that the standard leaves out

- **Tests**: `Rfc9777GeneralQueryNoState`, observation 5 (a MODE_IS_INCLUDE record for K, which A
  stopped); `Rfc9777AddressSourceResponse`, observation 5 (a MODE_IS_INCLUDE record with no
  sources, in answer to a Query for S2, which A does not want).
- **Statements**: RFC9777-LTIM-1, REP-22 (one record for each address with listening state, and
  never a MODE_IS_INCLUDE record with an empty source list); LTIM-10 (no Current-State Record
  with an empty source list).
- **The code**: `Mldv2::processHostGeneralQueryTimer` makes a record for every entry of the
  address table ([`Mldv2.cc:583-590`](../../../../../src/inet/networklayer/icmpv6/Mldv2.cc)),
  and a stop keeps the entry; `Mldv2::processHostGroupQueryTimer` sends IS_IN(A ∩ B) without a
  look at its size (lines 632-637).
- **Scope**: the twin of IGMP gap 8.
- **Repaired** on 2026-09-25 (`525b54ebfe`): as IGMP gap 8.

### Gap 12 (defect) — the S flag of a Multicast Address Specific Query comes from the timer before it is lowered

- **Test**: `Rfc9777LastListenerQuerySFlag`, observation 6, at the first Query.
- **Statement**: RFC9777-RQRY-14.
- **The code**: `Mldv2::sendGroupSpecificQuery` computes the flag from the Filter Timer
  ([`Mldv2.cc:1312`](../../../../../src/inet/networklayer/icmpv6/Mldv2.cc)) before it lowers the
  timer; every retransmission sets the flag without a look at the timer (line 544).
- **Scope**: the twin of IGMP gap 9.
- **Repaired** on 2026-09-25 (`64827f317b`): the flag comes from the Filter Timer at each
  transmission, after it is lowered.

### Gap 13 (defect) — a Report cancels the retransmissions of the Queries

- **Test**: `Rfc9777SourceBlockedOtherListener`, observation 3: no second Multicast Address and
  Source Specific Query follows the answer of A.
- **Statements**: RFC9777-RQRY-16; RREP-11 and RST-28 are not reached. The S flag of the second
  Query, RQRY-17 and RQRY-18, has no evidence: `Rfc9777SourceQuerySFlag` passes on the first
  Query alone.
- **The code**: `Mldv2::processReport` cancels the retransmissions of an address when any record
  for it arrives ([`Mldv2.cc:845-851`](../../../../../src/inet/networklayer/icmpv6/Mldv2.cc)).
- **Scope**: the twin of IGMP gap 10.
- **Repaired** on 2026-09-25 (`488bb64208`): together with gap 14, as IGMP gaps 10 and 11.

### Gap 14 (defect) — a Multicast Address and Source Specific Query does not lower the Source Timers

- **Test**: `Rfc9777SourceBlockedOnlyListener`, observation 2: R forwards a datagram of S1 at
  52.5 s, 2.5 s after the stop of its only listener.
- **Statements**: RFC9777-RQRY-2, RQRY-3, RST-25, RST-29, FWD-4; RQRY-15 and RREP-6, the lowering
  itself.
- **The code**: `Mldv2::sendGroupAndSourceSpecificQuery` sends the Query and schedules its
  retransmission, and changes no Source Timer
  ([`Mldv2.cc:1368-1405`](../../../../../src/inet/networklayer/icmpv6/Mldv2.cc)).
- **Scope**: the twin of IGMP gap 11.
- **Repaired** on 2026-09-25 (`488bb64208`): together with gap 13, as IGMP gaps 10 and 11.

### Gap 15 (defect) — the forwarding asks for a listener of the address, not of the source

- **Test**: `Rfc9777SourceSpecificBlocking`, observation 3: the first datagram of S2 arrives on
  L1 at 10.00006 s, although A wants S1 only.
- **Statements**: RFC9777-FWD-5, RST-7; the forwarding of S1 holds in
  `Rfc9777SourceSpecificForwarding`.
- **The code**: IPv6 forwards a multicast datagram onto a leaf interface when
  `Ipv6InterfaceData::hasMulticastListener(address)` holds
  ([`Ipv6.cc:734`](../../../../../src/inet/networklayer/ipv6/Ipv6.cc)). `Mldv2::processReport`
  stores the forwarded sources with `setMulticastListeners`
  ([`Mldv2.cc:1086`](../../../../../src/inet/networklayer/icmpv6/Mldv2.cc)), and
  `hasMulticastListener(address, source)` exists
  ([`Ipv6InterfaceData.h:536`](../../../../../src/inet/networklayer/ipv6/Ipv6InterfaceData.h)),
  but the forwarding never asks it.
- **Scope**: the twin of IGMP gap 12.
- **Repaired** on 2026-09-25 (`dd03fb67e8`): the forwarding of IPv6 asks
  `hasMulticastListener(address, source)`.

### Gap 16 (defect) — the MLDv1 node stops the simulation on an MLDv2 Query

- **Tests**: `Rfc9777RouterV1Listener` and `Rfc9777BlockInV1Mode`: the run stops at the start;
  the run of `Rfc9777QuerierConfiguredV1` stops the same way, in the MLDv1 router R2.
- **Statements**: RFC2710-NODE-2 (a node accepts every Query of at least 24 octets); the
  statements of the router in the two checks — RFC9777-COMPR-12 to COMPR-16, COMPR-19 to
  COMPR-23, TIMER-18 — are not reached.
- **The code**: `Mldv1::processQuery` reads every Query as an MLDv1 Query of 24 octets
  ([`Mldv1.cc:351`](../../../../../src/inet/networklayer/icmpv6/Mldv1.cc)), and the 28 octets of
  an MLDv2 Query cannot be read so: "Cannot convert chunk from type inet::Mldv2Query to type
  inet::MldQuery".
- **Scope**: every link with an MLDv1 node, host or router, and an MLDv2 querier. RFC 9777
  §8.3.2 keeps the MLDv2 querier on MLDv2 Queries in exactly this case (RFC9777-COMPR-23), so the
  MLDv1 interoperation of the model cannot run with its own MLDv1 module.
- **Repaired** on 2026-09-25 (`db78bbed9f`): `Mldv1` reads a Query as an `MldMessage`, the base of
  both Query classes. A test repair comes first (`6d5c859230`).

### Gap 17 (missing feature) — no MLDv1 mode in an MLDv2 router

- **Test**: `Rfc9777QuerierConfiguredV1`, declared expected; the run stops at the start on gap 16.
- **Statements**: RFC9777-COMPR-1, COMPR-3, COMPR-4, COMPR-5, and COMPR-2; COMPR-7 is not
  reached.
- **The code**: `Mldv2` builds only `Mldv2Query` messages
  ([`Mldv2.cc:1291-1298`](../../../../../src/inet/networklayer/icmpv6/Mldv2.cc)) and has no
  parameter for an MLDv1 mode
  ([`Mldv2.ned`](../../../../../src/inet/networklayer/icmpv6/Mldv2.ned)). The router half of RFC
  9777 §8.3.1 is not in the model.
- **Scope**: an MLDv2 router cannot be the querier of a link with an MLDv1 router. The twin of
  IGMP gap 13, where the IGMP standard has the router switch by itself and RFC 9777 has the
  administrator configure it.
- **Added** on 2026-09-28 (`e743ad3166`): the parameter `routerVersion` of `Mldv2` (2 or 1) sets the
  MLDv1 mode of RFC 9777 §8.3.1; only the administrator sets it, and an MLDv1 General Query at a
  router that is not configured for it gets a warning. `Rfc9777QuerierConfiguredV1` sets it, as its
  check says.

## What the model does well

- **The Report** is an ICMPv6 message of type 143 with Hop Limit 1 to ff02::16; its length agrees
  with its records, its Reserved fields are zero, and its records carry no auxiliary data.
- **The Query** is 28 octets long, a General Query goes to ff02::1 with the unspecified address
  and no sources, its Maximum Response Code is 10000, its Code and Reserved fields are zero, and
  its Maximum Response Delay is shorter than the Query Interval.
- **The reports of a listener** follow §6.1: a start of listening sends CHANGE_TO_EXCLUDE_MODE at
  once, a stop CHANGE_TO_INCLUDE_MODE, and a change of the source list ALLOW_NEW_SOURCES and
  BLOCK_OLD_SOURCES records without an empty record, in INCLUDE and in EXCLUDE mode; with the
  interval of the standard, each Report is sent twice and no more. No Report names ff02::1.
- **The answers of a listener** come within the Maximum Response Delay and not at the instant of
  the Query: MODE_IS_EXCLUDE and MODE_IS_INCLUDE records for a General Query, an answer for the
  queried address only, and for a source-specific Query only the queried sources that the host
  wants, when it wants one of them.
- **The querier** sends its General Queries at 0 and 31.25 s and then every 125 s; the election
  gives up the role to a lower address and takes it back 255 s after the last Query of the other
  router.
- **The router state in EXCLUDE mode** is right: a start of listening starts the forwarding
  within one datagram, a router that missed it learns it from the next answer, a stop sends two
  Multicast Address Specific Queries 1 s apart with Maximum Response Code 1000 and ends the
  forwarding within the Last Listener Query Time, and the answer of another listener keeps it.
- **The MLDv1 side of a host** holds its formats: an MLDv2 host in MLDv1 mode sends Reports of 24
  octets to the address and Done messages to ff02::2, and returns to MLDv2 when its timer ends.
  The `Mldv1` router follows its Done procedure of RFC 2710 §6.

## Other findings

- **The documentation of `Mldv2` is stale** ([fact 3](#the-four-facts-of-the-level-1-look)): its
  "parity gaps" say that State-Change Reports and specific Queries are not repeated and that
  MLDv1 interoperation is missing. The runs show all three: `Rfc9777StartRepeatedCount`,
  `Rfc9777LastListenerQuery` and `Rfc9777ListenerV1Mode` reach the code, and the first two pass.
- **`Mldv2` has a lifecycle**, `handleStopOperation` and `handleCrashOperation`
  ([`Mldv2.cc:388-409`](../../../../../src/inet/networklayer/icmpv6/Mldv2.cc)), where `Igmpv3`
  has none ([`igmp/results.md`](../igmp/results.md#other-findings)). The checks never stop a
  node, so no test reads it.
- **No multicast routing of the model forwards in the mockup with a source.** The tests add one
  route with a test module, `MulticastLeafRoute6` of
  [`MldChecks.h`](../../../../../tests/protocol/mld/MldChecks.h), as the IGMP tests do; the
  finding and its reasons are in [`igmp/results.md`](../igmp/results.md#other-findings).
- **A channel that is disabled at initialization never comes back**; the finding is in
  [`igmp/results.md`](../igmp/results.md#other-findings), and the MLD mockups start every link
  enabled for the same reason.
- **The router state had the defects of IGMP**: the source-map loops and the BLOCK row of
  EXCLUDE mode ([`igmp/results.md`](../igmp/results.md#other-findings)); the same commits repair
  them in `Mldv2`.
- **An `Mldv1` node stopped the simulation on an MLDv2 Report**, as on an MLDv2 Query (gap 16):
  `Mldv1::processMldMessage` read every message as an `MldMessage`. Repaired on 2026-09-25 with
  gap 5 (`367940bc9b`); RFC 4443 §2.4 (b) ignores an informational
  message of an unknown type.
- **`Icmpv6` emits `packetDropped` without declaring it.** When a checksum is wrong,
  `Icmpv6::processICMPv6Message` emits the signal, and `Icmpv6.ned` has no `@signal` for it, so a
  debug run stops there. The repair of gap 4 does not make the path more frequent, and leaves it.

## The four facts of the level 1 look

The level 1 pass named four facts for this pass to check
([`conformance.md`](conformance.md#part-1--the-claims)):

1. **`Mldv1` stops the simulation on an unrecognized message type; `Mldv2` does not.** The
   unrecognized type needs a crafted message, level 3. The run found a second case on the
   normal path: `Mldv1` stops on an MLDv2 Query, a type that it knows in a length that it does
   not read (gap 16), and on an MLDv2 Report; both are repaired.
2. **No MLD message carries a Router Alert option, and no comment names it.** Confirmed: gap 3,
   repaired.
3. **The "parity gaps" of `Mldv2.ned` are stale.** Confirmed by the runs: see
   [Other findings](#other-findings).
4. **The Multicast Address Listening Interval is the value of RFC 3810.** Confirmed: 260 s where
   RFC 9777 gives 270 s, gap 6, repaired.

## What the next pass owes

In the order I would do it:

1. **Level 3**, the crafted messages: the validation of every received message, among them the
   source, Hop Limit and Router Alert rules that gaps 2 and 3 would break between real nodes,
   and the unknown types of both modules. MLD message validation is the mandatory feature with
   the fewest checks at level 2.
2. **The Older Version Host Present Interval** has its own parameter since the repair of gap 6,
   and no test measures it: RFC9777-TIMER-19 is owed.
3. **The declaration of `packetDropped` in `Icmpv6.ned`**, see [Other findings](#other-findings).
4. **The level 2 statements this pass left**: the closing list of
   [`checks.md`](../../protocol/mld/checks.md#statements-this-pass-wrote-no-check-for) names what
   each needs.
