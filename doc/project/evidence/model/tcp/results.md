# TCP checks — run results and model analysis (pass 4, level 4)

> **Kind:** report · **Status:** snapshot 2026-09-14 · **Seal:** none · **Owns:** — · **Stands on:** [catalog.md](../../standard/rfc9293/catalog.md), [rfc6298/catalog.md](../../standard/rfc6298/catalog.md), [rfc5681/catalog.md](../../standard/rfc5681/catalog.md), [checks.md](../../protocol/tcp/checks.md)

Step 7 artifact of the standards test workflow. This is the first document of the TCP workflow that may reference code.

- Date: 2026-09-14 12:31 +0200
- INET: branch `topic/rfc-tests-tcp-level4`, commit `e0ac3b7307`, tree clean
- Trees: src `72114fab35`, tests/protocol `4add3d87de` — the commit left master in the landing rebase; the same src tree is on master from `83396aa655` to `ab1447501a`
- OMNeT++: 6.4.0
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-31-generic x86_64
- Command, after the `setenv` scripts of OMNeT++ and INET:

  ```sh
  cd tests/protocol/lib && MODE=debug ./build.sh
  inet_run_protocol_tests -p inet -w tcp
  ```
- Earlier passes ran on other trees. The pass log of [`coverage.md`](coverage.md#pass-log) names them. Every verdict below is the verdict of the run above, except the rows that name a later date: a later commit changed those by hand, and each row names it. The fresh run of 2026-09-23 in [`coverage.md`](coverage.md) confirms every one of them.

## Verdicts

| Test | Checks | Verdict |
| --- | --- | --- |
| Rfc9293ConnectionEstablishment.test | RFC9293-EST-1, EST-2, SEQ-1, HDR-1, CKSUM-1; ISS-1, OPT-1 | PASS |
| Rfc9293DataTransfer.test | RFC9293-DATA-1, SEG-1, ACK-1; ACK-2 | PASS |
| Rfc9293Push.test | RFC9293-PSH-1 | **PASS** since 2026-10-14 — gap 1 closed by `tcp: change: PSH in the last buffered segment by default` |
| Rfc9293ConnectionTermination.test | RFC9293-FIN-1, FIN-2 | PASS |
| Rfc9293FlowControl.test | RFC9293-WND-1, WND-2; ACKD-1 | PASS |
| Rfc9293Reset.test | RFC9293-RST-1 | PASS |

Pass 3, level 3:

| Test | Checks | Verdict |
| --- | --- | --- |
| Rfc9293ChecksumDiscard.test | RFC9293-CKSUM-2 | PASS |
| Rfc9293OutOfWindowSegment.test | RFC9293-SEGA-1, SEGA-2, RST-2 | PASS |
| Rfc9293BlindReset.test | RFC9293-RSTP-1 | PASS |
| Rfc9293ValidReset.test | RFC9293-RSTP-2; covers RST-1 | PASS |
| Rfc9293NoResetForReset.test | RFC9293-RST-3 | PASS |
| Rfc9293ShrunkWindow.test | RFC9293-WND-4 | PASS |
| Rfc9293NoWindowShrink.test | RFC9293-WND-3 | PASS |
| Rfc9293SoftIcmpError.test | RFC9293-ICMP-3; covers ICMP-1 | PASS |
| Rfc9293ChecksumDefault.test | RFC9293-CKSUM-1 (the value) | **FAIL (expected)** — defect with a blocked repair, gap 2, declared since 2026-09-17; see below |
| Rfc9293ShrunkWindowNoNewData.test | RFC9293-WND-5 | **PASS** since 2026-09-15; gap 3 was a **test error**, not a defect |
| Rfc9293SourceQuench.test | RFC9293-ICMP-2 | **PASS** since 2026-09-14; gap 4 is closed by the ICMP repair |

Summary after pass 3: 19 tests, 15 PASS, 4 FAIL, each failure declared with `%# expected-result: FAIL`. The other four suites of the same tree stay where they were: IPv4 23 (17 PASS, 6 expected FAIL), UDP 13 (10 PASS, 3 expected FAIL), IPv6 29 (21 PASS, 8 expected FAIL), QUIC 6 PASS.

Eight of the eleven new tests pass, and they are the ones that matter most for a protocol that carries a connection: a corrupt segment is discarded and the stream still arrives; a segment outside the window draws an acknowledgment and never a reset; a reset from a third party who cannot guess the sequence numbers is ignored, while one that fits is obeyed; a reset draws no reset; a shrunk window does not break the sender; and a soft ICMP error does not end the connection. Those are the rules that keep a connection alive under attack and under a fault, and the model keeps them.

Summary of pass 2: 8 tests, 7 PASS, 1 FAIL (expected), in 2.1 s. The runner's overall verdict is PASS, because the one failure is declared. The SYN's header length was captured as 24 octets: the MSS option is present, so the `should` of RFC9293-OPT-1 is met.

## Pass 4, level 4: the two control loops

RFC 6298 and RFC 5681 entered the in-scope set, and ten checks were written against them. The suite is 27 tests: **22 PASS, 2 FAIL (expected), 3 FAIL (unexpected)**.

Since 2026-09-14 it is **23 PASS, 1 FAIL (expected), 3 FAIL (unexpected)**. `Rfc9293SourceQuench.test` passes and its declaration is removed: the ICMP repair that the IPv4 suite asked for closed gap 4 as well, exactly as that test predicted.

A fresh run on master, commit `28536bd0a5`, gives **25 PASS, 2 FAIL (expected), 0 FAIL (unexpected)**: gap 5 was repaired by `f3064a83d0`, gap 3 was a test error, and gap 2 is a declared defect whose repair is blocked. See [`coverage.md`](coverage.md) for the later verdicts.

| Test | Checks | Verdict |
| --- | --- | --- |
| Rfc6298InitialTimeout.test | RFC6298-INIT-1 | PASS |
| Rfc6298FirstMeasurement.test | RFC6298-FIRST-1, RFC6298-RTO-1; covers MIN-1 | **PASS** since 2026-09-15; gap 5 is repaired |
| Rfc6298BackoffDoubling.test | RFC6298-BACK-1, RFC6298-EARLY-1 | PASS |
| Rfc6298KarnsRule.test | RFC6298-KARN-1 | PASS |
| Rfc6298TimeoutAfterLostSyn.test | RFC6298-SYN-1 | PASS |
| Rfc5681InitialWindow.test | RFC5681-IW-1, RFC5681-IW-2 | PASS |
| Rfc5681SlowStartGrowth.test | RFC5681-SS-2; covers SS-1, SSTH-1 | PASS |
| Rfc5681TimeoutResponse.test | RFC5681-LOSS-1, RFC5681-LOSS-3 | PASS |
| Rfc5681FastRetransmit.test | RFC5681-FR-1, FR-2, FR-3, FR-5; covers ACK-2 | PASS |
| Rfc5681WindowAfterLostSyn.test | RFC5681-IW-3 | PASS |

Nine of the ten pass. Both control loops are written, and the one difference the checks find is a defect rather than an absence.

## The RFC defaults (2026-10-14)

This is not a new pass. It is a run of the same checks after the defaults of `Tcp` changed to those of the RFCs. The new defaults are SACK, timestamps and window scaling; delayed ACKs and Limited Transmit; an advertised window of 65535 octets; the MSS of the address family; the initial window of RFC 5681; NewReno; an initial RTO of 1 s; RACK-TLP and PRR; PSH on the last buffered segment; the passive ECN; and path MTU discovery. `LinuxTcp` has the defaults of Linux, and `LegacyTcp` those of master before the change. The owner decided on 2026-09-29 that the tests run the defaults of `Tcp` and override a parameter only where a check needs it, with the reason in the ini.

- Date: 2026-10-14 17:36 +0200
- INET: branch `topic/tcp-modern-defaults`, the commit `tcp: add: LinuxTcp, the defaults of Linux`, tree clean
- Trees: src `d5bd1c1f6f`, tests/protocol `61bfc48f7f`
- OMNeT++: 6.4.0, commit `cf58891643`
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-38-generic x86_64
- Command, after the `setenv` scripts of OMNeT++ and INET: `inet_run_protocol_tests -p inet --filter '(tcp|self)/'`
- Result: TCP 27 tests, **26 PASS, 1 FAIL (expected), 0 FAIL (unexpected)**; the 21 protocol self tests pass. The declared failure is gap 2 (the checksum default). Gap 1 (PSH) is closed: the default `pushLastBufferedSegment` sets the bit, and `Rfc9293Push.test` passes and declares nothing.

The tests were first made to hold under new defaults on the branch `topic/tcp-new-audit-fixes`, on 2026-09-29. There, 15 of the 27 tests and two self tests failed. The analysis found four defects of the model and some test errors. The staged landing of that branch brought the repair of each defect together with the feature, so no defect reached master:

| Finding on the branch | Seen in | On master |
| --- | --- | --- |
| The tail loss probe stayed armed after the acknowledgment of all data; in TIME_WAIT it sent the FIN again, and the peer answered with a reset | Rfc9293OutOfWindowSegment (a forbidden reset at t=0.6) | part of `tcp: add: Tail Loss Probe (RFC 8985 section 7), selected by tlpEnabled` |
| RFC 6298 §5.7 was missing: after a SYN timeout the RTO stayed at the 1 s default and did not go to 3 s | Rfc6298TimeoutAfterLostSyn | `tcp: add: an RTO of 3 s after a SYN timeout (RFC 6298 section 5.7)` |
| The default initial window was IW10 (RFC 6928), above the bound of RFC 5681 §3.1 | Rfc5681InitialWindow | the default is the bound of RFC 5681 (`tcp: change: the initial window of RFC 5681 by default`); IW10 is the option `initialWindow = "rfc6928"` |
| An ICMP net unreachable aborted a connection in SYN_SENT, against RFC 9293 MUST-56 | the self test ReactiveInject | the abort is the option `softIcmpErrorsAbortSetup`, off by default (`tcp: add: the soft ICMP errors of Linux abort a setup, by a parameter`) |

The last two are Linux behavior. The model keeps them as options, and `LinuxTcp` selects them.

**Test errors and their repair.** The commit `tests: change: the TCP standards tests hold under the RFC defaults` repairs the tests that hold under the old defaults too. A test that needs a new default changes in the commit of that default.

| Class | Tests | Repair |
| --- | --- | --- |
| A blocking `never` step hid the next event: SACK and RACK repair a loss within a millisecond, and host A sends its next segments in the same flight | SoftIcmpError, SourceQuench, ValidReset, OutOfWindowSegment, ChecksumDiscard | the guard runs beside the next steps (`meanwhile`); ChecksumDiscard forbids an acknowledgment of the discarded data only until the retransmission |
| A constant of the old defaults: 536-octet segments, a 20-octet ACK header | DataTransfer, ConnectionEstablishment | the effective send MSS is read from the run (RFC 9293 §3.7.1); the header length must be 20 octets plus the options |
| The scenario no longer made the condition: host B's fourth acknowledgment was its FIN, after the whole stream | ShrunkWindow, and **ShrunkWindowNoNewData, which passed without looking at anything** | a 30000-octet stream, and the shrink on host B's first acknowledgment of data |
| One segment where the check needs two | KarnsRule | 2000 octets, and the guard starts at the retransmission |
| The check reads the rule of one algorithm, the Reno of RFC 5681, which CUBIC (RFC 9438) changes on purpose | SlowStartGrowth, TimeoutResponse, FastRetransmit | the ini selects Reno; FastRetransmit also turns off SACK, RACK and PRR |
| The tail loss probe re-arms the retransmission timer (RFC 8985 §7.3) | BackoffDoubling, TimeoutAfterLostSyn | the probe is off |
| A framework test needed a SYN four octets longer than the ACK | the self test CaptureWithUnit | three options are off |

Master needed two more repairs. `Tcp` takes no RTT sample from the handshake (that is `LinuxTcp`), so with timestamps the first sample comes from the timestamp clock, which counts milliseconds (RFC 7323 §4.1). The sample is then up to 1 ms below the time on the link, and `Rfc6298FirstMeasurement.test` widens its bands by 1 ms. And the Nagle algorithm of RFC 9293 holds the second, partial segment of `Rfc6298KarnsRule.test` until the first one is acknowledged, where the source branch sends both at once; the test keeps its steps, and its description says so.

FastRetransmit also changed its observable. The model keeps the window of Linux: without SACK, each duplicate acknowledgment takes one segment out of the data counted in flight, and the published `cwnd` stays at the threshold. So the check no longer reads the published value; a guard forbids new data in recovery that takes FlightSize past the threshold plus 3 + k segments. On the branch, a comparison with master's module test `tcp_fastrexmit_1` found three defects of that recovery; the stages that brought the recovery to master include the repairs, for example `tcp: change: set ssthresh from FlightSize, as RFC 5681 equation (4) says`.

On the branch, each new guard and assertion was checked to fail on a copy with one changed detail, and each copy failed at the intended place.

## Linux equivalence: the packetdrill suite

This is not a pass of the workflow. It is the run of the implementation oracle of TCP ([Evidence from an implementation oracle](../../../guide/derive-tests-from-a-standard.md#evidence-from-an-implementation-oracle)): the packetdrill scripts of Linux run in the `PacketDrillApp` of inet-gpl, each one through its wrapper in [`tests/protocol/tcp/linux/`](../../../../../tests/protocol/tcp/linux/README.md) or [`tests/protocol/tcp/packetdrill/`](../../../../../tests/protocol/tcp/packetdrill/README.md). The `tcp.ini` of the suite selects `LinuxTcp`, so each result speaks for `LinuxTcp`, not for the RFC defaults of `Tcp`.

- Date: 2026-10-15 18:01 +0200
- INET: master `3d3b535fc8`, the code of this run; the commit of this section changes only documents
- inet-gpl: master `4a3894d`
- Scripts: 321 in `tests/protocol/tcp` of inet-gpl: 159 Linux selftests in `linux/`, and 162 scripts of upstream packetdrill `2c4001c4d6fc` in `packetdrill/`
- Oracle: Linux 7.1.3, with the result of each script in `tests/packetdrill/tcp/linux-results.csv` of inet-gpl
- OMNeT++: 6.4.0, commit `cf58891643`
- Build: debug, INET and inet-gpl built from these commits
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-38-generic x86_64
- Commands: `make test` and `make summary` in `tests/packetdrill` of inet-gpl; then, with `INETGPL_ROOT` set, `inet_run_protocol_tests -p inet --filter 'tcp/'`
- Result of the suite: 318 scripts run, **303 MATCH**, 12 KERNEL_DRIFT, 3 UNSUPPORTED_FEATURE; 3 scripts are skipped, because they need IPv6
- Result of INET's runner: TCP 333 tests, **329 PASS, 4 FAIL (expected), 0 FAIL (unexpected)**. The 306 wrappers give 303 PASS and 3 FAIL (expected); the 27 RFC tests give 26 PASS and 1 FAIL (expected), gap 2

The 15 scripts without a wrapper are the 12 that Linux 7.1.3 itself does not pass (kernel drift) and the 3 that need IPv6; the READMEs of the two folders list them with the reason. The 3 wrappers that expect `FAIL` change a `tc qdisc` in the middle of the script, and INET does not model the drops of the local queue: an unimplemented feature.

**What the run shows.** Each of the 303 wrappers that expect `PASS` passes: on every script that Linux passes and INET can run, `LinuxTcp` sends and accepts the same segments as Linux 7.1.3. Eleven features of the feature map have scripts, and the column "Linux equivalence" of [conformance.md](conformance.md#part-2--conformance-matrix) gives their counts. Most scripts exercise documents that the TCP catalogs do not have, and so show where the catalogs stop:

| Document outside the TCP catalogs | Scripts |
| --- | --- |
| RFC 7413, TCP Fast Open | 80 |
| RFC 9768, Accurate ECN | 58 |
| RFC 2018 and RFC 6675, SACK and its loss recovery | 8 |
| RFC 6937, Proportional Rate Reduction | 8 |
| RFC 9293 section 3.7.4, the Nagle algorithm | 6 |
| RFC 7323, timestamps | 5 |
| RFC 3042, Limited Transmit | 4 |
| RFC 5482, the user timeout | 4 (2 of them expect `FAIL`) |
| RFC 9438, CUBIC | 4 (1 of them expects `FAIL`) |
| RFC 3168, ECN | 2 |
| RFC 2385, the MD5 signature option | 2 |
| RFC 2883, D-SACK; RFC 4821, the path MTU search; RFC 4987, SYN cookies; RFC 5961, the checks of a segment | 1 each |

Other scripts exercise interfaces of Linux that no standard defines: `MSG_ZEROCOPY` (21), `MSG_EOR` (8), blocking socket calls (8), `SO_TIMESTAMPING` (7), `TCP_INFO` (6), the checks of system call arguments (6), `TCP_INQ` (4), `TCP_USER_TIMEOUT` (4), `TCP_MAXSEG` (3), `TCP_NOTSENT_LOWAT`, `sendfile` and `splice` (2 each), `epoll`, `ioctl` and the cwnd moderation of Linux (1 each).

**What the run does not show.** A match is not a pass of a catalog statement, and the ledger and the verdicts of the matrix do not change. Where the defaults of `LinuxTcp` depart from the RFCs by design ([conformance.md finding 9](conformance.md#9-where-linux-departs-from-an-rfc-the-default-follows-the-rfc)), for example the initial window of ten segments in the scripts of slow start, a match shows the behavior of Linux, not that of the RFC; such a departure is known, and it is not a new finding. This record did not compare each script with the statements of the catalogs, so it reports no other finding.

## Gap 5 (pass 4): the first round-trip measurement is smoothed — defect

RFC 6298 section 2.2 gives the first measurement a case of its own: the smoothed value becomes R and the variance becomes R/2. The model has no such case. `TcpBaseAlg::receivedDataAck` applies the smoothing formula of section 2.3 to every measurement, the first one included ([TcpAlgorithmBase.cc:576-591](../../../../../src/inet/transportlayer/tcp/flavours/TcpAlgorithmBase.cc#L576-L591), the code after the repair). The run states it in one line:

    Measured RTT=400.05104ms, updated SRTT=50.00638ms, new RTO=2875.0319ms

| | RFC 6298 | the model |
| --- | --- | --- |
| smoothed value | 400.05 ms | 50.01 ms, one eighth |
| variance | 200.03 ms | 706.26 ms |
| timeout | 1200.15 ms | 2875.03 ms, 2.4 times |

The class is **defect**. Code exists for the behaviour, it runs on the first acknowledgment, and it produces the wrong number. A timeout that long delays every recovery from a lost segment, and it is the timeout a connection starts with. The effect reproduces on a second link: `Rfc6298KarnsRule.test` measures 0.108 ms and smooths it to 0.0135 ms, the same factor of eight.

## The model publishes a control variable only where it recomputes it

**Five of the ten checks could not read the value the standard names**, and each had to measure the rule on the wire instead. This is one property of the model, and it shapes the whole pass.

| The value the check names | Where the model publishes it | What the check does instead |
| --- | --- | --- |
| the timeout before any measurement | nowhere; `rto` comes only from `receivedDataAck` | measures the interval to the first retransmission |
| the timeout after each expiry | nowhere; the expiry path emits nothing | measures the intervals between retransmissions |
| the timeout at the first data send | nowhere, for the same reason | measures the interval again |
| the initial congestion window | nowhere; `established` sets it and emits nothing | adds up the data in flight before the first acknowledgment |
| the threshold before any loss | nowhere; `TcpReno` publishes it when it changes it | not read; the observation is dropped |

Two consequences worth recording. The backoff check can verify only one of its two expected observations, and the note in `checks.md` is right that this matters: a model that doubled the variable but not the interval would pass one and fail the other, and this pass cannot tell those two apart. And the threshold observation of the slow-start check is dropped entirely.

A second trap follows from the same property. Where the model *does* publish a value, it may publish it **earlier than the episode the check is about**. The threshold is published before any loss, and a step that bound the first publication ended the run at t=0.2007 with a verdict, before the relay had removed anything. The timeout-response check selects the publication at least one second after the loss begins for that reason.

## Three checks of this pass were vacuous before they were repaired

Each passed while establishing nothing, and each was found by disbelieving a pass rather than by a failure. They are recorded because the shape repeats.

1. **A predicate on a scalar signal was refused by the framework.** The slow-start guard held over nothing and passed with a bound of one segment, a quarter of a segment, and zero. The third of those made no sense, which is what exposed it. Fixed in the framework.
2. **The initial window was read from the first publication of `cwnd`**, which is one acknowledgment later than the initial window. It passed, because the grown value is still inside the bound. It now measures the data in flight before the first acknowledgment.
3. **The window after a lost SYN was read the same way**, and that one nearly recorded a defect that does not exist: the model implements the rule and quotes RFC 5681 in the code.

## Sharpening candidates for the next pass

- **Observation 4 of the fast-retransmit check** — the window returns to the threshold when the repair is acknowledged — is not asserted. Telling that publication apart from the growth that follows needs the acknowledgment of new data as an anchor.
- **The estimator side of the backoff check**, once the expiry publishes the timeout.
- **The threshold before any loss**, once the model publishes it where it is set.
- The **round-trip variance** is not read anywhere, although `rttvar` is published beside `srtt`. Gap 5 quotes it, and a check of its own would bound it.

## The class of each failure, reviewed 2026-09-11

The guide's step 7 named one class, `model gap`, when passes 2 and 3 ran. Master replaced it with five classes and a decidable line: *does code exist for this specific behaviour?* If it exists the model claims the behaviour and the failure is a **defect**, which declares nothing; only an absent behaviour is an **unimplemented feature**, which may declare `%# expected-result: FAIL`.

All four failures of this suite were reviewed against that line, each against the code and not against the earlier wording.

| Test | Does code exist for the behaviour? | Class | Declared? |
| --- | --- | --- | --- |
| Rfc9293Push | no call to `setPshBit` exists anywhere in the sender; two comments say so | unimplemented feature; closed 2026-10-14 | yes; removed 2026-10-14 |
| Rfc9293SourceQuench | `Icmp::processIcmpMessage` still has no branch for type 4, but its default discards instead of throwing, which meets the rule | closed 2026-09-14 | no, removed |
| Rfc9293ChecksumDefault | **Defect, and the repair is blocked.** `TcpChecksumInsertionHook::computeChecksum` computes the value and the `computed` mode runs it; what is wrong is the default, which `Tcp.ned` sets to `declared`. Changing it to `computed` was tried on 2026-09-15 and reverted. With a computed default, `examples/inet/nclients -c lwip__inet` stops with "Received 1 B of TCP data without a GenericAppMsgReq region tag at its front". The protocol suite cannot see this, because none of its scenarios uses a region-tagged payload; the fingerprint run found it. Corrected on 2026-10-16: the cause is not the serialization, see below. **TCP keeps the declared default by the decision of the owner, and two defects of other modules stop a computed default.** The UDP default waits for a limitation of its own | **defect with a blocked repair** | **yes, since 2026-09-17** |
| Rfc9293ShrunkWindowNoNewData | **Reclassified 2026-09-15: a test error, and the model obeys the rule.** `sendData` refuses the send and says so: `Effective window is zero (advertised window 100, congestion window 2144), cannot send.` Host A had 2320 octets buffered, so it wanted to send and did not. 59 microseconds later host B advertises 7504 again, because only the relay shrank anything, and host A then sends lawfully. The old watch ran for 0.3 seconds and caught that lawful send. It runs for 50 microseconds now, inside the interval the shrunk edge stands | **test error** | **no, removed** |

Two declarations were removed. The suite result moves from 18 TOTAL, 14 PASS, 4 FAIL (expected) — reported as PASS — to 18 TOTAL, 14 PASS, 2 FAIL (expected), 2 FAIL (unexpected), reported as **FAIL**. That is the intended signal: a defect keeps the suite red until somebody fixes it.

Since 2026-09-17 the checksum default is declared again, as a **defect with a blocked repair**. The guide now allows a declaration when a named limitation keeps the repair away for a long time, and the description of the test names it: a serialized TCP payload loses its region tags. The suite is 27 tests: 25 PASS and 2 FAIL (expected), and it reports PASS.

Since 2026-10-14 gap 1 is closed, and its declaration is removed. The suite is 27 tests: 26 PASS and 1 FAIL (expected).

Since 2026-10-16 the declaration of gap 2 names other causes. Item T9 of the open TCP items found that the payload keeps its region tags, and that two defects of other modules stop a computed default (the record of T9 in commit `742884a572`):

- The receive cache of `TcpLwip` (`TcpLwipReceiveQueue::notifyAboutIncomingSegmentProcessing()`) reads the payload with `Packet::peekAt()`, which counts from the start of the packet. With a declared checksum, `TcpLwip` trims the headers below TCP first; with a computed one, it does not, and the cache takes one byte of the IPv4 header.
- `TcpChecksumInsertionHook` expects the TCP header right after the network header of the `NetworkProtocolInd` tag. Mobile IPv6 adds extension headers in its local-out hook and does not update that tag, so `MIPv6_tcp_handover` with a computed checksum stops in the hook.

The owner dropped T9 on 2026-10-16 and keeps the declared default (D-165). The test stays FAIL (expected), and its description names the decision and the two defects.

Neither defect is a large repair. One is a NED default; the other is one comparison in the send decision.

## Gap 1 (pass 2): the PSH bit is never set — unimplemented feature

**Closed on 2026-10-14.** The parameter `pushLastBufferedSegment` sets PSH on the segment after which no queued data waits, and it is on by default (`tcp: change: PSH in the last buffered segment by default`). `Rfc9293Push.test` passes and declares nothing. The text below is the finding of pass 2.

`Rfc9293Push.test` keeps the faithful assertion — the last segment of a 5000-octet send carries PSH — and declares `%# expected-result: FAIL`. The class is **unimplemented feature**: the model does not claim this behaviour, on three pieces of evidence:

1. No call sets the PSH bit anywhere in the TCP sender. `sendSegment` carries the comment at the place where the bit would be set: [TcpConnectionUtil.cc:1008](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L1008) `// TODO when to set PSH bit?`
2. The SEND processing says why: [TcpConnectionEventProc.cc:116-117](../../../../../src/inet/transportlayer/tcp/TcpConnectionEventProc.cc#L116-L117) `// FIXME how to support PUSH? One option is to treat each SEND as a unit of data, and set PSH at SEND boundaries`.
3. The receiver ignores the bit: [TcpConnectionRcvSegment.cc:542](../../../../../src/inet/transportlayer/tcp/TcpConnectionRcvSegment.cc#L542) `// FIXME observe PSH bit`, and [TcpConnectionRcvSegment.cc:987](../../../../../src/inet/transportlayer/tcp/TcpConnectionRcvSegment.cc#L987) `// Now: URG and PSH we don't support yet`.

The requirement is conditional, and the condition holds: RFC 9293 MUST-61 binds a sender whose SEND call offers no PUSH flag, and the model's socket interface offers none. Within a simulation the practical effect is small — the model's receiver delivers data to the application at once in its default mode — but on the wire the model's segments differ from every real stack's, which set PSH on the last segment of a write. Traces compared with real captures, and peers that hold data until PSH, see the difference.

The gap was found by this pass, not by the earlier one: the earlier pass had no push statement in the catalog. The test is separate from the data transfer test on purpose, so that its failure cannot block the decisive observation that the whole stream was acknowledged.

## Gap 2 (pass 3): the default mode writes no checksum at all — defect

**Statement.** [RFC9293-CKSUM-1](../../standard/rfc9293/catalog.md#rfc9293-cksum-1), must: "The TCP checksum is never optional. The sender MUST generate it (MUST-2)", §3.1, `rfc9293.txt:458-459`.

**What the model does.** The `checksumMode` parameter of the ~Tcp module is `default("declared")` (src/inet/transportlayer/tcp/Tcp.ned:189). In that mode nothing is computed: the field stays at zero and the header carries a flag that asserts it correct, which `Tcp::checkChecksum` then believes without arithmetic (the CHECKSUM_DECLARED_CORRECT branch, Tcp.cc:527). The computed mode is correct; the run shows host B, told to compute, carrying exactly the value the document defines.

**Why it is a gap.** RFC 9293 is stronger here than RFC 768 is for UDP: the checksum is never optional, and there is no mode in which a TCP sender may leave it out. A host in its default state sends segments that carry no checksum at all, and two INET hosts accept each other only because both consult the same flag.

**The same finding as UDP gap 1, one layer up.** The UDP pass found the same default and the same placeholder mechanism (`doc/project/evidence/model/udp/results.md`). The two share a cause and a correction. They differ in what the standard allows: RFC 768 lets a sender generate no checksum, so the UDP default is a wrong value where a lawful one exists; RFC 9293 allows no such thing.

**Not a defect of the module.** Five checks of this pass depend on the computed mode and pass. What fails is the choice of default.

## Gap 3 (pass 3): new data goes past a shrunk window edge — defect

**Statement.** [RFC9293-WND-5](../../standard/rfc9293/catalog.md#rfc9293-wnd-5), should not: "If this happens, the sender SHOULD NOT send new data (SHLD-15)", §3.8.6, `rfc9293.txt:2155`, where "this" is the usable window becoming negative.

**What the model does.** The relay shrank one acknowledgment to a window of 100 octets. The run shows host A's TCP reading it at t=0.2002652, with the acknowledgment number at 26608, so the new right edge is 26708. At t=0.20032412, in the same moment of simulated time, host A's TCP decided to send a further full segment at sequence 27144, which carries 536 octets and starts 436 octets beyond the edge. The segment is new data, not a retransmission: the sequence number had never been sent before.

**Why it is a gap.** The rule exists because a peer that shrank its window has less room than it promised, and data sent past the new edge is data the peer must throw away. The level 2 check of the same feature passed: with a window that only grows, the model stays inside it (RFC9293-WND-2). The rule breaks only when the edge moves backward, which is what level 3 is for.

**What holds.** The stronger half of the same event holds. `Rfc9293ShrunkWindow.test` shows the connection surviving the negative window and finishing the whole transfer, which is MUST-34. The two are separate tests on purpose, so that this failure cannot hide that verdict.

## Gap 4 (pass 3): a Source Quench stops the run — unimplemented feature

**Statement.** [RFC9293-ICMP-2](../../standard/rfc9293/catalog.md#rfc9293-icmp-2), must: "TCP implementations MUST silently discard any received ICMP Source Quench messages (MUST-55)", §3.9.2.2, `rfc9293.txt:2841-2842`.

**What the model does.** It stops the simulation:

```
Error: Unknown ICMP type 4 -- in module (inet::Icmp) Rfc9293IcmpNet.host1.ipv4.icmp
(id=48), at t=0.20016446s, event #84
```

`Icmp::processIcmpMessage` switches on the type, and the default branch throws `cRuntimeError("Unknown ICMP type %d")` (src/inet/networklayer/ipv4/Icmp.cc). TCP is never offered the message, so the rule that TCP must discard it is never reached.

**Why it is a gap.** A silent discard leaves the host running. RFC 9293 keeps this rule because a host still meets the message: RFC 6633 deprecated Source Quench for routers, and what is deprecated for senders is still received by hosts.

**The same branch as an IPv4 finding.** `tests/protocol/ipv4/Rfc1122UnknownIcmpType.test` reaches the same default branch with type 42, a number that names no message ([ipv4/results.md](../ipv4/results.md)). This test shows that the branch is reached by a type the standards still describe, and that a TCP connection is what meets it. One correction in the ICMP module closes both.

## Deviations between the English observations and the test steps

1. **Connection establishment, observation 1 observed at the link layer.** The TCP checksum is inserted by an IPv4 post-routing hook that the TCP module creates when the computed mode is on ([Tcp.cc:67-69](../../../../../src/inet/transportlayer/tcp/Tcp.cc#L67-L69), `TcpChecksumInsertionHook`). At the TCP module's own send signal the field is still zero, so the nonzero-checksum condition is checked at `host1.eth[0].mac`, which is the wire.
2. **Flow control, a sink instead of an echo on host B.** An echo application piggybacks its acknowledgment on the data it returns, which removes the acknowledgment delay the check relies on. The close time is also dropped from that scenario: 3000 octets through a 300-octet window at one round per 0.2 s take about 2 s.
3. **Data transfer, observation 3 requires data.** A pure acknowledgment that host A sends while its next sequence number is `ISS_A + 537` carries that same number; the step now requires a positive data length. Found in review, after the agent-written step had passed on a segment that did carry data.
4. **A tester defect, worked around.** Capturing a unit-bearing field (the SYN's header length) makes the tester's capture substitution fail on the next step with `Attempt to use the value '24B' as a dimensionless number` (`tests/protocol/lib/EventPattern.cc`, `formatCaptureValue`, which substitutes every stored capture through `intValue()`). The test captures the number without its unit through a lambda. Tooling, not model.

## Scenario settings beyond the templates

- `**.checksumMode = "computed"` in every test. The default, [Tcp.ned:189](../../../../../src/inet/transportlayer/tcp/Tcp.ned#L189), is `"declared"`, a placeholder; the TCP checksum is never optional (MUST-2), so the computed mode is the conformant configuration.
- `*.host2.tcp.advertisedWindow = 300` and `*.host2.tcp.delayedAcksEnabled = true` in the flow-control test. The delay is the parameter `delayedAckTimeout`, [Tcp.ned:267](../../../../../src/inet/transportlayer/tcp/Tcp.ned#L267), 0.2 s by default, which is what the check document assumes and within MUST-40.
- `sendBytes = 5000B` (data transfer, push) and `3000B` (flow control); `connectPort = 7000` with `*.host2.numApps = 0` (reset).

Pass 3 adds:

- `sim-time-limit = 10s` wherever a check takes a segment away. Host A has one segment in flight when the loss happens, so no duplicate acknowledgment can arrive and the recovery waits for the retransmission timeout, which starts at three seconds.
- `*.host2.app[0].echoFactor = 0`, so that the reverse direction carries acknowledgments only. An observation about a segment from host B cannot then be confused with an echo.
- `**.tcp.checksumMode = "computed"` and not `**.checksumMode`, in the two ICMP tests. The wildcard would also reach the ICMP module, whose declared mode is what lets the relay change the type of a message without computing an ICMP checksum, exactly as tests/protocol/ipv4/Rfc1122UnknownIcmpType.test relies on.
- `Rfc9293ChecksumDefault.test` says nothing about host A's checksum on purpose. The absence of the setting is the subject of that check.

## Failure history during authoring

Pass 2: three test errors and one gap. The errors: a `notBefore` copied from the old template onto a step whose anchor had moved; the echo application in the flow-control scenario (deviation 2); the unit-bearing capture (deviation 4). The review added the data condition of deviation 3. The gap is the PSH bit, above.

Pass 3: five test errors and three gaps. Nothing was reverted, skipped or softened; each error was a fault of the test, and each was corrected before the verdict was recorded.

1. **The header chunk lives in its own namespace.** `TcpHeader` is `inet::tcp::TcpHeader`, and the helper file must say so or nothing compiles.
2. **A relay that removed the payload removed the trailer with it.** The first `makeReset` read the length of what followed the header, which is the payload **and** the link-layer trailer. The frame that left was malformed and the receiver never saw it. The helper now takes the payload length from the IPv4 total length.
3. **A predicate that reads IPv4 fields cannot run at a transport module.** The empty acknowledgment of the out-of-window check is recognised by its IPv4 total length, and TCP hands a segment down before that header exists. The observation moved to the interface.
4. **An interface reports a send when the transmission ends.** The first version of the shrunk-window check watched host A's interface, which can report a send that was decided before the acknowledgment arrived. The watch moved to the module, where the decision is made. The verdict did not change, but it would not have been trustworthy.
5. **A relay holds one rule at a time.** The Source Quench check needs two changes, one on the way out and one on the way back. The first version put both on one relay, and the second silently replaced the first: the tap reported `mutated 0`. Two relays in series fixed it. The lesson was already in the IPv6 notes; it is here now for TCP as well.

The last error is worth naming twice. Before it was found, the check failed for the reason a reader would have expected — and the description in the test file already claimed the crash that the corrected version does produce. The claim was removed until the run showed it. A test that fails for the wrong reason teaches nothing, and a description written ahead of the evidence is how that goes unnoticed.

## Model analysis — where INET implements the checked behavior

- **Handshake, sequence space (EST-1, EST-2, SEQ-1):** `sendSyn` [TcpConnectionUtil.cc:736](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L736) and `sendSynAck` [:779](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L779), each with `snd_nxt = iss + 1` ([:751](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L751), [:790](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L790)). The initial sequence number: [:673-676](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L673-L676).
- **MSS option, header length (OPT-1, HDR-1):** written into the SYN in `writeHeaderOptions`, [TcpConnectionUtil.cc:1454-1465](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L1454-L1465); 20 octets plus the 4-octet option, 24 observed.
- **Segmentation (SEG-1):** `sendSegment` caps a segment at `snd_mss` less the option length, [TcpConnectionUtil.cc:983-984](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L983-L984); `sendData` sends whole segments of the effective MSS, [:1094-1106](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L1094-L1106). Observed: 536 octets in the first segment.
- **Cumulative acknowledgment, delivery (ACK-1, DATA-1):** `rcv_nxt` advances over in-order data at [TcpConnectionRcvSegment.cc:500](../../../../../src/inet/transportlayer/tcp/TcpConnectionRcvSegment.cc#L500); delivery in `sendAvailableDataToApp`, [TcpConnectionUtil.cc:566-577](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L566-L577). Observed: the acknowledgment of `ISS_A + 5001`.
- **Send window (WND-2):** `sendData` bounds new data by `min(snd_wnd, cwnd) - (snd_nxt - snd_una)`, [TcpConnectionUtil.cc:1075-1078](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L1075-L1078). The initial congestion window is one segment, [TcpAlgorithmBase.cc:119-120](../../../../../src/inet/transportlayer/tcp/flavours/TcpAlgorithmBase.cc#L119-L120), so a 300-octet receiver window is the binding limit. Observed: a 300-octet first segment, silence for 0.15 s, the delayed acknowledgment at 0.2 s, and the resume at the edge.
- **Advertised window (WND-1):** `updateRcvWnd`, [TcpConnectionUtil.cc:1650-1670](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L1650-L1670), with the receiver's silly-window floor at [:1660-1661](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L1660-L1661). Observed: 300 in the SYN-ACK and in the acknowledgments.
- **Checksum (CKSUM-1):** inserted by the hook, [Tcp.cc:43-77](../../../../../src/inet/transportlayer/tcp/Tcp.cc#L43-L77); verified in `checkChecksum`, [Tcp.cc:496-535](../../../../../src/inet/transportlayer/tcp/Tcp.cc#L496-L535), and a failing segment is dropped with reason `INCORRECTLY_RECEIVED`, [Tcp.cc:149-156](../../../../../src/inet/transportlayer/tcp/Tcp.cc#L149-L156). That drop is RFC9293-CKSUM-2, level 3.
- **Close (FIN-1, FIN-2):** `sendFin` [TcpConnectionUtil.cc:922](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L922); `rcv_nxt` over the FIN, [TcpConnectionRcvSegment.cc:627-630](../../../../../src/inet/transportlayer/tcp/TcpConnectionRcvSegment.cc#L627-L630).
- **Reset for a closed port (RST-1):** `segmentArrivalWhileClosed`, [TcpConnectionRcvSegment.cc:62-75](../../../../../src/inet/transportlayer/tcp/TcpConnectionRcvSegment.cc#L62-L75): `ackNo = SEG.SEQ + data length + SynFinLen`, sequence number 0, through `sendRstAck` ([TcpConnectionUtil.cc:853-863](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L853-L863)). The connecting side goes to CLOSED on the reset, with no retry, [TcpConnectionBase.cc:457-464](../../../../../src/inet/transportlayer/tcp/TcpConnectionBase.cc#L457-L464).

## Model observations the checks did not claim

1. **The default mode has no flow control, by the model's own words.** [Tcp.ned:53-58](../../../../../src/inet/transportlayer/tcp/Tcp.ned#L53-L58): in the default "autoread" mode "the advertised window never decreases so there is effectively no flow control". The flow-control check passed because it tests the sender's respect for a fixed small window, which does not need the window to close. A closing window needs the "explicit-read" mode that the same note names, and which the applications expose as `autoRead = false`; that is the next flow-control check.
2. **The checksum is not computed by default**, and when it is, it is computed one layer down. Observers at the TCP module see a zero field (deviation 1).
3. **The initial sequence number** meets MUST-8 — the 4-microsecond clock — and not SHLD-1, the pseudorandom term ([TcpConnectionUtil.cc:676](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L676)), as the earlier pass recorded.
4. **Urgent data is unsupported too.** The same comments that mark PSH unsupported name URG with it. RFC 9293 requires the mechanism (MUST-30 to MUST-32) and discourages its use; it is a level 5 candidate for a `defect`.

## Feature support

The verdicts above feed the support column and the achieved level of the coverage ledger, [`coverage.md`](coverage.md#feature-support).

## Sharpening candidates for the next pass

The level 3 line below is done: every item of it is a test of pass 3.

- **Level 3, done:** RFC9293-CKSUM-2 with a corrupted segment; a reset on a live connection and the acceptance rules for a received reset; a shrunk window; ICMP errors on a connection.
- **Level 3, what remains:** RFC9293-ICMP-4, whether a hard error should abort a synchronized connection. The model decides deliberately not to, citing blind reset attacks (TcpConnectionUtil.cc), and RFC 9293 itself notes that implementations differ. A check would record a `declined` with a reason, which is worth having but settles nothing.
- **Level 4:** RFC 5681 and RFC 6298 in the in-scope set; the zero-window probe (RFC9293-ZWP-1) and the acknowledgment delay bound (RFC9293-ACKD-1) as statistical checks; the TIME-WAIT duration.
- **A closing window** with `autoRead = false` on the receiver, for the half of flow control the default mode cannot show.
- **RFC9293-SEQ-2** on the existing mockup; **RFC9293-ISS-2** split into its two halves.
- **The tester:** substitute captures without forcing them to dimensionless integers, so a unit-bearing field can be captured directly.
