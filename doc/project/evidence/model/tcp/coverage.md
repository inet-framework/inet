# TCP — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc9293/catalog.md](../../standard/rfc9293/catalog.md), [rfc6298/catalog.md](../../standard/rfc6298/catalog.md), [rfc5681/catalog.md](../../standard/rfc5681/catalog.md), [features.md](../../protocol/tcp/features.md), [results.md](results.md)

The single place that holds the changing state of the TCP workflow. Every other artifact
of this protocol states what a standard says and never changes again unless the standard
changes. This one changes on every pass.

**No step edits an artifact of an earlier step. Steps 5, 6 and 7 record their outcome here.**

State of the ledger, from this run:

- Date: 2026-09-23 18:26 +0200
- INET: branch `topic/standards-tests-wave0`, commit `28536bd0a5` (on `master`), tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`
- OMNeT++: 6.4.0, commit `cf58891643`
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-34-generic x86_64
- Command: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/tcp$'`
- Target level: 4

The run: 27 tests, 25 PASS, 2 FAIL (expected), 0 FAIL (unexpected).

## Statement coverage

`Status` is the state of the workflow, not of the standard: `selected` (a check targets it),
`covered` (a selected check establishes it as a side effect), `candidate` (practical, not yet
chosen), `owed` (the model claims it and no check reaches it yet: the coverage debt), `later`
(needs a toolset beyond the current level, or another test category).

| Catalog ID | Status | Check section | Test file | Verdict |
| --- | --- | --- | --- | --- |
| [RFC9293-EST-1](../../standard/rfc9293/catalog.md#rfc9293-est-1) | selected | [connection-establishment](../../protocol/tcp/checks/establishment.md#connection-establishment) | Rfc9293ConnectionEstablishment.test | PASS |
| [RFC9293-EST-2](../../standard/rfc9293/catalog.md#rfc9293-est-2) | selected | [connection-establishment](../../protocol/tcp/checks/establishment.md#connection-establishment) | Rfc9293ConnectionEstablishment.test | PASS |
| [RFC9293-ISS-1](../../standard/rfc9293/catalog.md#rfc9293-iss-1) | covered | [connection-establishment](../../protocol/tcp/checks/establishment.md#connection-establishment) | Rfc9293ConnectionEstablishment.test | PASS |
| [RFC9293-ISS-2](../../standard/rfc9293/catalog.md#rfc9293-iss-2) | candidate (unit and module) | — | — | — |
| [RFC9293-SEQ-1](../../standard/rfc9293/catalog.md#rfc9293-seq-1) | selected | [connection-establishment](../../protocol/tcp/checks/establishment.md#connection-establishment) | Rfc9293ConnectionEstablishment.test | PASS |
| [RFC9293-SEQ-2](../../standard/rfc9293/catalog.md#rfc9293-seq-2) | candidate | — | — | — |
| [RFC9293-ACK-1](../../standard/rfc9293/catalog.md#rfc9293-ack-1) | selected | [data-transfer](../../protocol/tcp/checks/data-transfer.md#data-transfer) | Rfc9293DataTransfer.test | PASS |
| [RFC9293-ACK-2](../../standard/rfc9293/catalog.md#rfc9293-ack-2) | covered | [data-transfer](../../protocol/tcp/checks/data-transfer.md#data-transfer) | Rfc9293DataTransfer.test | PASS |
| [RFC9293-FIN-1](../../standard/rfc9293/catalog.md#rfc9293-fin-1) | selected | [connection-termination](../../protocol/tcp/checks/termination.md#connection-termination) | Rfc9293ConnectionTermination.test | PASS |
| [RFC9293-FIN-2](../../standard/rfc9293/catalog.md#rfc9293-fin-2) | selected | [connection-termination](../../protocol/tcp/checks/termination.md#connection-termination) | Rfc9293ConnectionTermination.test | PASS |
| [RFC9293-DATA-1](../../standard/rfc9293/catalog.md#rfc9293-data-1) | selected | [data-transfer](../../protocol/tcp/checks/data-transfer.md#data-transfer) | Rfc9293DataTransfer.test | PASS |
| [RFC9293-SEG-1](../../standard/rfc9293/catalog.md#rfc9293-seg-1) | selected | [data-transfer](../../protocol/tcp/checks/data-transfer.md#data-transfer) | Rfc9293DataTransfer.test | PASS |
| [RFC9293-OPT-1](../../standard/rfc9293/catalog.md#rfc9293-opt-1) | covered | [connection-establishment](../../protocol/tcp/checks/establishment.md#connection-establishment) | Rfc9293ConnectionEstablishment.test | PASS; the SYN header is 24 octets, the option is present |
| [RFC9293-PSH-1](../../standard/rfc9293/catalog.md#rfc9293-psh-1) | selected | [push-on-the-last-segment](../../protocol/tcp/checks/data-transfer.md#push-on-the-last-segment) | Rfc9293Push.test | **FAIL (expected)** — unimplemented feature, gap 1: the model never sets PSH |
| [RFC9293-WND-1](../../standard/rfc9293/catalog.md#rfc9293-wnd-1) | selected | [flow-control](../../protocol/tcp/checks/flow-control.md#flow-control) | Rfc9293FlowControl.test | PASS |
| [RFC9293-WND-2](../../standard/rfc9293/catalog.md#rfc9293-wnd-2) | selected | [flow-control](../../protocol/tcp/checks/flow-control.md#flow-control) | Rfc9293FlowControl.test | PASS |
| [RFC9293-ZWP-1](../../standard/rfc9293/catalog.md#rfc9293-zwp-1) | later (level 4: a timer with a tolerance) | — | — | — |
| [RFC9293-ACKD-1](../../standard/rfc9293/catalog.md#rfc9293-ackd-1) | covered | [flow-control](../../protocol/tcp/checks/flow-control.md#flow-control) | Rfc9293FlowControl.test | PASS; one delayed acknowledgment observed at 0.2 s, under the bound; the bound as a distribution is level 4 |
| [RFC9293-CKSUM-1](../../standard/rfc9293/catalog.md#rfc9293-cksum-1) | selected | [connection-establishment](../../protocol/tcp/checks/establishment.md#connection-establishment), [checksum-by-default](../../protocol/tcp/checks/checksum.md#checksum-by-default) | Rfc9293ConnectionEstablishment.test, Rfc9293ChecksumDefault.test | PASS for the presence of the field; **FAIL (expected)** for its value in the default state: gap 2, a defect whose repair is blocked, declared by `4548adeb04` |
| [RFC9293-HDR-1](../../standard/rfc9293/catalog.md#rfc9293-hdr-1) | selected | [connection-establishment](../../protocol/tcp/checks/establishment.md#connection-establishment) | Rfc9293ConnectionEstablishment.test | PASS |
| [RFC9293-RST-1](../../standard/rfc9293/catalog.md#rfc9293-rst-1) | selected | [connection-reset](../../protocol/tcp/checks/reset.md#connection-reset) | Rfc9293Reset.test | PASS |
| [RFC9293-CKSUM-2](../../standard/rfc9293/catalog.md#rfc9293-cksum-2) | selected | [checksum-discard](../../protocol/tcp/checks/checksum.md#checksum-discard) | Rfc9293ChecksumDiscard.test | PASS |
| [RFC9293-SEGA-1](../../standard/rfc9293/catalog.md#rfc9293-sega-1) | selected | [out-of-window-segment](../../protocol/tcp/checks/segment-acceptance.md#out-of-window-segment) | Rfc9293OutOfWindowSegment.test | PASS |
| [RFC9293-SEGA-2](../../standard/rfc9293/catalog.md#rfc9293-sega-2) | selected | [out-of-window-segment](../../protocol/tcp/checks/segment-acceptance.md#out-of-window-segment) | Rfc9293OutOfWindowSegment.test | PASS |
| [RFC9293-RST-2](../../standard/rfc9293/catalog.md#rfc9293-rst-2) | selected | [out-of-window-segment](../../protocol/tcp/checks/segment-acceptance.md#out-of-window-segment) | Rfc9293OutOfWindowSegment.test | PASS |
| [RFC9293-RST-3](../../standard/rfc9293/catalog.md#rfc9293-rst-3) | selected | [no-reset-for-a-reset](../../protocol/tcp/checks/reset.md#no-reset-for-a-reset) | Rfc9293NoResetForReset.test | PASS |
| [RFC9293-RSTP-1](../../standard/rfc9293/catalog.md#rfc9293-rstp-1) | selected | [blind-reset](../../protocol/tcp/checks/reset.md#blind-reset) | Rfc9293BlindReset.test | PASS |
| [RFC9293-RSTP-2](../../standard/rfc9293/catalog.md#rfc9293-rstp-2) | selected | [valid-reset](../../protocol/tcp/checks/reset.md#valid-reset) | Rfc9293ValidReset.test | PASS |
| [RFC9293-WND-3](../../standard/rfc9293/catalog.md#rfc9293-wnd-3) | selected | [no-window-shrink](../../protocol/tcp/checks/window.md#no-window-shrink) | Rfc9293NoWindowShrink.test | PASS; for a transfer whose receiving program consumes at once |
| [RFC9293-WND-4](../../standard/rfc9293/catalog.md#rfc9293-wnd-4) | selected | [shrunk-window](../../protocol/tcp/checks/window.md#shrunk-window) | Rfc9293ShrunkWindow.test | PASS |
| [RFC9293-WND-5](../../standard/rfc9293/catalog.md#rfc9293-wnd-5) | selected | [no-new-data-past-a-shrunk-edge](../../protocol/tcp/checks/window.md#no-new-data-past-a-shrunk-edge) | Rfc9293ShrunkWindowNoNewData.test | PASS. Gap 3 was a test error, not a defect; see the results |
| [RFC9293-ICMP-1](../../standard/rfc9293/catalog.md#rfc9293-icmp-1) | covered | [soft-icmp-error](../../protocol/tcp/checks/icmp.md#soft-icmp-error) | Rfc9293SoftIcmpError.test | PASS; the report reached the connection, which is what the check needed of it |
| [RFC9293-ICMP-2](../../standard/rfc9293/catalog.md#rfc9293-icmp-2) | selected | [source-quench](../../protocol/tcp/checks/icmp.md#source-quench) | Rfc9293SourceQuench.test | PASS. Gap 4 was closed by the ICMP repair `40c9f04e21` |
| [RFC9293-ICMP-3](../../standard/rfc9293/catalog.md#rfc9293-icmp-3) | selected | [soft-icmp-error](../../protocol/tcp/checks/icmp.md#soft-icmp-error) | Rfc9293SoftIcmpError.test | PASS |
| [RFC9293-ICMP-4](../../standard/rfc9293/catalog.md#rfc9293-icmp-4) | candidate | — | — | — |
| [RFC6298-INIT-1](../../standard/rfc6298/catalog.md#rfc6298-init-1) | selected | [initial-timeout](../../protocol/tcp/checks/retransmission-timer.md#initial-timeout) | Rfc6298InitialTimeout.test | PASS; measured on the wire as the interval to the first retransmission, because the model publishes no timeout before a measurement |
| [RFC6298-FIRST-1](../../standard/rfc6298/catalog.md#rfc6298-first-1) | selected | [first-measurement](../../protocol/tcp/checks/retransmission-timer.md#first-measurement) | Rfc6298FirstMeasurement.test | PASS. Gap 5, a defect, was repaired by `f3064a83d0` |
| [RFC6298-UPD-1](../../standard/rfc6298/catalog.md#rfc6298-upd-1) | owed | — | — | — a check needs a second measurement in the same mockup; the model publishes the smoothed value and the variance at every measurement |
| [RFC6298-UPD-2](../../standard/rfc6298/catalog.md#rfc6298-upd-2) | owed | — | — | — the same run as UPD-1: the gains follow from two consecutive values |
| [RFC6298-RTO-1](../../standard/rfc6298/catalog.md#rfc6298-rto-1) | selected | [first-measurement](../../protocol/tcp/checks/retransmission-timer.md#first-measurement) | Rfc6298FirstMeasurement.test | PASS |
| [RFC6298-MIN-1](../../standard/rfc6298/catalog.md#rfc6298-min-1) | covered | [first-measurement](../../protocol/tcp/checks/retransmission-timer.md#first-measurement) | Rfc6298FirstMeasurement.test | PASS |
| [RFC6298-MAX-1](../../standard/rfc6298/catalog.md#rfc6298-max-1) | covered | [backoff-doubling](../../protocol/tcp/checks/retransmission-timer.md#backoff-doubling) | Rfc6298BackoffDoubling.test | PASS; the doubling stays under the upper bound of the model, 240 seconds, which the `may` permits |
| [RFC6298-KARN-1](../../standard/rfc6298/catalog.md#rfc6298-karn-1) | selected | [karns-rule](../../protocol/tcp/checks/retransmission-timer.md#karns-rule) | Rfc6298KarnsRule.test | PASS |
| [RFC6298-SAMP-1](../../standard/rfc6298/catalog.md#rfc6298-samp-1) | owed | — | — | — a check needs a transfer of several round trips and a count of the measurements in each |
| [RFC6298-GRAN-1](../../standard/rfc6298/catalog.md#rfc6298-gran-1) | owed | — | — | — a check needs a steady round trip above one second, so that the variance falls to zero and the floor of MIN-1 does not hide the term |
| [RFC6298-EARLY-1](../../standard/rfc6298/catalog.md#rfc6298-early-1) | selected | [backoff-doubling](../../protocol/tcp/checks/retransmission-timer.md#backoff-doubling) | Rfc6298BackoffDoubling.test | PASS |
| [RFC6298-TMR-1](../../standard/rfc6298/catalog.md#rfc6298-tmr-1) | owed | — | — | — the timer state is not published; a check reads it from the retransmission times |
| [RFC6298-TMR-2](../../standard/rfc6298/catalog.md#rfc6298-tmr-2) | owed | — | — | — as TMR-1 |
| [RFC6298-TMR-3](../../standard/rfc6298/catalog.md#rfc6298-tmr-3) | owed | — | — | — as TMR-1 |
| [RFC6298-EXP-1](../../standard/rfc6298/catalog.md#rfc6298-exp-1) | owed | — | — | — the backoff check sees the same segment again, but it does not assert that the segment is the earliest unacknowledged one |
| [RFC6298-BACK-1](../../standard/rfc6298/catalog.md#rfc6298-back-1) | selected | [backoff-doubling](../../protocol/tcp/checks/retransmission-timer.md#backoff-doubling) | Rfc6298BackoffDoubling.test | PASS for the intervals on the wire. The timeout itself is not read, because the expiry path publishes nothing; see the results |
| [RFC6298-SYN-1](../../standard/rfc6298/catalog.md#rfc6298-syn-1) | selected | [timeout-after-a-lost-syn](../../protocol/tcp/checks/retransmission-timer.md#timeout-after-a-lost-syn) | Rfc6298TimeoutAfterLostSyn.test | PASS |
| [RFC6298-COLL-1](../../standard/rfc6298/catalog.md#rfc6298-coll-1) | owed | — | — | — a check needs a measurement after a backoff |
| [RFC5681-USE-1](../../standard/rfc5681/catalog.md#rfc5681-use-1) | owed | — | — | — the slow start check shows slow start; no check shows congestion avoidance |
| [RFC5681-WIN-1](../../standard/rfc5681/catalog.md#rfc5681-win-1) | owed | — | — | — a check needs an advertised window below the congestion window |
| [RFC5681-IW-1](../../standard/rfc5681/catalog.md#rfc5681-iw-1) | selected | [initial-window](../../protocol/tcp/checks/congestion-control.md#initial-window) | Rfc5681InitialWindow.test | PASS; measured as the data in flight before the first acknowledgment |
| [RFC5681-IW-2](../../standard/rfc5681/catalog.md#rfc5681-iw-2) | selected | [initial-window](../../protocol/tcp/checks/congestion-control.md#initial-window) | Rfc5681InitialWindow.test | PASS |
| [RFC5681-IW-3](../../standard/rfc5681/catalog.md#rfc5681-iw-3) | selected | [window-after-a-lost-syn](../../protocol/tcp/checks/congestion-control.md#window-after-a-lost-syn) | Rfc5681WindowAfterLostSyn.test | PASS |
| [RFC5681-SSTH-1](../../standard/rfc5681/catalog.md#rfc5681-ssth-1) | owed | [slow-start-growth](../../protocol/tcp/checks/congestion-control.md#slow-start-growth) | Rfc5681SlowStartGrowth.test | — the test names it, and drops the observation: the model publishes the threshold only when a loss changes it |
| [RFC5681-SS-1](../../standard/rfc5681/catalog.md#rfc5681-ss-1) | covered | [slow-start-growth](../../protocol/tcp/checks/congestion-control.md#slow-start-growth) | Rfc5681SlowStartGrowth.test | PASS |
| [RFC5681-SS-2](../../standard/rfc5681/catalog.md#rfc5681-ss-2) | selected | [slow-start-growth](../../protocol/tcp/checks/congestion-control.md#slow-start-growth) | Rfc5681SlowStartGrowth.test | PASS |
| [RFC5681-CA-1](../../standard/rfc5681/catalog.md#rfc5681-ca-1) | owed | — | — | — a check needs a transfer that passes the threshold |
| [RFC5681-CA-2](../../standard/rfc5681/catalog.md#rfc5681-ca-2) | owed | — | — | — as CA-1 |
| [RFC5681-LOSS-1](../../standard/rfc5681/catalog.md#rfc5681-loss-1) | selected | [timeout-response](../../protocol/tcp/checks/congestion-control.md#timeout-response) | Rfc5681TimeoutResponse.test | PASS |
| [RFC5681-LOSS-2](../../standard/rfc5681/catalog.md#rfc5681-loss-2) | owed | — | — | — a check needs a second timeout of the same segment |
| [RFC5681-LOSS-3](../../standard/rfc5681/catalog.md#rfc5681-loss-3) | selected | [timeout-response](../../protocol/tcp/checks/congestion-control.md#timeout-response) | Rfc5681TimeoutResponse.test | PASS |
| [RFC5681-FR-1](../../standard/rfc5681/catalog.md#rfc5681-fr-1) | selected | [fast-retransmit-and-fast-recovery](../../protocol/tcp/checks/congestion-control.md#fast-retransmit-and-fast-recovery) | Rfc5681FastRetransmit.test | PASS |
| [RFC5681-FR-2](../../standard/rfc5681/catalog.md#rfc5681-fr-2) | selected | [fast-retransmit-and-fast-recovery](../../protocol/tcp/checks/congestion-control.md#fast-retransmit-and-fast-recovery) | Rfc5681FastRetransmit.test | PASS |
| [RFC5681-FR-3](../../standard/rfc5681/catalog.md#rfc5681-fr-3) | selected | [fast-retransmit-and-fast-recovery](../../protocol/tcp/checks/congestion-control.md#fast-retransmit-and-fast-recovery) | Rfc5681FastRetransmit.test | PASS |
| [RFC5681-FR-4](../../standard/rfc5681/catalog.md#rfc5681-fr-4) | owed | — | — | — a check needs a fourth and a fifth duplicate acknowledgment in the same episode |
| [RFC5681-FR-5](../../standard/rfc5681/catalog.md#rfc5681-fr-5) | owed | [fast-retransmit-and-fast-recovery](../../protocol/tcp/checks/congestion-control.md#fast-retransmit-and-fast-recovery) | Rfc5681FastRetransmit.test | — the test names it, and does not assert its observation 4: the window after the repair is acknowledged |
| [RFC5681-IDLE-1](../../standard/rfc5681/catalog.md#rfc5681-idle-1) | owed | — | — | — a check needs an idle period longer than one timeout; the model restarts an idle connection in its congestion control |
| [RFC5681-ACK-1](../../standard/rfc5681/catalog.md#rfc5681-ack-1) | later (statistical) | — | — | — the 500 ms bound needs a tolerance; the second-segment rule is RFC9293-ACKD-1 of the flow-control check |
| [RFC5681-ACK-2](../../standard/rfc5681/catalog.md#rfc5681-ack-2) | covered | [fast-retransmit-and-fast-recovery](../../protocol/tcp/checks/congestion-control.md#fast-retransmit-and-fast-recovery) | Rfc5681FastRetransmit.test | PASS |

74 entries in three catalogs.

- **RFC 9293, 35 entries.** 31 reached a test: 29 with a PASS, RFC9293-PSH-1 with a declared
  FAIL for an unimplemented feature, and RFC9293-CKSUM-1 with a PASS for the field and a
  declared FAIL for its value. 4 wait: RFC9293-ISS-2 and RFC9293-SEQ-2 for a unit or module
  test, RFC9293-ZWP-1 for a timer with a tolerance, and RFC9293-ICMP-4 for a decision about
  which behaviour the model intends.
- **RFC 6298, 18 entries.** 9 reached a test, all with a PASS. 9 are `owed`.
- **RFC 5681, 21 entries.** 11 reached a test, all with a PASS. 9 are `owed`, and two of the
  nine are named by a test that does not assert them (RFC5681-SSTH-1, RFC5681-FR-5). 1 waits
  for the statistical suite (RFC5681-ACK-1).

**The 18 `owed` rows are the debt of pass 4.** Pass 4 wrote one check for each of its ten
features, and a feature with several core statements got a check for some of them. Each
`owed` row says what its check needs. Eight of them are mandatory, see
[the achieved level](#achieved-level).

## Feature support

The rule of step 7: `supported` when every core check ran and passed, `partial` when a core
check passed and another core check failed as a model gap or did not run, `not supported`
when every core check that ran failed as a model gap, `untested` when no core check ran. A
failure of a `supporting` check does not lower the value; it is noted beside the feature.

| Feature | Core checks and their state | Support |
| --- | --- | --- |
| [TCP-F-ESTABLISH](../../protocol/tcp/features.md#tcp-f-establish) | RFC9293-EST-1 PASS, EST-2 PASS | **supported** |
| [TCP-F-SEQUENCE](../../protocol/tcp/features.md#tcp-f-sequence) | RFC9293-SEQ-1 PASS, FIN-1 PASS | **supported** |
| [TCP-F-ACKNOWLEDGE](../../protocol/tcp/features.md#tcp-f-acknowledge) | RFC9293-ACK-1 PASS | **supported** |
| [TCP-F-TERMINATE](../../protocol/tcp/features.md#tcp-f-terminate) | RFC9293-FIN-2 PASS | **supported** |
| [TCP-F-DATA-TRANSFER](../../protocol/tcp/features.md#tcp-f-data-transfer) | RFC9293-DATA-1 PASS, SEG-1 PASS; supporting RFC9293-PSH-1 **FAIL (expected)**, unimplemented | **supported**, with the PSH gap noted |
| [TCP-F-FLOW-CONTROL](../../protocol/tcp/features.md#tcp-f-flow-control) | RFC9293-WND-1 PASS, WND-2 PASS | **supported** |
| [TCP-F-CHECKSUM](../../protocol/tcp/features.md#tcp-f-checksum) | RFC9293-CKSUM-2 PASS; CKSUM-1 PASS for the field, **FAIL (expected)** for its value by default | **partial** |
| [TCP-F-HEADER](../../protocol/tcp/features.md#tcp-f-header) | RFC9293-HDR-1 PASS | **supported** |
| [TCP-F-RESET](../../protocol/tcp/features.md#tcp-f-reset) | RFC9293-RST-1 PASS; supporting RFC9293-RST-3 PASS | **supported** |
| [TCP-F-SEGMENT-ACCEPTANCE](../../protocol/tcp/features.md#tcp-f-segment-acceptance) | RFC9293-SEGA-1, SEGA-2, RST-2 all PASS | **supported** |
| [TCP-F-RESET-VALIDATION](../../protocol/tcp/features.md#tcp-f-reset-validation) | RFC9293-RSTP-1, RSTP-2 both PASS | **supported** |
| [TCP-F-WINDOW-ROBUSTNESS](../../protocol/tcp/features.md#tcp-f-window-robustness) | RFC9293-WND-4 PASS; supporting WND-3 PASS, WND-5 PASS | **supported** |
| [TCP-F-ICMP-HANDLING](../../protocol/tcp/features.md#tcp-f-icmp-handling) | RFC9293-ICMP-3 PASS, ICMP-1 PASS, ICMP-2 PASS | **supported** |
| [TCP-F-RTO-ESTIMATOR](../../protocol/tcp/features.md#tcp-f-rto-estimator) | RFC6298-FIRST-1 PASS, RTO-1 PASS, UPD-1 `owed` | **partial** |
| [TCP-F-RTO-BOUNDS](../../protocol/tcp/features.md#tcp-f-rto-bounds) | RFC6298-INIT-1 PASS, MIN-1 PASS; supporting MAX-1 PASS | **supported** |
| [TCP-F-RTO-BACKOFF](../../protocol/tcp/features.md#tcp-f-rto-backoff) | RFC6298-EARLY-1 PASS, BACK-1 PASS on the wire, EXP-1 `owed` | **partial** |
| [TCP-F-RTT-SAMPLING](../../protocol/tcp/features.md#tcp-f-rtt-sampling) | RFC6298-KARN-1 PASS; supporting SAMP-1 `owed` | **supported** |
| [TCP-F-CONGESTION-WINDOW](../../protocol/tcp/features.md#tcp-f-congestion-window) | RFC5681-SS-2 PASS, USE-1 `owed`, CA-2 `owed` | **partial** |
| [TCP-F-INITIAL-WINDOW](../../protocol/tcp/features.md#tcp-f-initial-window) | RFC5681-IW-1, IW-2, IW-3 all PASS | **supported** |
| [TCP-F-LOSS-RESPONSE](../../protocol/tcp/features.md#tcp-f-loss-response) | RFC5681-LOSS-1 PASS, LOSS-3 PASS; supporting LOSS-2 `owed` | **supported** |
| [TCP-F-FAST-RETRANSMIT](../../protocol/tcp/features.md#tcp-f-fast-retransmit) | RFC5681-FR-1, FR-2, FR-3 PASS, FR-5 `owed` | **partial** |
| [TCP-F-RESTART-IDLE](../../protocol/tcp/features.md#tcp-f-restart-idle) | RFC5681-IDLE-1 `owed` | **untested** |
| [TCP-F-DELAYED-ACK](../../protocol/tcp/features.md#tcp-f-delayed-ack) | RFC5681-ACK-2 PASS, ACK-1 `later` (statistical) | **partial** |

Twenty-three features: sixteen supported, six partial, one untested.

- `TCP-F-CHECKSUM` is partial because the send half fails in the default state: gap 2, a
  defect whose repair is blocked, declared by `4548adeb04`. The receive half passes.
- `TCP-F-WINDOW-ROBUSTNESS` and `TCP-F-ICMP-HANDLING` were a supported feature with a noted
  gap and a partial feature after pass 3. Both are supported now: gap 3 was a test error, and
  the ICMP repair `40c9f04e21` closed gap 4.
- The five partial features of level 4 are partial for one reason: a core statement has no
  check yet. No core check of level 4 fails. Gap 5, the only defect of pass 4, was repaired by
  `f3064a83d0`.
- `TCP-F-RESTART-IDLE` is untested, and the model claims it: its congestion control restarts
  an idle connection. The check is owed.

Bounds: every feature rests on one connection, one link, one data size per check, and one
loss at a time; the flow-control feature rests on a fixed small window, not on a closing one,
because the model's default mode never closes it (see the results). The four features of
pass 3 rest on a single crafted event each, which is what an edge is. The features of level
4 rest on the values that the model publishes; where it publishes none, the check measures
the interval on the wire (see the results).

## Achieved level

**Level 2 reached. Level 3 partial. Level 4 partial.** Target: level 4, from
[`standards.md`](../../protocol/tcp/standards.md#target-level).

Pass 4 recorded "4, reached". That record read the tables of RFC 9293 only, which this
ledger did not update until 2026-09-23. With RFC 6298 and RFC 5681 in the in-scope set, the
criteria of level 3 and level 4 cover their statements too, and neither criterion holds yet.

**Level 2.** Every normal-path mandatory mechanism of RFC 9293 appears as a feature, and every
mandatory feature has a core check that ran. The inventory:

| RFC 9293 mechanism | Where | Feature, or the reason it is not one |
| --- | --- | --- |
| header format, data offset, checksum, MSS option | §3.1, §3.7.1 | TCP-F-HEADER, TCP-F-CHECKSUM, TCP-F-ESTABLISH |
| sequence numbers, initial sequence number | §3.4 | TCP-F-SEQUENCE |
| connection establishment | §3.5 | TCP-F-ESTABLISH |
| connection close | §3.6 | TCP-F-TERMINATE |
| segmentation, acknowledgment, the push function | §3.7, §3.8, §3.9.1.2 | TCP-F-DATA-TRANSFER, TCP-F-ACKNOWLEDGE |
| window management | §3.8.6 | TCP-F-FLOW-CONTROL |
| reset for a connection that does not exist | §3.5.2, §3.10.7.1 | TCP-F-RESET |
| retransmission and its timer | §3.8.1 | RFC 6298 governs; the four TCP-F-RTO and TCP-F-RTT features |
| congestion control | §3.8.2 | RFC 5681 governs; TCP-F-CONGESTION-WINDOW, TCP-F-INITIAL-WINDOW, TCP-F-LOSS-RESPONSE, TCP-F-FAST-RETRANSMIT, TCP-F-RESTART-IDLE |
| zero-window probing, the acknowledgment delay bound, TIME-WAIT | §3.8.6.1, §3.8.6.3, §3.6 | level 4: timers and distributions |
| a shrunk window, silly window avoidance, reset on a live connection, ICMP handling, simultaneous open and close | §3.8.6, §3.5.2, §3.9.2, §3.10 | level 3: need a fault or a crafted segment |
| urgent data | §3.8.5 | **judgment:** mandatory to implement (MUST-30 to MUST-32) and discouraged in use by the same document; no ordinary transfer exercises it, so it is not a normal-path mechanism for level 2; level 5. The model does not support it, see the results |
| keep-alives, path MTU discovery, the other options, precedence and security | §3.8.4, §3.7.2, §3.2, §3.9 | `may`, `should`, or extensions; level 5 |

**Level 3, partial.** The criterion: the catalogs hold every mandatory statement of the
in-scope documents, and each one has a check. The first half holds for all three documents.
The second half holds for RFC 9293: pass 3 gave every `must` and `must not` of its edges a
check, and only RFC9293-ICMP-4, a `should`, has none. It does not hold for the two documents
of pass 4. Nine mandatory statements have no check:

| Document | Statements | What blocks them |
| --- | --- | --- |
| RFC 6298 | UPD-1, SAMP-1, GRAN-1 | a second measurement; a count of measurements per round trip; a steady round trip above one second |
| RFC 5681 | USE-1, CA-2 | a transfer that passes the threshold |
| RFC 5681 | SSTH-1 (the reduction), FR-5 | a test names each one and does not assert it |
| RFC 5681 | FR-4 | a fourth and a fifth duplicate acknowledgment |
| RFC 5681 | ACK-1 (the 500 ms bound) | a statistical check |

None of them needs a new tool: each needs a longer or a second scenario in a mockup that
exists, except ACK-1.

**Level 4, partial.** The criterion: the catalogs hold every timer and every control loop of
the in-scope set, and each one has a check with a stated tolerance. The first half does not
hold: the TIME-WAIT timer of RFC 9293 §3.6 has no catalog entry. The second half does not
hold either: RFC9293-ZWP-1, RFC9293-ACKD-1 as a bound and RFC5681-ACK-1 wait for statistical
checks, and the owed rows above include timer rules (RFC6298-TMR-1 to TMR-3, RFC5681-IDLE-1).

| Level | State | Evidence |
| --- | --- | --- |
| 1, Survey | **reached** | The standards map pins RFC 9293, RFC 6298 and RFC 5681; [`conformance.md`](conformance.md) maps the claimed RFCs onto it and names every obsolete one. |
| 2, Core | **reached** | The inventory above; every level-2 core statement of RFC 9293 has a PASS, and one declared unimplemented feature sits on a supporting statement. |
| 3, Edge | partial | RFC 9293: reached in pass 3. RFC 6298 and RFC 5681: nine mandatory statements without a check. |
| 4, Dynamics | partial | Ten features of the two control loops, each with a check that ran; eighteen owed statements; TIME-WAIT not in the catalog; three bounds wait for statistical checks. |
| 5, Complete | not started | urgent data, keep-alives, the options, PMTU. |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-08 | 2, partial | RFC 9293 in scope; 10 catalog entries; 4 features; 3 checks; 3 tests; flow control had no feature. On a branch since dropped; the tests were restored from it. | 5 PASS |
| 2 | 2026-09-08 | **2, reached** | 22 catalog entries (12 new), 9 features (5 new: data transfer, flow control, checksum, header, reset), 6 checks (3 new), 6 tests (3 new, 2 extended) | 8 tests: 7 PASS, 1 FAIL (expected) — the PSH bit; 8 features supported, 1 partial; see [`results.md`](results.md) and [`conformance.md`](conformance.md) |
| 3 | 2026-09-09 | **3, reached** | RFC 9293's edges added to the same catalog: 13 entries, 4 features, 11 checks, 11 tests. No new document | 19 tests: 15 PASS, 4 declared FAIL naming four model gaps; 11 features supported, 2 partial; see [`results.md`](results.md) |
| 4 | 2026-09-14 | recorded as 4, reached; corrected on 2026-09-23 to 3 partial and 4 partial | RFC 6298 and RFC 5681 enter the in-scope set: 39 catalog entries, 10 features, 10 checks, 10 tests, all new. The obsolete-citation sweep of the pass counted the old citations of the model; it changed no source file. | 27 tests: 22 PASS, 2 FAIL (expected), 3 FAIL (unexpected). One new finding, gap 5, a defect in the first round-trip measurement. Two of the level 3 declarations were withdrawn earlier in the pass, because they covered defects. See [`results.md`](results.md) |
| — | 2026-09-23 | 2 reached, 3 partial, 4 partial | Re-run only, no new check. The run record of pass 4 named `e0ac3b7307`, a commit that the landing rebase removed from master. This row gives the ledger a run on master, adds the 39 statement rows and the 10 feature rows of pass 4, and corrects the level. | 27 tests: 25 PASS, 2 FAIL (expected), 0 FAIL (unexpected). Changed since pass 4: RFC6298-FIRST-1 PASS (gap 5, `f3064a83d0`); RFC9293-WND-5 PASS (gap 3, a test error); RFC9293-ICMP-2 PASS (gap 4, `40c9f04e21`); RFC9293-CKSUM-1 declared (gap 2, `4548adeb04`) |

## Out of scope

The catalogs record what the passes left out: a shrunk or zero window beyond the probe
statement, the options other than MSS, urgent data, keep-alives, the simultaneous cases, and
the TIME-WAIT duration of RFC 9293; the options and extensions that RFC 6298 and RFC 5681
name as other documents. Each is level 3 to level 5 work, and the inventory above says which.
