# Repair the findings of the TCP parameter tests

**Status:** pending, design only; no commit of this plan exists yet. Written 2026-11-02 from `master` at `e097360348` of the checkout `inet-master`, which has two untracked plan drafts of other work (`ieee80211-edca-management-recovery.md`, `ieee80211-txop-boundaries-and-rate-selection.md`) that this plan does not touch.

**Approval:** on 2026-11-02 the user asked for this plan and settled three decisions: D1 (PF-1, option 1), D2 (PF-10, option 2, with `stopOperationTimeout = 0s` in the lifecycle examples) and D3 (PF-15, drop the dead code). The plan as a whole, the proposed decisions D4 to D8, and the changes of recorded expectations in C1 and C2 wait for the approval of the user ([PLR-APPROVAL](../../doc/project/rule/planning.md#plr-approval), [TR-BASELINE-DELIBERATE](../../doc/project/rule/testing.md#tr-baseline-deliberate)). The implementation happens in a worktree of the branch `topic/tcp-parameter-findings`.

## The problem

The parameter tests of TCP ([the procedure](../../doc/project/guide/derive-tests-from-parameters.md), [the catalog](../../doc/project/evidence/model/tcp/parameters.md#findings)) found 15 defects and gaps in the modules of `transportlayer/tcp`, in `TcpSpoof` and in `TcpSessionApp`. That work changed no model code; it recorded each finding and kept the 6 tests that fail because of one. This plan repairs the findings.

The findings are of five kinds:

| Kind | Findings | The repair | The failing test that becomes the acceptance test |
| --- | --- | --- | --- |
| a defect of TCP | PF-4, PF-6, PF-9, PF-10, PF-14 | change the code | `TcpWindowScalingFactorAuto`, `TcpAlgorithmClassWestwood`, `TcpStopOperationExtraTimeNegativeAlias`, `TcpStopOperationExtraTimeOtherSegment` |
| a defect of `TcpSpoof` | PF-11, PF-12, PF-13 | change the code | `TcpSpoofIsSYNTrueAnswer`, `TcpSpoofSeqNoIss` |
| a missing check of a value | PF-2, PF-8 | add a check and its error test | — |
| a gap of the documentation | PF-1, PF-3, PF-5, PF-7 | change the NED comment, and test the stated behavior | — |
| dead code | PF-15 | remove it | — |

An example of a defect: a scenario stops a node with `**.tcp.stopOperationExtraTime = -1s`, the default of the 28 other modules of INET that have this parameter. At the stop, `Tcp::handleStopOperation()` calls `delayActiveOperationFinish()` and then `startActiveOperationExtraTimeOrFinish()`, whose branch for a negative value invokes the done callback of the lifecycle controller inside `handleOperationStage()`. The run stops with "Usage error: callback may only be invoked if (and after) you returned 'false' from initiateStateChange()" (PF-9). After the repair, the stage of TCP ends at once, as the mixin means it for a negative value.

## Decisions

- **D1 (the user, PF-1): document what `tClose = -1` does.** `TcpSessionApp` closes the connection after its last send when `tClose = -1`; its NED says that -1 issues no CLOSE. 101 ini and test files of the tree set `tClose = -1`, so a change of the code would change all of their runs and baselines. The NED comment changes, and the code stays.
- **D2 (the user, PF-10): the stop of `Tcp` closes its connections and waits for them, up to `stopOperationTimeout`.** Today the stage deletes all connections at once, without a segment, and waits for nothing, so the timeout acts only when the extra time is longer. With D2 the parameter gets the meaning that its name gives it, and the FIXME in `Tcp::handleStopOperation()` ("close connections??? yes, because the applications may not close them!!!") is answered. `stopOperationTimeout = 0s` keeps the deletion at once; the lifecycle examples whose results change get this value, so their baselines stay (C3). A crash does not change: `handleCrashOperation()` deletes the connections at once, as before.
- **D3 (the user, PF-15): remove the dead code.** The default branch of `TcpAlgorithmBase::getInitialWindow()` and the class `DcTcpFamily`.
- **D4 (proposed): one plan, one commit for each finding or for each group of findings with one cause.** `TcpSpoof` and `TcpSessionApp` are outside the module `Tcp`, but each of their repairs is small and has its own commit, so a reviewer can take them one at a time ([PR-SPLIT-ONE-CHANGE](../../doc/project/rule/pull-request.md#pr-split-one-change)).
- **D5 (proposed, PF-14): a stopped `Tcp` drops a segment that reaches it, without an answer and without an error.** A node whose TCP is down sends no RST, as a node whose TCP is off.
- **D6 (proposed, PF-11): `TcpSpoof` declares the checksum of its segments correct.** That is the default of `checksumMode` of `Tcp` too ([Tcp.ned](../../src/inet/transportlayer/tcp/Tcp.ned)); a computed checksum is a later option, which no finding asks for.
- **D7 (proposed, PF-12): `destAddress` of `TcpSpoof` has no default, and an empty value stops the run at the start with a message about the parameter.** The default `""` can never work.
- **D8 (proposed, PF-13): `seqNo = -1` of `TcpSpoof` takes the ISS of the clock as `Tcp` does: one tick in 4 µs, modulo 2^32** (RFC 9293 section 3.4.1, as the NED comment of `initialSendSequenceNumber` says).

## Verified facts

- The 6 failing tests and their causes are in the catalog rows and in the descriptions of the tests in [tests/protocol/tcp/parameter/](../../tests/protocol/tcp/parameter/README.md). On `e097360348` the parameter suite gives 324 PASS and these 6 FAIL.
- `Tcp::handleStopOperation()` calls `reset()`, `delayActiveOperationFinish(stopOperationTimeout)` and `startActiveOperationExtraTimeOrFinish(stopOperationExtraTime)`, in this order ([Tcp.cc](../../src/inet/transportlayer/tcp/Tcp.cc)); `OperationalMixin` ends the stage at the first of the timeout and the end of the extra time ([OperationalMixinImpl.h](../../src/inet/common/lifecycle/OperationalMixinImpl.h)). `Tcp` does not override `handleMessageWhenDown()`, so a message that reaches it later than the time of the change stops the run.
- The code shows that `stopOperationTimeout = 0s` ends the stage of today at the time of the stop: the timeout and the extra timer (`stopOperationExtraTime = 0s`) expire at that time, the timeout goes first because the extra timer has the lowest scheduling priority, and the stage ends in the crash handler. C3 relies on this and checks it with the fingerprints.
- The examples that stop nodes and run TCP over the stop are the BGP examples (`examples/bgpv4/BgpLifecycle`, `BgpWithdrawal`, `BgpWithdrawal6`, `BgpDiamondFailover`, `BgpDiamondFailover6`); the fingerprint store [tests/fingerprint/store.json](../../tests/fingerprint/store.json) has entries of them. The other examples with stops (AODV, DSDV, DYMO, GPSR, OSPF, RIP, DHCP, MPLS, IEEE 802.1D, QUIC) may run TCP applications too; C3 finds out by measurement.
- The only configuration of the tree whose automatic window scale factor changes with PF-4 is [tcp_synack_window_1.test](../../tests/module/tcp_synack_window_1.test) (`advertisedWindow = 131072`: the code gives the factor 1, and 2 is right). The other windows above 65535 in the tree (350000, 5000000) keep their factor.
- `examples/inet/tcpclientserver`, `[Config inet-westwood]`, is the only configuration with `TcpWestwood`, and the fingerprint store has its entries.
- `DcTcp` derives from `TcpReno` and uses only `DcTcpFamilyStateVariables` of `DcTcpFamily`; no NED type, no other class and nothing in inet-gpl uses the class `DcTcpFamily`.

## Unresolved facts

- **U1 (blocks C4): what the connections that TCP closes at the stop send to their applications.** The stage of the applications ends before the stage of TCP, and an application that is down stops the run on a message. C4 needs a way to close a connection without an indication to its application. Check: read how `TcpConnection` sends indications (`sendIndicationToApp()`), and whether a connection can be detached from its application; run `BgpLifecycle` with C4 and look for "received when ... is down".
- **U2 (blocks C3): which examples change their results with C4.** Check: run the fingerprint tests of all examples with a stop, with C4 on top of C3, and compare.
- **U3 (C2): whether the statistical baseline holds `inet-westwood`.** Check: `opp_run_statistical_tests -m release -f tcpclientserver` before and after C2.

## The commits

The commits are in the order of their dependencies ([PR-SERIES-ORDER](../../doc/project/rule/pull-request.md#pr-series-order)). C1, C2, C6, C7, C8 to C12 depend on nothing in this plan; C4 depends on C3, and C5 on C4. Each commit updates the catalog rows that it changes, marks its finding as repaired (the catalog is a ledger and keeps the row), and changes the descriptions of the tests that it turns from FAIL to PASS ([TR-SHIP-WITH](../../doc/project/rule/testing.md#tr-ship-with)). Each commit that changes the behavior of a protocol has the reviewer document of [TR-PLAN-PROTOCOL](../../doc/project/rule/testing.md#tr-plan-protocol) beside the tests, and each commit that changes behavior has its release note file ([RR-WHATSNEW-FILE](../../doc/project/rule/release.md#rr-whatsnew-file)).

### C1 — `tcp: fix: the automatic window scale factor represents the whole advertised window` (PF-4)

**Gap and evidence:** the NED says that `windowScalingFactor = -1` selects "the smallest shift count that represents the largest window". `TcpConnection::writeHeaderOptions()` shifts the window down with truncation and starts from one byte less than the window: 131071 bytes get the factor 1, whose largest window is 131070, and 262141 to 262144 bytes get 2, whose largest window is 262140. Linux `tcp_select_initial_window()` gives 3 for 262144.

**Code action:** select the smallest s from 0 to 14 for which 65535 << s is at least the largest window, with integer arithmetic.

**Tests:** the existing `parameter/setup/TcpWindowScalingFactorAuto.test` (262144 bytes, expects 3) turns from FAIL to PASS. A new facet `TcpWindowScalingFactorAutoBoundary.test` checks the boundary: 131071 bytes give 2, and the scaled window of the first ACK carries the whole offer. The expected output of `tests/module/tcp_synack_window_1.test` changes: the window field of its ACK at 4.203 s follows the factor 2 (a recorded expectation: [TR-BASELINE-DELIBERATE](../../doc/project/rule/testing.md#tr-baseline-deliberate)).

**Documents:** the reviewer document `tests/protocol/tcp/window-scale-selection.md`; the release note `doc/whatsnew/fixes/tcp-window-scale-auto.txt`.

**Verification:** `opp_run_protocol_tests -m debug -w tcp -f 'parameter/setup/TcpWindowScalingFactor'`, `cross_class.py TcpWindowScalingFactorAuto TcpWindowScalingFactorAutoBoundary`, `opp_run_module_tests -m debug -f tcp_synack_window`, and the fingerprint tests of the TCP examples. Limit: the tests check the factor and the first scaled windows, not a long transfer.

### C2 — `tcp: fix: TcpWestwood takes an RTT sample from each ACK of new data` (PF-6)

**Gap and evidence:** `TcpWestwood::receivedAckForUnackedData()` calls `sentInfo.clearTo(snd_una)` before `sentInfo.get(firstSeqAcked)`, so the item of the first acknowledged segment is always gone. `w_RTTmin` keeps `SIMTIME_MAX`, the threshold never comes from the bandwidth estimate, and after a fast retransmission the window becomes 4294967295 and then wraps to 0.

**Code action:** read the transmit information of the acknowledged segment before the list is cleared.

**Tests:** the existing `parameter/congestion/TcpAlgorithmClassWestwood.test` turns from FAIL to PASS. A new facet `TcpAlgorithmClassWestwoodRto.test` (class `westwood-rto`) checks the timeout of Westwood, which the coverage gap loop could not test before: a tail loss without TLP, and at the timeout the threshold is the bandwidth estimate times the smallest RTT divided by `w_a`, and at least 2 MSS (calibrated from a run, then checked against the formula with the state of the algorithm, level 3, because no signal carries the estimate). The fingerprints of `inet-westwood` change ([opp_update_fingerprint_test_results](../../doc/project/guide/change-a-baseline.md)), and the statistical baseline too if U3 finds it there.

**Documents:** `tests/protocol/tcp/westwood-rtt-sample.md`; `doc/whatsnew/fixes/tcp-westwood-rtt-sample.txt`.

**Verification:** the two tests and their cross-class check, the regression of the protocol suites, `opp_run_fingerprint_tests -m release -f tcpclientserver`. Limit: one path with one loss.

### C3 — `examples: change: the lifecycle examples with TCP keep the stop of TCP at once` (prepares D2)

**Purpose:** the examples that U2 finds get `**.tcp.stopOperationTimeout = 0s` in their configurations, with a comment that names the reason. With the code of today this value gives the same results (the verified fact above), so this commit changes no fingerprint; after C4 it keeps the deletion at once. The commit comes before C4 so that no commit of the series changes these baselines.

**Verification:** the fingerprint tests of these examples give the same values before and after the commit.

### C4 — `tcp: change: the stop of Tcp closes its connections and waits for them` (PF-10, PF-9)

**Gap and evidence:** see D2 and the example of PF-9 above.

**Design:** `Tcp::handleStopOperation()` does this:

1. With `stopOperationTimeout = 0s`, it deletes all connections at once, as today.
2. Else it issues the CLOSE call of RFC 9293 section 3.10.4 to each connection that its application did not close: a listening connection and one in SYN_SENT end at once; one in SYN_RECEIVED, ESTABLISHED or CLOSE_WAIT sends its queued data and a FIN. It reuses the existing CLOSE processing of `TcpConnection` ([AR-EXT-REUSE](../../doc/project/rule/architecture.md#ar-ext-reuse)), and it sends no indication to the application (U1).
3. The stage waits while a connection has data or a FIN that the peer has not acknowledged (FIN_WAIT_1, CLOSING, LAST_ACK). A connection in FIN_WAIT_2 or TIME_WAIT needs nothing more from this node; the stage does not wait for the 2 MSL of TIME_WAIT, which would always reach the timeout. During the wait, `Tcp` is in the state STOPPING and handles segments and timers as usual.
4. When no connection waits, the stage deletes the rest and calls `startActiveOperationExtraTimeOrFinish()`. When nothing waits from the start, it calls it at once, without `delayActiveOperationFinish()`, so a negative extra time ends the stage inside the handler, as the mixin means it (PF-9). The timeout of `stopOperationTimeout` deletes all connections at once through the crash handler, as today.

**Tests:** the classes of `Tcp.stopOperationTimeout` change, and the tests of the group `module` change with them: `zero` (0s: the connections end at once, without a segment), `waits` (2s, the default: a connection that the application left open sends its FIN, and the stage ends when the peer acknowledges it; host2 sees the FIN, and the echo replies of the pings stop after it), and `expires` (the tap drops the segments of the peer: the stage ends at the timeout, without the FIN exchange). The classes of `stopOperationExtraTime` stay; `TcpStopOperationExtraTimeNegativeAlias.test` turns from FAIL to PASS. The examples of PF-9 and PF-10 in the guide and in the catalog describe the repaired state ([DR-WHAT-IS](../../doc/project/rule/documentation.md#dr-what-is)).

**Documents:** `tests/protocol/tcp/stop-closes-connections.md`, with a diagram of the stop with the FIN exchange and one of the timeout; `doc/whatsnew/incompatible/tcp-stop-closes-connections.txt` and the section `doc/src/migration-guide/development/tcp-stop-operation.rst`, which names `stopOperationTimeout = 0s` for the old behavior ([RR-BREAK-MIGRATE](../../doc/project/rule/release.md#rr-break-migrate)).

**Verification:** the group `module` and its cross-class check, the regression of the protocol suites, the fingerprint tests of all examples with a stop (no change after C3), and `opp_run_module_tests -m debug -f lifecycle`. Limit: the tests use one connection; the BGP examples check more connections only through their fingerprints.

### C5 — `tcp: fix: a stopped Tcp drops the segments that reach it` (PF-14)

**Gap and evidence:** with a positive extra time, the network layer stays up after the stage of TCP has set TCP down, and a SYN of the peer stops the run with "Message 'SYN' received when Tcp is down".

**Code action:** `Tcp::handleMessageWhenDown()` drops a packet from the network layer, with the signal `packetDropped`, and keeps the error of the mixin for the other messages (D5).

**Tests:** `parameter/module/TcpStopOperationExtraTimeOtherSegment.test` turns from FAIL to PASS.

**Documents:** `tests/protocol/tcp/stopped-tcp-drops-segments.md`; `doc/whatsnew/fixes/tcp-stopped-drops-segments.txt`.

**Verification:** the group `module` and its cross-class check; the regression.

### C6 — `tcp: add: checks of the negative timers of TCP` (PF-2, PF-8)

`delayedAckTimeout`, `keepAliveIdleTime` and `keepAliveInterval` get a check at the start of a connection with a message about the parameter, in place of the error of the scheduler, and their NED comments name the range. Three new error tests: `parameter/ack/TcpDelayedAckTimeoutInvalid.test`, `parameter/keepalive/TcpKeepAliveIdleTimeInvalid.test`, `TcpKeepAliveIntervalInvalid.test`, with their catalog rows (class `invalid`). No protocol behavior changes, so no reviewer document. Release note `doc/whatsnew/compatible/tcp-negative-timer-checks.txt`.

### C7 — `tcp: change: the NED comments name the errors and the socket option of three parameters` (PF-3, PF-5, PF-7)

`windowScalingFactor` names the error of a factor whose largest window is below `advertisedWindow`, `advertisedWindow` names the error above 65535 << 14, and `nagleEnabled` names `TcpSocket::setNoDelay()` (TCP_NODELAY), which takes its place for one connection. The last one is a claim, so it gets the class `Tcp.nagleEnabled:socket-override` and its test `parameter/send/TcpNagleEnabledSocketOverride.test`. No code changes.

### C8 — `tcp: fix: TcpSpoof declares the checksum of its segments correct` (PF-11)

**Gap and evidence:** `TcpSpoof::sendSpoofPacket()` sets no checksum mode, so `Tcp::checkChecksum()` of the receiver stops the run with "unknown checksum mode: -1" at the first spoofed segment.

**Code action:** the header gets the mode "declared correct" (D6).

**Tests:** `parameter/spoof/TcpSpoofIsSYNTrueAnswer.test` turns from FAIL to PASS. The other spoof tests lose their tap rule that drops the segment, and their descriptions lose the text about PF-11; their observation stays on the wire. The cross-class check then runs for each spoof test.

**Documents:** `tests/protocol/tcp/tcpspoof-checksum-mode.md`; `doc/whatsnew/fixes/tcp-tcpspoof-checksum-mode.txt`.

### C9 — `tcp: fix: TcpSpoof takes the ISS of the clock as Tcp does` (PF-13)

**Gap and evidence:** `TcpSpoof::chooseInitialSeqNum()` calls `fmod()` with a `simtime_t`, so the modulus 2^32 + 1 becomes a `simtime_t` too, which the time scale cannot hold; `seqNo = -1` stops each run.

**Code action:** compute the ISS as `Tcp` does (D8), with integer arithmetic; reuse the computation of `TcpConnection` if it can be shared, else write the same expression.

**Tests:** `parameter/spoof/TcpSpoofSeqNoIss.test` (250000 at 1 s) turns from FAIL to PASS, and `TcpSpoofSeqNoValue.test` then fails with -1, as the cross-class check requires. The example of PF-13 in the guide describes the repaired state.

**Documents:** `tests/protocol/tcp/tcpspoof-iss.md`; `doc/whatsnew/fixes/tcp-tcpspoof-iss.txt`.

### C10 — `tcp: change: TcpSpoof needs a destination address` (PF-12)

`destAddress` loses its default, and `TcpSpoof::initialize()` stops the run with a message about the parameter when the value is empty (D7). `parameter/spoof/TcpSpoofDestAddressEmpty.test` checks the new message, at the start of the run. Release note `doc/whatsnew/compatible/tcp-tcpspoof-dest-address.txt`.

### C11 — `tcp: remove: the dead code that the coverage of the TCP tests found` (PF-15)

Remove the class `DcTcpFamily` (`DcTcpFamily.h`, `DcTcpFamily.cc`; `DcTcpFamilyState.msg` stays, because `DcTcp` uses it) and the default branch of `TcpAlgorithmBase::getInitialWindow()`. The removal of a public C++ class is incompatible for a user who derives from it: release note `doc/whatsnew/incompatible/tcp-dctcpfamily-removed.txt`. Verification: the build, the congestion group, and the DcTcp tests.

### C12 — `applications: change: the NED of TcpSessionApp says what tClose = -1 does` (PF-1)

The NED comment of `tClose` says that -1 closes the connection after the last send (D1); `parameter/ini/_bulk.ini` loses its note about the finding. A new protocol test `tests/protocol/tcp/app/TcpSessionAppCloseAfterLastSend.test` checks the stated behavior: host1 sends 10000 bytes with `tClose = -1`, and its FIN follows its last data segment. No behavior changes, so no release note.

### C13 — `plan: change: record the repairs of the TCP parameter findings`, and the move to `plan/done/`

The record of the facts that the implementation finds, then the move of this plan as a separate commit ([PR-SPLIT-MOVE](../../doc/project/rule/pull-request.md#pr-split-move)).

## Verification of the whole series

- The parameter suite: 330 tests and the new ones; after C12, no unexpected FAIL is left from the findings.
- The cross-class check for each new and changed test, and for the spoof group after C8.
- The regression of all protocol suites against `e097360348`: no other verdict changes. On that commit, `wifi/common/WifiBlockAckDataRetry.test` already fails because of the 802.11 commits of `master`; this plan does not touch it.
- The fingerprint tests: only the entries of `inet-westwood` change (C2); the module test `tcp_synack_window_1` changes its expected output (C1).
- The commit gate `doc/project/enforcement/check-commits.sh` and the gates of the documents.

## Out of scope

The lines that the coverage gap loop sorted into the kinds (b) and (c) — the socket API and what the segments of the peer decide — belong to a test class of the socket API and to the standards tests. The failure of `WifiBlockAckDataRetry.test` on `master` belongs to the 802.11 work.
