# Repair the findings of the TCP parameter tests

**Status:** done on 2026-11-02 on the branch `topic/tcp-parameter-findings`, landed on `master` of `inet-master` on 2026-11-04; the record of the implementation is at the end. Written 2026-11-02 from `master` at `e097360348` of the checkout `inet-master`, which has two untracked plan drafts of other work (`ieee80211-edca-management-recovery.md`, `ieee80211-txop-boundaries-and-rate-selection.md`) that this plan does not touch.

**Approval:** on 2026-11-02 the user asked for this plan and settled three decisions: D1 (PF-1, option 1), D2 (PF-10, option 2, with `stopOperationTimeout = 0s` in the lifecycle examples) and D3 (PF-15, drop the dead code). On 2026-11-02 the user approved this plan (`d94f273099`) with the decisions D4 to D8 and the changes of recorded expectations in C1 and C2 ([PLR-APPROVAL](../../doc/project/rule/planning.md#plr-approval), [TR-BASELINE-DELIBERATE](../../doc/project/rule/testing.md#tr-baseline-deliberate)). The implementation happens in a worktree of the branch `topic/tcp-parameter-findings`.

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
- **D9 (added in the implementation, PF-16): the series repairs the defect of `Tcp` that the cross-class check of C8 found, in a commit of its own (C8a).** A hard ICMP error in SYN_RCVD sent "connection refused" to the application of a forked connection, which has no socket for it, and the dispatcher of a host stopped the run. The approval of the plan did not name this commit; it is separate, so a reviewer can take it or leave it ([PR-SPLIT-ONE-CHANGE](../../doc/project/rule/pull-request.md#pr-split-one-change)). Without it, the cross-class check of the spoof group gives no verdict for one run.

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

### C1 — `tcp: fix: the automatic window scale factor represents the whole advertised window` (PF-4) — done

**Gap and evidence:** the NED says that `windowScalingFactor = -1` selects "the smallest shift count that represents the largest window". `TcpConnection::writeHeaderOptions()` shifts the window down with truncation and starts from one byte less than the window: 131071 bytes get the factor 1, whose largest window is 131070, and 262141 to 262144 bytes get 2, whose largest window is 262140. Linux `tcp_select_initial_window()` gives 3 for 262144.

**Code action:** select the smallest s from 0 to 14 for which 65535 << s is at least the largest window, with integer arithmetic.

**Tests:** the existing `parameter/setup/TcpWindowScalingFactorAuto.test` (262144 bytes, expects 3) turns from FAIL to PASS. A new facet `TcpWindowScalingFactorAutoBoundary.test` checks the boundary: 131071 bytes give 2, and the scaled window of the first ACK carries the whole offer. The expected output of `tests/module/tcp_synack_window_1.test` changes: the window field of its ACK at 4.203 s follows the factor 2 (a recorded expectation: [TR-BASELINE-DELIBERATE](../../doc/project/rule/testing.md#tr-baseline-deliberate)).

**Documents:** the reviewer document `tests/protocol/tcp/window-scale-selection.md`; the release note `doc/whatsnew/fixes/tcp-window-scale-auto.txt`.

**Verification:** `opp_run_protocol_tests -m debug -w tcp -f 'parameter/setup/TcpWindowScalingFactor'`, `cross_class.py TcpWindowScalingFactorAuto TcpWindowScalingFactorAutoBoundary`, `opp_run_module_tests -m debug -f tcp_synack_window`, and the fingerprint tests of the TCP examples. Limit: the tests check the factor and the first scaled windows, not a long transfer.

### C2 — `tcp: fix: TcpWestwood takes an RTT sample from each ACK of new data` (PF-6) — done

**Gap and evidence:** `TcpWestwood::receivedAckForUnackedData()` calls `sentInfo.clearTo(snd_una)` before `sentInfo.get(firstSeqAcked)`, so the item of the first acknowledged segment is always gone. `w_RTTmin` keeps `SIMTIME_MAX`, the threshold never comes from the bandwidth estimate, and after a fast retransmission the window becomes 4294967295 and then wraps to 0.

**Code action:** read the transmit information of the acknowledged segment before the list is cleared.

**Tests:** the existing `parameter/congestion/TcpAlgorithmClassWestwood.test` turns from FAIL to PASS. A new facet `TcpAlgorithmClassWestwoodRto.test` (class `westwood-rto`) checks the timeout of Westwood, which the coverage gap loop could not test before: a tail loss without TLP, and at the timeout the threshold is the bandwidth estimate times the smallest RTT divided by `w_a`, and at least 2 MSS (calibrated from a run, then checked against the formula with the state of the algorithm, level 3, because no signal carries the estimate). The fingerprints of `inet-westwood` change ([opp_update_fingerprint_test_results](../../doc/project/guide/change-a-baseline.md)), and the statistical baseline too if U3 finds it there.

**Documents:** `tests/protocol/tcp/westwood-rtt-sample.md`; `doc/whatsnew/fixes/tcp-westwood-rtt-sample.txt`.

**Verification:** the two tests and their cross-class check, the regression of the protocol suites, `opp_run_fingerprint_tests -m release -f tcpclientserver`. Limit: one path with one loss.

### C3 — `examples: change: the lifecycle examples with TCP keep the stop of TCP at once` (prepares D2) — not needed: U2 found no example that changes

**Purpose:** the examples that U2 finds get `**.tcp.stopOperationTimeout = 0s` in their configurations, with a comment that names the reason. With the code of today this value gives the same results (the verified fact above), so this commit changes no fingerprint; after C4 it keeps the deletion at once. The commit comes before C4 so that no commit of the series changes these baselines.

**Verification:** the fingerprint tests of these examples give the same values before and after the commit.

### C4 — `tcp: change: the stop of Tcp closes its connections and waits for them` (PF-10, PF-9) — done

**Gap and evidence:** see D2 and the example of PF-9 above.

**Design:** `Tcp::handleStopOperation()` does this:

1. With `stopOperationTimeout = 0s`, it deletes all connections at once, as today.
2. Else it issues the CLOSE call of RFC 9293 section 3.10.4 to each connection that its application did not close: a listening connection and one in SYN_SENT end at once; one in SYN_RECEIVED, ESTABLISHED or CLOSE_WAIT sends its queued data and a FIN. It reuses the existing CLOSE processing of `TcpConnection` ([AR-EXT-REUSE](../../doc/project/rule/architecture.md#ar-ext-reuse)), and it sends no indication to the application (U1).
3. The stage waits while a connection has data or a FIN that the peer has not acknowledged (FIN_WAIT_1, CLOSING, LAST_ACK). A connection in FIN_WAIT_2 or TIME_WAIT needs nothing more from this node; the stage does not wait for the 2 MSL of TIME_WAIT, which would always reach the timeout. During the wait, `Tcp` is in the state STOPPING and handles segments and timers as usual.
4. When no connection waits, the stage deletes the rest and calls `startActiveOperationExtraTimeOrFinish()`. When nothing waits from the start, it calls it at once, without `delayActiveOperationFinish()`, so a negative extra time ends the stage inside the handler, as the mixin means it (PF-9). The timeout of `stopOperationTimeout` deletes all connections at once through the crash handler, as today.

**Tests:** the classes of `Tcp.stopOperationTimeout` change, and the tests of the group `module` change with them: `zero` (0s: the connections end at once, without a segment), `waits` (2s, the default: a connection that the application left open sends its FIN, and the stage ends when the peer acknowledges it; host2 sees the FIN, and the echo replies of the pings stop after it), and `expires` (the tap drops the segments of the peer: the stage ends at the timeout, without the FIN exchange). The classes of `stopOperationExtraTime` stay; `TcpStopOperationExtraTimeNegativeAlias.test` turns from FAIL to PASS. The examples of PF-9 and PF-10 in the guide and in the catalog describe the repaired state ([DR-WHAT-IS](../../doc/project/rule/documentation.md#dr-what-is)).

**Documents:** `tests/protocol/tcp/stop-closes-connections.md`, with a diagram of the stop with the FIN exchange and one of the timeout; `doc/whatsnew/incompatible/tcp-stop-closes-connections.txt` and the section `doc/src/migration-guide/development/tcp-stop-operation.rst`, which names `stopOperationTimeout = 0s` for the old behavior ([RR-BREAK-MIGRATE](../../doc/project/rule/release.md#rr-break-migrate)).

**Verification:** the group `module` and its cross-class check, the regression of the protocol suites, the fingerprint tests of all examples with a stop (no change after C3), and `opp_run_module_tests -m debug -f lifecycle`. Limit: the tests use one connection; the BGP examples check more connections only through their fingerprints.

### C5 — `tcp: fix: a stopped Tcp drops the segments that reach it` (PF-14) — done

**Gap and evidence:** with a positive extra time, the network layer stays up after the stage of TCP has set TCP down, and a SYN of the peer stops the run with "Message 'SYN' received when Tcp is down".

**Code action:** `Tcp::handleMessageWhenDown()` drops a packet from the network layer, with the signal `packetDropped`, and keeps the error of the mixin for the other messages (D5).

**Tests:** `parameter/module/TcpStopOperationExtraTimeOtherSegment.test` turns from FAIL to PASS.

**Documents:** `tests/protocol/tcp/stopped-tcp-drops-segments.md`; `doc/whatsnew/fixes/tcp-stopped-drops-segments.txt`.

**Verification:** the group `module` and its cross-class check; the regression.

### C6 — `tcp: add: checks of the negative timers of TCP` (PF-2, PF-8) — done

`delayedAckTimeout`, `keepAliveIdleTime` and `keepAliveInterval` get a check at the start of a connection with a message about the parameter, in place of the error of the scheduler, and their NED comments name the range. Three new error tests: `parameter/ack/TcpDelayedAckTimeoutInvalid.test`, `parameter/keepalive/TcpKeepAliveIdleTimeInvalid.test`, `TcpKeepAliveIntervalInvalid.test`, with their catalog rows (class `invalid`). No protocol behavior changes, so no reviewer document. Release note `doc/whatsnew/compatible/tcp-negative-timer-checks.txt`.

### C7 — `tcp: change: the NED comments name the errors and the socket option of three parameters` (PF-3, PF-5, PF-7) — done

`windowScalingFactor` names the error of a factor whose largest window is below `advertisedWindow`, `advertisedWindow` names the error above 65535 << 14, and `nagleEnabled` names `TcpSocket::setNoDelay()` (TCP_NODELAY), which takes its place for one connection. The last one is a claim, so it gets the class `Tcp.nagleEnabled:socket-override` and its test `parameter/send/TcpNagleEnabledSocketOverride.test`. No code changes.

### C8 — `tcp: fix: TcpSpoof declares the checksum of its segments correct` (PF-11) — done

**Gap and evidence:** `TcpSpoof::sendSpoofPacket()` sets no checksum mode, so `Tcp::checkChecksum()` of the receiver stops the run with "unknown checksum mode: -1" at the first spoofed segment.

**Code action:** the header gets the mode "declared correct" (D6).

**Tests:** `parameter/spoof/TcpSpoofIsSYNTrueAnswer.test` turns from FAIL to PASS. The other spoof tests lose their tap rule that drops the segment, and their descriptions lose the text about PF-11; their observation stays on the wire. The cross-class check then runs for each spoof test.

**Documents:** `tests/protocol/tcp/tcpspoof-checksum-mode.md`; `doc/whatsnew/fixes/tcp-tcpspoof-checksum-mode.txt`.

### C8a — `tcp: fix: a hard ICMP error in SYN_RCVD informs only an application that has the connection` (PF-16, D9) — done

**Requirement, gap and evidence:** RFC 9293 section 3.9.2.2 says that a hard ICMP error SHOULD abort the connection (SHLD-26), and section 3.10.7.3 says for a RST in SYN-RECEIVED that the user of a passive OPEN "need not be informed". `TcpConnection::processIcmpv4Error()` and `processIcmpv6Error()` send `TCP_I_CONNECTION_REFUSED` for a connection that is active, forked or Fast Open. A forked connection in SYN_RCVD without Fast Open has not sent `TCP_I_AVAILABLE`, so its application has no socket for it; in a `StandardHost` the `MessageDispatcher` stops the run with "Unknown socket". The cross-class check of C8 found it: `TcpSpoofIsSYNFalse.test` with `isSYN = true` gets a SYN-ACK of host2 back as Protocol Unreachable, because host1 has no TCP.

**Existing tests:** `tests/module/tcp_icmp_synrcvd_1.test` and `tcp_icmp6_synrcvd_1.test` use a server that does not fork; no test has a forked connection in SYN_RCVD that gets an ICMP error.

**Tests (all new):**
- `tests/module/tcp_icmp_synrcvd_2.test` and `tcp_icmp6_synrcvd_2.test`: the copies of the `_1` tests with `srv_app.fork = true` and `srv_app.logIndications = true`. The tester answers the SYN-ACK of the server with Destination Unreachable 3/2 (ICMPv4) or 1/4 (ICMPv6). Expected trace: the forked connection goes from SYN_RCVD to CLOSED, no indication reaches the application, and the retransmitted SYN of the client at 3 s forks a new connection, which the application accepts. Before the change the trace has `srv_app: TCP_I_CONNECTION_REFUSED` after the abort.
- `tests/protocol/tcp/parameter/spoof/TcpSpoofIsSYNTrueNoTcp.test`, facet `true-no-tcp` of `TcpSpoof.isSYN`: host1 has `TcpSpoof` and no TCP, host2 listens with `TcpEchoApp` (a forked listener). Steps: the SYN of the spoof at 1 s, the SYN-ACK of host2, the Protocol Unreachable of host1, and no segment of host2 in the next 3 s. Before the change the run stops at the ICMP error with "Unknown socket"; after it, the test passes. With `isSYN = false` it fails, because no SYN-ACK comes.

**Code action:** the condition of the indication follows the RST in SYN_RCVD (`TcpConnection::processRstInSynReceived()`): an active OPEN, or a Fast Open connection that the application has (`!isToBeAccepted()`). The state transition stays.

**Runner:** `opp_run_module_tests -m debug -f "tcp_icmp6?_synrcvd"`; `opp_run_protocol_tests -m debug -w tcp -f parameter/spoof/`; the cross-class check of the spoof group.

**Documents:** `tests/protocol/tcp/icmp-synrcvd-forked.md`; `doc/whatsnew/fixes/tcp-icmp-synrcvd-forked.txt`.

### C9 — `tcp: fix: TcpSpoof takes the ISS of the clock as Tcp does` (PF-13) — done

**Gap and evidence:** `TcpSpoof::chooseInitialSeqNum()` calls `fmod()` with a `simtime_t`, so the modulus 2^32 + 1 becomes a `simtime_t` too, which the time scale cannot hold; `seqNo = -1` stops each run.

**Code action:** compute the ISS as `Tcp` does (D8), with integer arithmetic; reuse the computation of `TcpConnection` if it can be shared, else write the same expression.

**Tests:** `parameter/spoof/TcpSpoofSeqNoIss.test` (250000 at 1 s) turns from FAIL to PASS, and `TcpSpoofSeqNoValue.test` then fails with -1, as the cross-class check requires. The example of PF-13 in the guide describes the repaired state.

**Documents:** `tests/protocol/tcp/tcpspoof-iss.md`; `doc/whatsnew/fixes/tcp-tcpspoof-iss.txt`.

### C10 — `tcp: change: TcpSpoof needs a destination address` (PF-12) — done

`destAddress` loses its default, and `TcpSpoof::initialize()` stops the run with a message about the parameter when the value is empty (D7). `parameter/spoof/TcpSpoofDestAddressEmpty.test` checks the new message, at the start of the run. Release note `doc/whatsnew/compatible/tcp-tcpspoof-dest-address.txt`.

### C11 — `tcp: remove: the dead code that the coverage of the TCP tests found` (PF-15) — done

Remove the class `DcTcpFamily` (`DcTcpFamily.h`, `DcTcpFamily.cc`; `DcTcpFamilyState.msg` stays, because `DcTcp` uses it) and the default branch of `TcpAlgorithmBase::getInitialWindow()`. The removal of a public C++ class is incompatible for a user who derives from it: release note `doc/whatsnew/incompatible/tcp-dctcpfamily-removed.txt`. Verification: the build, the congestion group, and the DcTcp tests.

### C12 — `applications: change: the NED of TcpSessionApp says what tClose = -1 does` (PF-1) — done

The NED comment of `tClose` says that -1 closes the connection after the last send (D1); `parameter/ini/_bulk.ini` loses its note about the finding. A new protocol test `tests/protocol/tcp/app/TcpSessionAppCloseAfterLastSend.test` checks the stated behavior: host1 sends 10000 bytes with `tClose = -1`, and its FIN follows its last data segment. No behavior changes, so no release note.

### C12a — `tests: change: the README of the TCP parameter tests counts the tests of the repairs` — done

Added in the implementation: the table of [tests/protocol/tcp/parameter/README.md](../../tests/protocol/tcp/parameter/README.md) gives the number of tests of each folder, and C1, C2, C4, C6, C7 and C8a changed seven of them without an update of the table. One commit corrects the table and the total (339 tests).

### C13 — `plan: change: record the repairs of the TCP parameter findings`, and the move to `plan/done/` — done

The record of the facts that the implementation finds, then the move of this plan as a separate commit ([PR-SPLIT-MOVE](../../doc/project/rule/pull-request.md#pr-split-move)).

## Verification of the whole series

- The parameter suite: 330 tests and the new ones; after C12, no unexpected FAIL is left from the findings.
- The cross-class check for each new and changed test, and for the spoof group after C8.
- The regression of all protocol suites against `e097360348`: no other verdict changes. On that commit, `wifi/common/WifiBlockAckDataRetry.test` already fails because of the 802.11 commits of `master`; this plan does not touch it.
- The fingerprint tests: only the entries of `inet-westwood` change (C2); the module test `tcp_synack_window_1` changes its expected output (C1).
- The commit gate `doc/project/enforcement/check-commits.sh` and the gates of the documents.

## Out of scope

The lines that the coverage gap loop sorted into the kinds (b) and (c) — the socket API and what the segments of the peer decide — belong to a test class of the socket API and to the standards tests. The failure of `WifiBlockAckDataRetry.test` on `master` belongs to the 802.11 work.

## The record of the implementation

**C1.**
- The one-byte error came only from the SYN of an active opener; the SYN-ACK of a passive opener had the exact window, and only the truncation of the loop made it too small. So `tests/module/tcp_synack_window_1.test`, a passive opener with 131072 bytes, already announced 2 and passes unchanged: no recorded expectation changes in C1.
- C1 adds a second facet, `auto-boundary-synack` (`TcpWindowScalingFactorAutoBoundarySynAck.test`), so that the truncation on the passive side has a test too. The three tests of the automatic factor fail with the selection of before (run in a build of this branch with the old code put back) and pass after it.
- The packetdrill wrappers (inet-gpl master `363e9ea`, built in debug mode against the worktree) keep their verdicts: 303 PASS and 3 expected FAILs, as on `e097360348`. The regression of the protocol suites has one unexpected FAIL less.

**C2.**
- The base call `TcpAlgorithmBase::receivedAckForUnackedData()` sends nothing, so the item of the first acknowledged segment is still in the list after it; only the explicit `clearTo()` before `get()` removed it. The repair moves the `clearTo()` after the sample, as in TcpVegas.
- `TcpAlgorithmClassWestwood.test` and the new `TcpAlgorithmClassWestwoodRto.test` fail with the old code and pass after the repair; the timeout facet compares the signal with the formula in a lambda that reads the state of the connection (`TcpConnection::getState()`, cast to `TcpWestwoodStateVariables`).
- The fingerprints of 21 entries of `examples/inet/tcpclientserver`, `inet-westwood`, change, and no other entry of the example (`opp_update_fingerprint_test_results -m debug --working-directory-filter tcpclientserver --config-filter '^inet-westwood$'`). The `TcpLwip` configurations of the example give errors in this build, because the feature is off; that is not related.
- U3: the statistical baseline has `examples/inet/tcpclientserver/omnetpp.ini-inet-westwood-#0.{sca,csv}`, so the `statistics` repository gets a matching commit, made in the copy of the GitHub job under `~/workspace/ghci-statistical`.
- The statistical baseline commit is `518a800` (`tcp: update the inet-westwood statistics after TcpWestwood takes its RTT sample`) on `master` of `inet-master/statistics`, made in the copy of the GitHub job at INET `b0263002b7` and OMNeT++ `e91154f414`; before the update the test failed (`ssthresh:vector:count` of the client connection 0, now 7), after it, it passes.

**C3 and C4.**
- U1: every message of a connection to its application passes `Tcp::sendToApp()`, so during the wait (the state STOPPING) `Tcp` drops these messages. A second fact came from `tests/module/tcpapp_lifecycle_7.test`: an application closes its socket in its own stage, and its CLOSE arrives as an event of the same time, so after the stage of TCP has closed the connection already; the run stopped with "Duplicate CLOSE command". The old code had deleted the connection, and the late message reached a TCP that was down already. So `Tcp::handleUpperCommand()` also ignores the commands of the applications during the wait.
- The timeout bounds only the wait: `Tcp::finishStop()` cancels it before the extra time. The old class `shorter` (a timeout that cuts the extra time) does not exist any more; the classes of `stopOperationTimeout` are `zero`, `waits` and `expires`, and `TcpStopOperationTimeoutLonger.test` and `TcpStopOperationTimeoutShorter.test` are gone.
- The FIN of the stop goes out inside the action `stopNode` of the tester, before the next step listens, so the tests observe it at the TCP of host2.
- U2: the fingerprint tests of all examples, showcases and tutorials that stop nodes (207 runs, the BGP lifecycle examples among them) keep their values with C4; the only errors are the `TcpLwip` configurations of `examples/inet/shutdownrestart`, whose feature is off in this build. So C3 has nothing to do, and no example needs `stopOperationTimeout = 0s`.
- The 51 lifecycle module tests pass. In the module suite, 7 tests of IEEE 802.1Q tags (`Ieee8021qTag*`) fail; this branch changes nothing in `src/inet/linklayer`, so they come from the VLAN commits of `master`.
- The NED comments of `stopOperationTimeout` and `stopOperationExtraTime` now say what the parameters do, and the examples of PF-9 and PF-10 in the guide no longer describe the defects as current.

**C5.**
- `Tcp::handleMessageWhenDown()` follows `MacProtocolBase::handleMessageWhenDown()`: a message from the lower gate (`TransportProtocolBase::isLowerMessage()`) is dropped, and the other messages keep the error of the mixin. A packet is dropped with `packetDropped` and the reason `OTHER_PACKET_DROP`, because no reason of `PacketDropReason` names a protocol that is down; an ICMP error indication is not a packet and gets no signal.
- `TcpStopOperationExtraTimeOtherSegment.test` fails with C4 alone and passes with C5; with `0s` it fails, as the cross-class check requires.

**C6.**
- The checks follow the sibling check of `maxPersistTimeout` in `TcpAlgorithm::initialize()`: `"<parameter> must be 0 or more, but it is %s"` with `SimTime::str()`. 0 stays valid, because `Tcp.delayedAckTimeout:zero` is a class and the keepalive times have a defined behavior at 0; the findings named only the negative values that reach the scheduler.
- The keepalive times are checked at the open also when keepalive is off, because `TcpAlgorithmBase::initializeAlgorithm()` reads them always.

**C7.**
- The wording of the override follows `notsentLowat` ("sets it for one connection"). `TcpNagleEnabledSocketOverride.test` uses a `TcpTestApp` that sets TCP_NODELAY on its socket; a copy without that call failed, so the test depends on the socket option. The texts of `TcpNagleEnabledOn.test` and `TcpNagleEnabledOff.test` now name the new class instead of PF-5.
- The source of `windowScalingFactor:too-small` and `advertisedWindow:too-large` is now "NED; code", because the NED names their errors.

**C8.**
- The spoof group gives 13 PASS and 1 FAIL (`TcpSpoofSeqNoIss.test`, PF-13). The generator of the group lost the tap rule and the text about PF-11, and the 13 other tests keep their observations on the wire. The regression of all protocol suites changes one verdict against C7: `TcpSpoofIsSYNTrueAnswer.test` from FAIL to PASS.
- The cross-class check of the group found a defect of `Tcp` that the tap rule hid: `TcpSpoofIsSYNFalse.test` with `isSYN = true` gave no verdict, because the run stopped with "Unknown socket" in the dispatcher of host2. host1 has no TCP, so its IP answers the SYN-ACK of host2 with Protocol Unreachable, and the forked connection of host2 sent "connection refused" to a socket that its application did not have. This is PF-16; C8a repairs it (D9).
- No renderer of mermaid is on this machine, so the rendering of the diagrams of the reviewer documents is unverified ([TR-PLAN-PROTOCOL](../../doc/project/rule/testing.md#tr-plan-protocol)).

**C8a.**
- The defect came with `d366c7dc53` (`tcp: fix: a hard ICMP error aborts a connection in SYN_RCVD too`), whose message says that only an application that has a socket for the connection gets the indication; its condition named `forked`, but a forked connection in SYN_RCVD has no socket in its application before `TCP_I_AVAILABLE`.
- The two module tests and the spoof facet fail with the old condition and pass with the new one. The cross-class check of the spoof group gives a verdict for each run again, except the two runs of PF-13: `TcpSpoofSeqNoIss.test` is not built for the cross-class run because it fails, and `TcpSpoofSeqNoValue.test` with `-1` stops with the error of PF-13.
- The 808 module tests of the filter `tcp` give 801 PASS and the 7 FAILs of `Ieee8021qTag*`; the packetdrill wrappers keep 303 PASS and 3 expected FAILs.

**C9.**
- `TcpConnection::selectInitialSeqNum()` computes the ISS with a double, not with integer arithmetic, as the code action of C9 assumed. `TcpSpoof::chooseInitialSeqNum()` takes the same expression, so both give the same number at the same time, also at the boundary of a tick. The expression is one line, and `tcp_common` has no shared place for a function that both modules could call, so the commit writes it again with a comment that names its source.
- The spoof group gives 15 PASS, and the cross-class check of the group gives 8 runs without a problem: `TcpSpoofSeqNoIss.test` fails with `12345`, and `TcpSpoofSeqNoValue.test` fails with `-1`. Only the spoof tests use `TcpSpoof` in the tree, so no other suite changes.
- The text of PF-13 named the modulus 2^32 + 1; the expression `1.0 + 0xffffffff` is 2^32, and the catalog row now says so.

**C10.**
- The message follows the checks of C6: `"destAddress must name a node or an address, but it is empty"`. The check is only for an empty value; a name resolves at the send, after the configuration of the network, as before.
- `TcpSpoofDestAddressEmpty.test` now expects the new message at the start; it fails with the old code, where the run stopped at 1 s with "Address contains no value". The text of the classes of `destAddress` is shared, so `TcpSpoofDestAddressHost.test` and `TcpSpoofDestAddressHostIpv6.test` lose the text about PF-12 too. The spoof group gives 15 PASS, and the cross-class check 8 runs without a problem.

**C11.**
- `DcTcpFamily.cc` also held `str()` and `detailedInfo()` of `DcTcpFamilyStateVariables`, which `DcTcp` uses; they moved to `DcTcp.cc`, as `TcpWestwood.cc` holds the functions of its own state. `DcTcp.h` includes `DcTcpFamilyState_m.h` in place of `DcTcpFamily.h`, and the comment of the message names `DcTcp`.
- Both callers of `TcpAlgorithmBase::getInitialWindow()` call it only for the values `rfc3390` and `rfc6928` of `initialWindow` or with `initialCwnd`, so the function now asserts the mode and has no branch for `rfc2001`.
- The removal of a public C++ class needs a section of the migration guide ([RR-BREAK-MIGRATE](../../doc/project/rule/release.md#rr-break-migrate)), which the code action of C11 did not name: `doc/src/migration-guide/development/tcp-dctcpfamily-removed.rst`.
- The parameter suite gives 339 PASS; the fingerprints of `examples/inet/dctcp` and `examples/inet/tcpthroughput` (6 runs) keep their values; the 808 module tests of the filter `tcp` give 801 PASS and the 7 FAILs of `Ieee8021qTag*`.

**C12.**
- `TcpSessionApp::sendData()` schedules the CLOSE at `max(tClose, now)` after the last command, so each time before the last send acts as -1 does; the NED comment says that, not only the value -1. The application always has a command, because an empty script stops the run at the start.
- The new test `tests/protocol/tcp/app/TcpSessionAppCloseAfterLastSend.test` passes, and a copy with `tClose = 1000s` fails at the step of the FIN. The code does not change, so the test passes before this commit too, and no release note is needed.

**C12a.**
- The table now gives 14 (`ack/`), 37 (`setup/`), 18 (`send/`), 82 (`congestion/`), 13 (`keepalive/`), 11 (`module/`) and 15 (`spoof/`) tests, 339 in total. The coverage table of the catalog keeps 330, because it records the coverage run of 2026-11-02.

**The whole series.**
- The parameter suite gives 339 PASS. The cross-class check ran for each new and changed test in its commit; the spoof group gives 8 runs without a problem.
- The regression of all protocol suites against `e097360348` changes only the verdicts of this series: the 6 failing tests of the findings pass, and the new tests pass. 68 unexpected FAILs remain, the same as on `e097360348`, and none of them is a TCP test.
- The 808 module tests of the filter `tcp` give 801 PASS and the 7 FAILs of `Ieee8021qTag*`, which fail on `master` too. The packetdrill wrappers of inet-gpl (`363e9ea`, debug) keep 303 PASS and 3 expected FAILs after C11.
- The fingerprint tests of the whole tree (1792 runs, debug mode, from the root of the worktree) give 1768 PASS and no changed fingerprint, so only the 21 entries of `inet-westwood` changed (C2). The 22 unexpected errors are configurations whose feature is off in this build (`TcpLwip`, `VoipStreamSender`, `Z3GateScheduleConfigurator`) and `tests/fingerprint/ethernet-nonstandardspeed.ini` with half duplex at 5 Gbps; four runs that use no TCP show "DONE (unexpected)" in their run line, and their fingerprint checks pass.
- The first commits wrote `tcp` as the area of the Change trailer and only `test` as the obligation. Before C13 a rewrite of the messages (no change of the trees) gave them the form of [CR-TAG-FORM](../../doc/project/rule/classification.md#cr-tag-form): the area `src.tcp`, `src.tcp.TcpSpoof` or `src.applications.TcpSessionApp`, the obligations `whatsnew`, `migration`, `fingerprint` and `statistical` where they apply, and a sentence about the baselines where the body had none.
- The gates `check-commits.sh` and `check-whatsnew.sh` pass on `d94f273099..HEAD`; one note says that the body of C4 has more than 300 words.
- The statistics repository has the matching commit `518a800` on `master` of `inet-master/statistics` (C2). It reached GitHub on 2026-11-02 before this series, so the statistical test of `inet-westwood` fails on GitHub until the INET commits follow.
- On 2026-11-04 the series moved onto `origin/master` `2f8fbcc493`, which already has the plan commit as `cd4cadb2c6`; the 40 new commits change no TCP source and no TCP test. On that base: the protocol suites give 67 unexpected FAILs, none of them a TCP test (`Rfc4861RaSolicited` passes now, after the ICMPv6 repairs of `master`); the 823 module tests of the filter `tcp` pass; the packetdrill wrappers keep 303 PASS and 3 expected FAILs; the fingerprint tests (1792 runs) give 1767 PASS and one FAIL, the configuration `Handover` of `examples/ipv6/mipv6`, which fails with the same values on GitHub for `2f8fbcc493` alone; the other results are as before. A last move onto `0244045621` adds one refactor of `ChunkSerializerRegistry`; the build and the TCP protocol tests pass there.
