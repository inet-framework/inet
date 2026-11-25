# Repair the defects that the baseline catch-up found

Status: **planned**, not approved. Created 2026-11-23 at the request of the maintainer: "save the defect list as a
pending plan in inet which needs to be done separately".

Checkout: INET `master` at `b436a63418` (2026-11-23), with the baseline pins statistics `853dd92020` and media
`cd94c731e7`. No local change.

Approval: pending. The request for this plan does not approve a repair ([PLR-APPROVAL](../../doc/project/rule/planning.md#plr-approval)).
Each repair needs its own approval before its implementation starts.

## 1. Where the list comes from

The baseline catch-up of 2026-11-12 to 2026-11-23 brought the statistical and chart baselines from the green point
`9ccb5c8e59` (2026-09-22, "G" below) up to master `bc39ec7571`, in 93 steps. For each commit that moved a result, the
catch-up found the cause and checked whether the change is correct. Each genuine bug that it found in a commit that it
examined went into a defect list as BD-n. The list holds old bugs and bugs with no effect on the results too.

A step whose cause is a defect kept the new values in the baseline, with the mark `Defect: BD-n` in the message of
the baseline commit. So the baselines of master hold the values of the open defects. A repair moves them again, with
its own explanation.

The list has 51 entries. Seven are repaired: BD-0, BD-1, BD-2, BD-22 and BD-45 by commits of the catch-up, and BD-42
and BD-50 by `858477687b` and `dd73c4376b` after it. This plan holds the 44 open entries. On 2026-11-25 the review of
change R2 of [baseline-catch-up-defect-repairs.md](baseline-catch-up-defect-repairs.md) against the RFC found two
more faults, and the maintainer added them as BD-51 and BD-52 ("yes to all of your questions"), so the list has 53
entries. The list itself is a file of
the job copy of the catch-up (`evidence/defects.json`), outside git, so this plan gives the complete facts of each
entry. The evidence of each step (logs, experiments, row differences) is in `evidence/steps/step-NNN/` of that job
copy. The baseline commit of each step in `statistics` and `media` gives the explanation of the step.

A check of master `d8a58564ee` on 2026-11-24 found BD-40 repaired before this list was written, BD-28 repaired, and
the IEEE 802.11 part of BD-33 repaired. An entry that the check changed has a line "State (2026-11-24)". So 42 entries
are open. The plan [baseline-catch-up-defect-repairs.md](baseline-catch-up-defect-repairs.md) repairs 27 of them: the
entries outside IEEE 802.11 that are new since v4.7.0 or easy. Later commits of that day address BD-29 and BD-30
(`bdd73c5a60`) and BD-31 (`ad34cc37dc`); their subsections of §3 describe them.

Each entry was found and checked at the commit that it names. The entries that say "master" were checked at
`bc39ec7571` or earlier. The commits from `bc39ec7571` to `b436a63418` change no source file of an entry: they change
LMac, one comment of the visualizer, the build and the baselines. A spot check at `b436a63418` finds the code of
BD-9, BD-30, BD-33, BD-34 and BD-37 unchanged.

## 2. Rules for each repair

1. Before the work, check the entry again on the master of that day. Master changes, and an entry can be repaired or
   changed by other work ([PLR-GROUNDED](../../doc/project/rule/planning.md#plr-grounded)).
2. Repair each entry, or each group of section 3 that shares one cause, as its own change: its own commit or series,
   its own classification and its own approval. Do not mix a repair with other work
   ([PR-SPLIT-DRIVEBY](../../doc/project/rule/pull-request.md#pr-split-driveby)).
3. Before the repair, write the test that shows the fault. Several entries were found by reading the code and never
   run (BD-25, BD-26, BD-29, BD-30, BD-32, BD-49; BD-11 on master). If the test does not show the fault, record that
   and close the entry.
4. A protocol repair needs its protocol tests and its reviewer document
   ([TR-PLAN-PROTOCOL](../../doc/project/rule/testing.md#tr-plan-protocol)). The "Check" line of each entry gives the
   idea of the test, not the test.
5. If a repair moves recorded results, re-record them in the same change, with the reason of each move
   ([change-a-baseline.md](../../doc/project/guide/change-a-baseline.md)). The "Baselines" line of each entry names the
   tests that hold values of the defect. "None" means that no test of the suites is known to show it.
6. TCP: the defaults of `~Tcp` follow the RFCs. A behavior that only Linux has gets a parameter, and `~LinuxTcp` sets
   it ([Tcp.ned](../../src/inet/transportlayer/tcp/Tcp.ned)).
7. An entry that is a limit of a model (BD-46, BD-47, BD-48) can end as a documented limit, if the maintainer decides
   not to extend the model.

## 3. Proposed order and groups

The maintainer decides the order. This proposal starts with the faults that stop a flow or a run, lose traffic, or
give values with no meaning.

| Priority | Entries | Reason |
| --- | --- | --- |
| 1 | BD-21, BD-44, BD-49, BD-33, BD-40, BD-39, BD-29, BD-30 | a TCP flow stops; pings get no answer with default bridge settings; NaN and a failed assertion; a run stops; a congestion window with no bound; a missing MPDU counts as received |
| 2 | BD-43, BD-38, BD-27, BD-26, BD-25, BD-35, BD-4, BD-7, BD-8, BD-14, BD-11, BD-32, BD-15, BD-24, BD-41, BD-37, BD-28 | the model departs from its standard or its own rules, and results move |
| 3 | BD-17, BD-18, BD-19, BD-20 | the defaults of `~Tcp` have a behavior of Linux with no parameter |
| 4 | BD-3, BD-10 | a value is not recorded, or keeps an old value |
| 5 | BD-31, BD-36 | the baselines of two examples show chance, not the effect of a commit |
| 6 | BD-5, BD-6, BD-9, BD-12, BD-13, BD-16, BD-23, BD-34 | text, documentation and small code faults with no known effect on the results |
| 7 | BD-46, BD-47, BD-48 | the limits of the obstacle loss model |

These entries share a cause or a place, so one change can repair each group:

- BD-14, BD-19 and BD-20: the count of the bytes in flight of TCP.
- BD-21 and BD-23: the override timer of the sender SWS avoidance.
- BD-29 and BD-30: `BlockAckRecord::getAckState()`.
- BD-8 and BD-39: the reaction of `TcpTahoe` to duplicate ACKs.
- BD-5, BD-13 and BD-16: comments of TCP, one commit.
- BD-46, BD-48 and BD-49: the constants and the equations of the material model.
- BD-31 and BD-36: the mapping of random streams in two examples.

The commit entries of each change ([PR-SERIES-PLAN](../../doc/project/rule/pull-request.md#pr-series-plan)) go into
this plan when the maintainer approves that change.

### Arithmetic 5 GHz band centers, BD-28

The maintainer requested implementation of this repair on 2026-11-23.

Commit `BD-28`, “ieee80211: fix: align 5 GHz arithmetic band centers to 5 MHz grid”.
Update `Ieee80211CompliantBands::band5GHz` in
[Ieee80211Band.cc](../../src/inet/physicallayer/wireless/ieee80211/mode/Ieee80211Band.cc) to pass `start = MHz(5002.5)`
so `Ieee80211ArithmeticalBand::getCenterFrequency(channelIndex)` produces `5000 + 5 * (channelIndex + 1)` MHz
(`5005..6000` MHz for indices `0..199`), following IEEE Std 802.11-2024 Clause 17.3.8.4.2 Equation (17-27) and
Clause 19.3.15.3 Equation (19-88) for `n_ch = 1..200` with 0-based `channelIndex = n_ch - 1`. Keep all other
bands and the unmapped standard-channel rejection of `"5 GHz"` unchanged.

- Extend [Ieee80211Band_1.test](../../tests/unit/Ieee80211Band_1.test) to assert all 200 center frequencies
  (`5005..6000` MHz), index `35` matching `"5 GHz (mapped 20 MHz)"` index `0` at `5180` MHz, out-of-bounds
  rejection (`-1`, `200`), and preserved `"5.9 GHz"` centers (`5860..5920` MHz). Before the fix, index `0` fails
  at `5002.5 MHz != 5005 MHz`.
- Add [A_5ghzBandCenters.test](../../tests/protocol/wifi/11a/A_5ghzBandCenters.test).
  An 802.11a infrastructure network (`ap`, `sta1`, `sta2`) runs on `"5 GHz"` channels `0` (`5005` MHz), `35`
  (`5180` MHz), and `199` (`6000` MHz) beside a passive ad-hoc `"5 GHz (mapped 20 MHz)"` index `0` listener
  (`5180` MHz). Check on-air `Ieee80211Channel` and `INarrowbandSignalAnalogModel` carrier frequencies on both
  initial management and `ST_DATA` transmissions, UDP delivery at `sta2`, and cross-band reception on `5180` MHz
  only in Run 1. On parent `0cb56f4a6f`, Runs 0 and 2 fail on the `2.5` MHz carrier mismatch (`5.0025` and
  `5.9975` GHz) and Run 1 fails with a scalar partial-interference error between `5.1775` GHz and `5.18` GHz.
- Re-record the moved `minSnir:histogram` statistical baselines for `showcases/wireless/multiradio` and
  `examples/wireless/nic` `Ieee80211MacV2X` in `inet-framework/statistics#5` and update the `inet.opp` pin.
- Build both `MODE=debug` and `MODE=release` (`make -j$(nproc) MODE=debug` and `make -j$(nproc) MODE=release`) and run:
  `opp_run_unit_tests --load . -p inet -m debug --no-build -f 'Ieee80211Band_1\.test'`,
  `opp_run_protocol_tests --load . -p inet -m debug --no-build -w '^tests/protocol/wifi$' -f '/(A_5ghzBandCenters|A_5ghz)\.test$'`,
  and `opp_run_protocol_tests --load . -p inet -m release --no-build -w '^tests/protocol/wifi$' -f '/A_5ghzBandCenters\.test$'`.
  Coverage limits: `A_5ghzBandCenters.test` exercises scalar-medium 802.11a OFDM transmissions on channels `0`,
  `35`, and `199`; `Ieee80211Band_1.test` checks all 200 indices; `"5 GHz"` intentionally retains no
  standard-channel mapping for HT management and provides no regulatory-domain model.
- Write [Arithmetic 5 GHz band centers](../../tests/protocol/wifi/5ghz-band-centers.md) beside the protocol test
  with the executed before/after comparison and reproducible commands.

### Basic Block Ack bitmap, BD-29 and BD-30

The maintainer approved implementation of this repair on 2026-11-23.

Commit `BD-29/30`, “ieee80211: fix: false acknowledgments in the Basic Block Ack bitmap”.
Adapt the receipt boundary from `5eb3f31938` for current Basic agreements. Seed the boundary from ADDBA and
advance it when Basic receipt history is retired, including receive-window advances that release no complete
MSDU. The bitmap uses cyclic ordering for old positions and exact sequence and fragment receipts otherwise.
See IEEE Std 802.11e-2005, 9.10.3.

- Add [Ieee80211BlockAckRecord_1.test](../../tests/unit/Ieee80211BlockAckRecord_1.test) for an empty record,
  a missing sequence across wrap, fragment-specific receipts, and retirement. Current master returns true
  for an in-range position in the empty record and can mistake the hole at 4094 for an old position.
- Add [WifiBasicBlockAckMissingData.test](../../tests/protocol/wifi/common/WifiBasicBlockAckMissingData.test).
  A station negotiates a Basic agreement with the AP, loses its first Block Ack policy DATA at the AP,
  and requests feedback after one MPDU. Check the AP's actual bitmap and the station's retry of the
  original sequence and TID. The empty record on the parent falsely acknowledges the missing DATA.
  Repeat with an out-of-window group frame in place of the missing unicast. Its sequence must leave the
  individual agreement untouched, so the bitmap still leaves the missing bit clear and the sender retries.
  Repeat with a group frame bearing the same sequence and fragment as the missing unicast. Check that the
  reply to the first received unicast retry sets the receipt bit.
- Extend [Ieee80211BlockAckReordering_1.test](../../tests/module/Ieee80211BlockAckReordering_1.test)
  with cases 11–17. In cases 11–13, drive successive BARs, one BAR exactly half a cycle past the last delivered receipt,
  and DATA beyond the receive window. Keep a live sequence partially received and check its positive and
  missing fragment bits in the Basic response. Check that a stale BAR leaves the current window intact.
  In case 14, receive fragment 0 at sequence 4095 and use three forward BARs to advance the window through
  a full cycle without new DATA. Check that the old receipt no longer sets fragment 0's bit when 4095 is reused.
  In cases 15–16, deliver a broadcast from the same peer and TID as an active Basic or Compressed agreement.
  The broadcast must reach ordinary delivery and leave the individual window and its receipts intact.
  Follow a broadcast with a matching first unicast retry and then a duplicate of that retry. Deliver the
  first retry once with either agreement and without an agreement (case 17).
- Build INET in debug mode and select these files with the unit and Wi-Fi protocol runners. Check the
  existing Compressed record and exchange tests as controls. The air-interface test covers an empty
  receiver record. The unit test covers sequence wrap and receipt retirement.
- Write [Basic Block Ack receipt reporting](../../tests/protocol/wifi/basic-blockack-receipts.md) beside
  the protocol test. Record the exact parent and repaired runs and their outcomes.

### Block Ack duplicate-drop reporting

The maintainer approved the six failing run-0 statistical baseline refreshes on 2026-11-24.

Commit `BA-REPORT`, “ieee80211: fix: report duplicate retries from the Block Ack receive window”.
Implement IEEE Std 802.11-2024, Table 10-6, RC9/RR4. The receive window handles duplicate
filtering under an agreement and the recipient reports confirmed retry drops. Extend
`Ieee80211BlockAckReordering_1` cases 15 and 16 to check the signal for both buffered and
previously delivered duplicates. Reuse the matching-sequence over-the-air test and the
[reviewer evidence](../../tests/protocol/wifi/basic-blockack-receipts.md).
Extend case 17 to leave a matching unicast entry in the ordinary cache before accepting
a Basic agreement. The first in-window retry must be delivered and emit no duplicate drop.

### Basic Normal Ack retry reporting

Commit `BA-BASIC-NORMAL-ACK`, “ieee80211: fix: count Normal Ack retries under Basic Block Ack”.
Depends on `BA-REPORT`. The Basic receipt record and reordering service retain evidence
of received Normal Ack fragments while keeping their bitmap bits clear. In the parent
the delivered sequence 100 leaves no receipt at the duplicate check, so its retry
is discarded without a signal.

- Add case 18 to [Ieee80211BlockAckReordering_1](../../tests/module/Ieee80211BlockAckReordering_1.test).
  An unseen fragment emits no duplicate signal. A repeat of the delivered Normal Ack fragment
  emits one. The Basic bitmap bit remains clear.
- Verify the approved release-mode `MixedTraffic` run-0 result. The receiver counts
  the retry of `Large-12`, raising the duplicate count from 12 to 13. The five other
  focused statistical comparisons retain their expectations.
- Build `MODE=debug` for the module fixture and `MODE=release` for the statistical comparisons. Run from the INET worktree root with the [module and statistical commands and filters](../../tests/protocol/wifi/basic-blockack-normal-ack-retries.md#verification).
- Record case 18 and the baseline comparison in [Basic Normal Ack retry reporting](../../tests/protocol/wifi/basic-blockack-normal-ack-retries.md).

### Compressed duplicate reporting after a reorder gap

Commit `BA-COMPRESSED-PRIOR-RECEIPT`, “ieee80211: fix: classify late Compressed retries using prior receipts”.
Follows `BA-BASIC-NORMAL-ACK` in the series and uses the duplicate callback from `BA-REPORT`.
The recipient service reads the Compressed receipt before updating it and passes that result
to `BlockAckReordering::processReceivedQoSFrame`. The declaration specifies the required order.
In the parent, a first-seen retry of sequence 101 can create the receipt that marks itself as a duplicate.

- Add case 19 to [Ieee80211BlockAckReordering_1](../../tests/module/Ieee80211BlockAckReordering_1.test).
  After reordering skips 101, the first arrival sets the Compressed bit and emits no duplicate
  signal. The next retry emits one signal.
- Run the six focused release statistical comparisons against the Basic follow-up's pin.
  Each is expected to match its reference.
- Build `MODE=debug` for the module fixture and `MODE=release` for the statistical comparisons. Run from the INET worktree root with the [module and statistical commands and filters](../../tests/protocol/wifi/compressed-blockack-retry-receipts.md#verification).
- Record case 19 and the baseline comparison in [Compressed retries after a reorder gap](../../tests/protocol/wifi/compressed-blockack-retry-receipts.md).

### LAN mobility random stream, BD-31

The maintainer approved implementation and the scoped baseline refresh on 2026-11-23.

Commit `BD-31`, “examples: fix: isolate lan80211 mobility from Wi-Fi backoff”.
Give the moving hosts RNG 1 in all three `lan80211` INI files. Wi-Fi backoff continues to draw from RNG 0.
Re-record the run-0 fingerprints and statistics for `TwoHosts`, `Streaming1` and `Ping1` in the same change.

- Automate the paired 100 s FTP `TwoHosts` run in
  [Lan80211MobilityIndependence_1.test](../../tests/module/Lan80211MobilityIndependence_1.test).
  At seed 0, vary TCP window scaling and check that both hosts keep their waypoint coordinates and change times
  even though the sink receives different amounts of data. The original runs diverge at 68.74 s. The fixed
  runs agree on all 101 logged waypoints.
- Check the run-0 baselines for the three configurations against the original and refreshed expectations. Use
  `tplx`, `~tND` and `~tNl` for fingerprints and release mode for statistics. Smoke-test `NHosts`, `Streaming2` and
  `Ping2` with four hosts.

### Group-frame Duration/ID, BD-25

The maintainer requested implementation on 2026-11-23.

Commit `BD-25`, “ieee80211: fix: stop reserving a successor for single-protection group frames”.
Apply IEEE Std 802.11-2024, 9.2.5.2 a) 9), to HCF's one-frame group sequence. Follow the
existing Block Ack-disabled forecast and omit the successor in the Block Ack-enabled path.
Keep the Duration/ID equation in single protection.

- Add [WifiSingleProtectionGroupDuration.test](../../tests/protocol/wifi/common/WifiSingleProtectionGroupDuration.test).
  Send group DATA with an already extracted successor, a positive TXOP limit, and single
  protection. Observe the transmitted headers and verify that each group frame ends its TXOP
  with zero Duration/ID. Use the model's fragmentation path to make the successor available.
  The legacy forecast can reserve it even though the group sequence ends. Run without Block
  Ack and with the Basic and Compressed profiles as separate cases.
- Run the same fixture before and after the fix with the debug Wi-Fi protocol runner and
  rerun `Ieee80211SingleProtectionGroup_1` as a control for the protection equation.
  Record the comparison in [Group-frame Duration/ID](../../tests/protocol/wifi/single-protection-group-duration.md).

### Basic Block Ack TXOP admission, BD-27

Commit `BD-27`, “ieee80211: fix: bound Basic Block Ack TXOP continuations”.
Use the group-frame reservation fix from PR #1481 as the parent. HCF checks the selected unicast
exchange before transmission, including RTS/CTS and any immediate ACK. Apply the strict TXNAV
comparison in IEEE Std 802.11-2024, 10.23.2.8, and count the leading SIFS against the nonzero TXOP
limit in 10.23.2.9. Keep the protected DATA rate selected before RTS through the CTS step.

- Add [WifiBasicTxopAdmission.test](../../tests/protocol/wifi/common/WifiBasicTxopAdmission.test).
  Use associated non-HT stations and queued UDP traffic to exercise Basic DATA in the production
  MAC. Check multiple protection with and without RTS, and use already extracted unicast fragments
  to exercise single protection. Check a tight TXOP deadline and a reservation too short for RTS/CTS
  plus DATA. Verify that fitting continuations can proceed and that deferred airtime does not
  prevent subsequent Basic DATA and feedback.
- Run the same fixture before and after the fix with a debug library and the Wi-Fi protocol runner
  filtered to `WifiBasicTxopAdmission.test`. The fixture covers continuations with positive
  limits at fixed rates and observes Block Ack feedback as evidence of progress.
  Record the results and controls in [Basic Block Ack TXOP admission](../../tests/protocol/wifi/basic-txop-admission.md).

### Basic Block Ack feedback scheduling, BD-43

Build on the Basic continuation admission from PR #1483.

1. “ieee80211: add: forecast Basic Block Ack requests before DATA transmission” adds the
   read-only query with the active TXOP and shares the existing ACK policy's selection rules.
   Extend [Ieee80211BlockAckScheduling_1.test](../../tests/unit/Ieee80211BlockAckScheduling_1.test)
   with one below-threshold case with an in-window successor and one with an empty queue
   after the current DATA. Check the forecast against live selection after transmission and
   verify that forecasting leaves the queue and ACK state intact. Verify that the ordinary
   and exclusion-aware peeks share a custom queue's ordering. Run the focused debug unit test
   and compare the selected run-0 release fingerprints against PR #1483.
2. “ieee80211: fix: reserve TXOP airtime for a due Basic Block Ack request” calls the forecast
   before HCF sends Basic DATA under single protection. Reserve the full feedback interval in
   the DATA Duration/ID if it fits the deadline. Check the actual request and reply against the
   live reservation and TXOP before submission. Add
   [WifiBasicBarReservation.test](../../tests/protocol/wifi/common/WifiBasicBarReservation.test)
   and compare a fitting request with one that must contend again. Verify the SIFS gap, grant
   identity, advertised duration and matching Basic bitmap in debug and release. Install a
   test ACK policy that checks the active TXOP and its remaining time when HCF calls the
   forecast for the observed DATA.

Compare the affected QoS and fragmentation configurations with matched seeds, using BARs that
open a grant and BARs that follow DATA to explain the change in contention. Keep agreement
retention a separate change. Record the test, workload and fingerprint comparisons in
[Basic Block Ack feedback scheduling](../../tests/protocol/wifi/basic-bar-reservation.md).

## 4. The entries

Each entry gives the place, the fault with its basis, the baselines that hold values of the fault, a proposed repair
and a proposed check. "Step N" is the step of the catch-up that found the entry.

### 4.1 TCP

#### BD-3: TCP does not record the initial cwnd and ssthresh

- **Place:** the TCP flavours, at the establishment of a connection. Older than G; shown by `13b4b270cc` (step 5).
- **Fault:** TCP does not emit cwnd and ssthresh when a connection is established. Before `13b4b270cc`, the initial
  ssthresh was visible only because Reno emitted the unchanged value at each growth step.
- **Baselines:** none. A repair adds rows to many statistical baselines.
- **Repair:** emit both values once, when the algorithm sets the initial window.
- **Check:** the first samples of the two vectors have the time of the establishment.

#### BD-4: a window update counts as a duplicate ACK

- **Place:** [TcpConnectionRcvSegment.cc](../../src/inet/transportlayer/tcp/TcpConnectionRcvSegment.cc), the update of
  the window before `receivedAckForAlreadyAckedData()`. From `45199ea23d` (step 7).
- **Fault:** condition (e) of the duplicate ACK of RFC 5681 (the advertised window does not change) compares the
  window of the ACK with `snd_wnd` after the connection wrote the window of this ACK into `snd_wnd`. So a window update
  (the same ACK number, a new window) still counts as a duplicate ACK. Only stale ACKs are excluded. Experiment W1 of
  step 7 (a test against the old window) moves 3 bulktransfer tests.
- **Baselines:** from step 7, at least `examples/inet/bulktransfer` `inet__inet`, `inet_inet_2a` and `inet_inet_2b`.
- **Repair:** compare the window of the ACK with the value of `snd_wnd` before the update.
- **Check:** a peer sends three ACKs with the same number and a larger window during a transfer. The sender must not
  start a fast retransmit.

#### BD-5: `updateWndInfo()` lost its comment

- **State (2026-11-24):** repaired by `519e772c16` (change R1 of [baseline-catch-up-defect-repairs.md](baseline-catch-up-defect-repairs.md)), on master on 2026-11-24.
- **Place:** [TcpConnection.h](../../src/inet/transportlayer/tcp/TcpConnection.h), `getFlightSize()` and
  `updateWndInfo()`. From `b2313324ef` (step 8).
- **Fault:** the new comment of `getFlightSize()` is below the comment of `updateWndInfo()`. The documentation tool
  attaches only the second comment to `getFlightSize()`, and `updateWndInfo()` has no comment.
- **Baselines:** none.
- **Repair:** put each comment above its function.
- **Check:** the fingerprints do not change.

#### BD-6: `calculateSsthreshForRto()` repeats the formula of `calculateSsthresh()`

- **Place:** [TcpClassicAlgorithmBase.cc](../../src/inet/transportlayer/tcp/flavours/TcpClassicAlgorithmBase.cc),
  `calculateSsthreshForRto()`. From `b2313324ef` (step 8).
- **Fault:** the function repeats the formula of `calculateSsthresh()` with `2*snd_mss` in place of `2*snd_effmss`.
  Now `snd_effmss` equals `snd_mss`, so no result changes. If the effective MSS differs one day, the lower limit after
  a timeout differs from the lower limit after a fast retransmit.
- **Baselines:** none.
- **Repair:** use one function for both cases, with `snd_effmss`.
- **Check:** the fingerprints do not change.

#### BD-7: after an ECN-Echo at a cwnd of one MSS, new data leaves at once

- **Place:** `processEce()` and `sendData()` of the connection. From `c875220148` (step 9); the missing effect is older
  than G.
- **Fault:** RFC 3168 section 6.1.2: at an ECN-Echo with a cwnd of one MSS, the sender resets the retransmission timer
  and sends a new packet only when the timer expires. INET restarts the timer, but `sendData()` after `processEce()`
  and the next ACKs send new data at once. In `TcpRenoIncast` client18, a new segment leaves in the same event at
  t=62.059170 s, and 1.2 ms after the reaction at t=62.173818 s, with an RTO of 1 s. The restart also occurs when the
  halving gives one MSS (up to 2 MSS before the halving), where the RFC does not ask for it.
- **Baselines:** `examples/inet/dctcp` `TcpRenoIncast` (from step 9).
- **Repair:** restart the timer only when cwnd is one MSS at the ECN-Echo, and send no new data until the timer
  expires.
- **Check:** an ECN-Echo arrives at a cwnd of one MSS. No new segment leaves before the timer expires.

#### BD-8: `TcpTahoe` takes ssthresh from cwnd, not from FlightSize

- **Place:** [TcpTahoe.cc](../../src/inet/transportlayer/tcp/flavours/TcpTahoe.cc) (line 107 at `bc39ec7571`). From
  `b1b214c002` (step 11).
- **Fault:** `TcpTahoe` sets ssthresh to max(cwnd/2, 2*SMSS) at the third duplicate ACK and at a timeout. RFC 5681
  equation (4) uses FlightSize, and RFC 5681 section 3.1 warns against the use of cwnd. When cwnd is larger than
  `snd_wnd`, the value is more than half of the data in flight. In `inet-tahoe` run 0 at t=0.412 s: cwnd 12328,
  `snd_wnd` and FlightSize 7504, ssthresh 6164, where equation (4) gives 3752. The value is above equation (4) at 222
  of the 240 third duplicate ACKs of `inet-tahoe` runs 0 to 6. Reno and NewReno use FlightSize since `b2313324ef`.
- **Baselines:** `examples/inet/tcpclientserver` `inet-tahoe` (from step 11).
- **Repair:** use the calculation of Reno, with FlightSize.
- **Check:** at the third duplicate ACK, with cwnd larger than the receiver window, ssthresh equals
  max(FlightSize/2, 2*SMSS).

#### BD-9: `Rfc6675Recovery` tests `isLost(snd_una + 1)`

- **Place:** [Rfc6675Recovery.cc](../../src/inet/transportlayer/tcp/flavours/Rfc6675Recovery.cc),
  `receivedDuplicateAck()`, line 332. From `d6da042a42` (step 12).
- **Fault:** RFC 6675 section 5, step (2), tests IsLost(HighACK + 1), that is the sequence number `snd_una`. The code
  tests `snd_una + 1`. The results are the same, because the segment at `snd_una` is never SACKed.
- **Baselines:** none.
- **Repair:** test `isLost(snd_una)`.
- **Check:** the fingerprints do not change.

#### BD-10: the `unacked` vector keeps an old value during SACK recovery

- **Place:** `Rfc6675Recovery::stepC()` and its Limited Transmit loop. From `d6da042a42` (step 12).
- **Fault:** these sends call `conn->sendSegment()`, which does not emit `unacked`. In `inet_inet_2b`, server conn-17,
  more than 40 segments leave from t=20.81 s to 21.81 s, and `unacked` does not change. Before `d6da042a42`, the
  recovery path of the connection emitted it. On master, `Rfc6675Recovery` emits `unacked` nowhere.
- **Baselines:** `examples/inet/bulktransfer` `inet_inet_2b` (from step 12).
- **Repair:** emit `unacked` after each send of the recovery, as the normal send path does.
- **Check:** the vector of `inet_inet_2b` changes at each send of the recovery.

#### BD-11: with SACK, the sender sends beyond the right edge of the window after a timeout

- **Place:** `TcpConnection::sendData()` ([TcpConnectionUtil.cc](../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc),
  line 2042 at `bc39ec7571`) and `sendSegment()`. Older than G; shown by `d6da042a42` (step 12).
- **Fault:** after a timeout, `sendData()` checks the window with `unackedInWindow = snd_nxt - snd_una`, and `snd_nxt`
  is back at `snd_una`. Then `sendSegment()` skips the SACKed data and sends at `snd_max`. In `inet_inet_2b`, client1
  at t=2.768369 s sends 383585..385037, and the right edge of the window is 383780: 1257 bytes beyond it. The send
  window ends at SND.UNA + SND.WND (RFC 9293 section 3.8.6). The parent of `d6da042a42` sent 51 such segments of
  client1, up to 47721 bytes beyond the edge; `d6da042a42` leaves 1. Master has the same window check; the case was
  not run on master.
- **Baselines:** `examples/inet/bulktransfer` `inet_inet_2b`.
- **Repair:** check the window against the sequence numbers that `sendSegment()` really sends.
- **Check:** with SACK, a timeout occurs when the next data that is not SACKed is near the edge of the window. No
  segment passes the edge.

#### BD-13: `TcpReno.h` keeps the comment of a removed function

- **State (2026-11-24):** repaired by `519e772c16` (change R1 of [baseline-catch-up-defect-repairs.md](baseline-catch-up-defect-repairs.md)), on master on 2026-11-24.
- **Place:** [TcpReno.h](../../src/inet/transportlayer/tcp/flavours/TcpReno.h). From `985edc2057` (step 15).
- **Fault:** the commit removes `TcpReno::isInFastRecovery()` but keeps its comment. The comment is now above
  `public:` and describes a function that does not exist.
- **Baselines:** none.
- **Repair:** delete the comment.
- **Check:** the fingerprints do not change.

#### BD-14: a fast retransmit after a timeout gives a burst

- **Place:** `TcpConnection::getFlightSize()` and `TcpClassicAlgorithmBase::getBytesInFlight()`. From `b2313324ef`
  (step 8); found at step 16.
- **Fault:** after a timeout, the go-back-N slow start sends the window again. A fast retransmit of Reno or NewReno
  without SACK in that time sets ssthresh from FlightSize = `snd_max - snd_una`, which still holds the data sent
  before the timeout. The room to send comes from a smaller count of bytes in flight: `snd_nxt - snd_una` before
  `5414ba0b34`, and the count of Linux without the lost bytes after it. So cwnd jumps, and the sender sends a burst.
  In `tcpwindowscale` `WS_enabled` at t=22.6027 s: cwnd 40656, outstanding 435600, ssthresh 217800, and about 150
  segments leave at once. In the terms of RFC 5681, FlightSize (435600) is larger than cwnd (222156), so no new
  segment can leave. Linux takes ssthresh from cwnd.
- **Baselines:** `examples/inet/tcpwindowscale` `WS_enabled`, and the other tests with a fast retransmit after a
  timeout (from step 8).
- **Repair:** use one count of the data in flight for ssthresh and for the room to send. Repair it with BD-19 and
  BD-20, which change the same count.
- **Check:** a timeout, then three duplicate ACKs during the go-back. The segments that leave after the fast
  retransmit fit in the room that RFC 5681 gives.

#### BD-15: the ACK of the SYN-ACK grows cwnd on the passive side

- **State (2026-11-24):** open on master `d8a58564ee`. A pure ACK of the SYN-ACK does not reach the algorithm (`TcpConnectionRcvSegment.cc:1858` skips it in SYN_RCVD), so the fault occurs only when the ACK of the SYN-ACK carries data (`:750-756`). Change R11 of [baseline-catch-up-defect-repairs.md](baseline-catch-up-defect-repairs.md) repairs it.
- **Place:** `TcpClassicAlgorithmBase::established()` and `receivedAckForUnackedData()`. From `a653d3bfbc` (step 17);
  the form with one MSS is older than G.
- **Fault:** `established()` sets cwnd to the initial window. Then the same ACK reaches `receivedAckForUnackedData()`
  with 1 acknowledged byte, the SYN. Before `a653d3bfbc`, cwnd grew by one MSS; after it, by 1 byte. RFC 5681 starts
  the data transfer with cwnd at the initial window, and the SYN is no data.
- **Baselines:** `examples/inet/tcpapp` `BurstyTraffic` and `RequestResponseTraffic`. Without the SYN byte, 4 cwnd
  rows of each change.
- **Repair:** do not count the SYN as acknowledged data for the congestion control.
- **Check:** on the passive side, cwnd after the ACK of the SYN-ACK equals the initial window.

#### BD-16: a wrong comment in `Rfc5681CongestionControl::congestionAvoidance()`

- **State (2026-11-24):** repaired by `519e772c16` (change R1 of [baseline-catch-up-defect-repairs.md](baseline-catch-up-defect-repairs.md)), on master on 2026-11-24.
- **Place:** [Rfc5681CongestionControl.cc](../../src/inet/transportlayer/tcp/flavours/Rfc5681CongestionControl.cc).
  From `80357d0e09`; found at step 19.
- **Fault:** the comment "cwnd is floored at 2*SMSS everywhere it is reduced" is wrong. A timeout sets cwnd to 1 MSS
  (`TcpClassicAlgorithmBase::processRexmitTimer()`), and the ECN-Echo reaction can leave it at 1 MSS. The code guards
  the division, so no result changes.
- **Baselines:** none.
- **Repair:** correct the comment.
- **Check:** the fingerprints do not change.

#### BD-17: `~Tcp` answers a repeated SYN in SYN_RCVD with a SYN-ACK

- **Place:** the processing of a SYN in SYN_RCVD of a passive open
  ([TcpConnectionRcvSegment.cc](../../src/inet/transportlayer/tcp/TcpConnectionRcvSegment.cc)). From `e8c4285dcd`
  (step 24).
- **Fault:** `~Tcp` sends the SYN-ACK again for a SYN that arrives again, as Linux `tcp_check_req()` does. RFC 9293
  answers such an unacceptable segment with a plain ACK. This behavior has no parameter and is always on, against the
  rule of `Tcp.ned`.
- **Baselines:** `examples/bgpv4/BgpAndOspfv3`, `examples/manetrouting/multiradio` `MultiRadio` (from step 24).
- **Repair:** a parameter for the behavior of Linux. `~Tcp` answers with an ACK, and `~LinuxTcp` sets the parameter.
- **Check:** a test for each value of the parameter.

#### BD-18: slow start of `~Tcp` has the limit of Linux `tcp_is_cwnd_limited()`

- **State (2026-11-24):** open on master `d8a58564ee`. The limit is also in `TcpCubic.cc:165`. Change R15 of [baseline-catch-up-defect-repairs.md](baseline-catch-up-defect-repairs.md) repairs it.
- **Place:** the slow start of the classic flavours. From `f7ff78102b` (step 18).
- **Fault:** the slow start of `~Tcp` grows cwnd only while cwnd is below twice the peak of the segments in flight of
  the current window, as Linux `tcp_is_cwnd_limited()` does, always. `Tcp.ned` says that the defaults of `~Tcp` have
  no mechanism that only Linux has, and this one has no parameter.
- **Baselines:** the 42 tests of step 18, and `DcTcpIncast` from step 19.
- **Repair:** a parameter, off in `~Tcp` and on in `~LinuxTcp`.
- **Check:** a sender that the application limits, in slow start, for each value of the parameter.

#### BD-19: without SACK, the classic flavours count the bytes in flight as Linux does

- **Place:** `TcpClassicAlgorithmBase::getBytesInFlight()`. From `5414ba0b34` (step 16).
- **Fault:** the count has an inferred SACK for each duplicate ACK and lost marks, always. The room to send equals the
  room of RFC 5681, except after a timeout, where the lost marks keep the room smaller. That difference is a mechanism
  that only Linux has, with no parameter.
- **Baselines:** the 6 behavior tests of step 16: `arptest2`, `bulktransfer` `inet__inet` and `inet_inet_2a`,
  `tcp_ppp_reconnect`, `tcpthroughput` `Delay-100ms-advWin64K`, and `tcpwindowscale` `WS_enabled`.
- **Repair:** a parameter. `~Tcp` uses the count of RFC 5681. Repair it with BD-14 and BD-20.
- **Check:** after a timeout, the room to send of `~Tcp` equals the room of RFC 5681.

#### BD-20: with SACK, outside a recovery, `~Tcp` counts the bytes in flight as Linux does

- **Place:** the count of the bytes in flight with SACK. From `958dee5cf3` (step 21).
- **Fault:** the count is the outstanding bytes less the SACKed and lost bytes, plus the retransmitted bytes, always.
  RFC 6675 defines its pipe only for the recovery, so this is a mechanism of Linux with no parameter. The maintainer
  counts it as a defect, although no RFC sets the old count either.
- **Baselines:** `examples/inet/bulktransfer` `inet_inet_2b` (from step 21).
- **Repair:** with BD-19.
- **Check:** outside a recovery, the room to send of `~Tcp` with SACK does not use the count of Linux.

#### BD-21: held data waits for ever, because the override timer does not expire while segments arrive

- **State (2026-11-25):** repaired by `54234c9502` (change R2 of [baseline-catch-up-defect-repairs.md](baseline-catch-up-defect-repairs.md)), on master on 2026-11-25, with the protocol tests and the reviewer document `tests/protocol/tcp/sws-override-timer.md`. No stored fingerprint or statistic moves.
- **Place:** `processWindowUpdate()` (from `40b842fc88`) and the persist timer. Shown by `6148d90738` (step 30).
- **Fault:** RFC 9293 section 3.8.6.2.1, rule (4), gives the sender SWS avoidance an override timeout. The persist
  timer is the override timer while the window is open. But `processWindowUpdate()` treats each segment with
  SEG.ACK = SND.UNA, an open window and nothing in flight as a window update. It cancels the persist timer, and
  `sendData()` starts it again with a full RTO. When the peer sends segments at intervals shorter than the RTO, held
  data waits for ever. This is the deadlock that rule (4) prevents. In `inet_inet_2b`: 3223 starts, 3222 cancels and
  0 expiries; the flow from the server to client1 stops at t=11.27 s.
- **Baselines:** `examples/inet/bulktransfer` `inet_inet_2b` (from step 30).
- **Repair:** do not restart a running override timer for a segment that does not change the window. Repair it with
  BD-23.
- **Check:** a sender holds a small segment, and the peer sends data at intervals shorter than the RTO with no change
  of the window. The held segment leaves when the override timer expires.

#### BD-23: the override timeout is one RTO, so at least 1 s

- **State (2026-11-25):** repaired by `8411e13607` and `4f05735a09` (change R2 of [baseline-catch-up-defect-repairs.md](baseline-catch-up-defect-repairs.md)), on master on 2026-11-25: the override timeout of `~Tcp` is the RTO held in 0.1 s to 1.0 s, and the parameter `swsOverrideAfterRto` keeps the RTO in `~LinuxTcp`. No stored fingerprint or statistic moves.
- **State (2026-11-24):** open on master `d8a58564ee`. The first interval is at least `minRexmitTimeout`: 1 s in `~Tcp` and 200 ms in `~LinuxTcp`. Change R2 of [baseline-catch-up-defect-repairs.md](baseline-catch-up-defect-repairs.md) repairs it with BD-21.
- **Place:** `startPersistTimer()`. From `6148d90738` (step 30).
- **Fault:** RFC 9293 section 3.8.6.2.1: "The override timeout should be in the range 0.1 - 1.0 seconds." The persist
  timer starts at the RTO, and `minRexmitTimeout` is 1 s, so the override comes after 1 s or more. The RFC also
  suggests the combination with the zero-window probe timer, whose first interval is the RTO (SHLD-29).
- **Baselines:** none at step 30. A repair can move the TCP tests with held data.
- **Repair:** an override interval in the range 0.1 s to 1.0 s, or a documented choice. Decide it with BD-21.
- **Check:** the time from the hold to the send is in the range.

#### BD-32: a data segment of 1 byte gets an ACK at once

- **Place:** [TcpConnectionRcvSegment.cc](../../src/inet/transportlayer/tcp/TcpConnectionRcvSegment.cc), "check for
  persist probe" ("TODO how to check if it is really a persist probe?"). Older than G; active by default from
  `fce9274b35` (step 44).
- **Fault:** the code sets `ack_now` for each data segment of 1 byte. So with delayed ACKs, 1 byte of data in an open
  window gets an ACK at once, where RFC 9293 SHLD-18 asks for a delayed ACK. A real zero-window probe is not
  acceptable (RFC 9293 section 3.10.7.4) and gets its ACK on that path already. Found in the code; not run.
- **Baselines:** none known.
- **Repair:** delete the special case.
- **Check:** with delayed ACKs, a segment of 1 byte in an open window gets a delayed ACK.

#### BD-34: two deprecated alias parameters of `Tcp.ned` still act

- **Place:** [Tcp.ned](../../src/inet/transportlayer/tcp/Tcp.ned): `increasedIWEnabled` and `ecnWillingness`. Found at
  step 48 (`588034dc8a`).
- **Fault:** `increasedIWEnabled` is the same as `initialWindow = "rfc3390"` (since `ed18cbd7ce`), and
  `ecnWillingness = true` is `tcpEcnMode = "rfc3168"` (since `88d161714a`). The maintainer decided on 2026-10-15
  (D-120) that a moved or renamed parameter loses its old name, with no deprecated alias. The aliases are older than
  that decision.
- **Baselines:** none.
- **Repair:** delete the two parameters and their code, change each use in the repository to the new parameter, and
  add a whatsnew entry.
- **Check:** no use of the old names remains; the fingerprints do not change.

#### BD-35: a loss that RACK found waits for a duplicate ACK, the RACK timer or the RTO

- **Place:** `Rfc6675Recovery::processSACKOption()`, `reoTimeout()` and `step4()`. Active by default from `eca05a10b4`
  (step 50).
- **Fault:** RACK marks losses at each ACK with new SACK information, also at an ACK that carries data. But only a
  duplicate ACK (with no data), the RACK reordering timer or the RTO acts on the marks. In a two-way transfer, whose
  ACKs come with data, a loss that RACK found waits. RFC 8985 section 6.2 runs the detection at each ACK, and its
  result starts the recovery. Check R1 of step 50 (act on the marks at once): the RTOs of three bulktransfer flows
  fall from 5 to 2, 3 to 1 and 3 to 1.
- **Baselines:** none hold a value of the defect alone. A repair moves `examples/inet/bulktransfer` (check R1).
- **Repair:** start the recovery when an ACK with data brings new loss marks.
- **Check:** in a two-way transfer, a loss that RACK finds starts the recovery at that ACK.

#### BD-39: `TcpTahoe` and `TcpWestwood` start a second fast retransmit in the same window

- **State (2026-11-24):** open on master `d8a58564ee`. The numbers of `inet-westwood` above come from before `7beeb0dc88`, which changes the threshold of Westwood. Change R5 of [baseline-catch-up-defect-repairs.md](baseline-catch-up-defect-repairs.md) repairs it with BD-8 and measures them again.
- **Place:** `TcpTahoe::receivedDuplicateAck()` and `TcpWestwood::receivedDuplicateAck()`. Older than G; shown by
  `07722bc21b` (step 59).
- **Fault:** the two flavours have no guard against a second fast retransmit in the same window. A third duplicate
  ACK that their own go-back retransmissions cause starts another fast retransmit and go-back: the functions test
  `dupacks == dupthresh` and nothing else. RFC 6582 section 6 names the problem and its guard, the variable `recover`,
  which `TcpNewReno` has ([TcpNewReno.cc](../../src/inet/transportlayer/tcp/flavours/TcpNewReno.cc), line 47). The
  hold of Nagle on the partial segment of a go-back hid most of it. After `07722bc21b`, `inet-tahoe` run 0 does 164
  fast retransmits in place of 7, with 8 queue drops, and sends 1198 segments in place of 721 for 1 MB (the data
  arrives at 1.83 s in place of 1.26 s). `inet-westwood` run 0 sends 2439 segments in place of 1497 and needs 36
  timeout periods in place of 21 (37.7 s in place of 22.5 s).
- **Baselines:** `examples/inet/tcpclientserver` `inet-tahoe` and `inet-westwood` (from step 59).
- **Repair:** the guard of RFC 6582 section 6 in both flavours.
- **Check:** after a loss and a go-back, one fast retransmit occurs in each window.

#### BD-40: `TcpWestwood` never measures the RTT, so its threshold never moves

- **State (2026-11-24):** **closed.** `7beeb0dc88` (2026-11-04, "tcp: fix: TcpWestwood takes an RTT sample from each ACK of new data") repairs it, with the tests `TcpAlgorithmClassWestwood.test` and `TcpAlgorithmClassWestwoodRto.test` and the reviewer document `tests/protocol/tcp/westwood-rtt-sample.md`. That commit is older than `bc39ec7571`, so the entry was already repaired when this list was written.
- **Place:** `TcpWestwood::receivedAckForUnackedData()`
  ([TcpWestwood.cc](../../src/inet/transportlayer/tcp/flavours/TcpWestwood.cc)). Older than G; shown by `db81577ca0`
  (step 64).
- **Fault:** the function calls `state->sentInfo.clearTo(state->snd_una)` before
  `state->sentInfo.get(firstSeqAcked)`, after the base class moved `snd_una` past the acknowledged segment. So the
  entry of that segment is gone. The RTT of the segment and `w_RTTmin` are never measured, and
  `recalculateSlowStartThreshold()`, which the duplicate ACK path and the timeout path guard with
  `w_RTTmin != SIMTIME_MAX`, never runs. The bandwidth estimate never sets the threshold: the ssthresh vector of the
  client of `inet-westwood` run 0 has no sample. The threshold keeps its initial value, and at the end of each fast
  recovery the window takes it: 65535 before `db81577ca0`, and 4294967295, the default `initialSsthresh`, after it,
  which is no bound. `TcpVegas` reads the entry first and clears the list later. The Westwood paper sets the
  threshold to the estimated bandwidth times the smallest RTT at each loss event.
- **Baselines:** `examples/inet/tcpclientserver` `inet-westwood` (from step 64).
- **Repair:** read the entry before `clearTo()`, as `TcpVegas` does.
- **Check:** the ssthresh vector of `inet-westwood` has samples, and after a loss the threshold equals the estimated
  bandwidth times the smallest RTT.

#### BD-51: data that TCP_CORK holds below the window waits as long as the cork stays set

- **State (2026-11-25):** open. Added by the maintainer on 2026-11-25. Not in the scope of
  [baseline-catch-up-defect-repairs.md](baseline-catch-up-defect-repairs.md) until a revision of that plan takes it.
- **Place:** `TcpAlgorithmBase::sendData()` and the cork timer
  ([TcpAlgorithmBase.cc](../../src/inet/transportlayer/tcp/flavours/TcpAlgorithmBase.cc)). From `6db7d1a459`
  (2026-10-13, "tcp: add: TCP_CORK and MSG_MORE, with the cork timer"), new since v4.7.0. Found by the review of R2
  against the RFC, not by a step of the catch-up.
- **Fault:** a partial segment that TCP_CORK holds, with a window below one segment, is also held by the sender SWS
  avoidance. The persist timer starts only when the cork timer does not run, so the override of RFC 9293 section
  3.8.6.2.1, rule (4), never starts. The expiry of the cork timer does not bypass the SWS hold, and the cork timer
  starts again. So the data waits as long as the cork stays set and the window does not grow. Linux
  `tcp_write_wakeup()` ignores the cork. Found in the code; not run.
- **Baselines:** none known.
- **Repair:** the expiry of the override sends the held data also under the cork, as Linux does. Decide the form at
  gate A.
- **Check:** a protocol test: the application sets TCP_CORK and writes less than one segment, and the peer advertises
  a window below one segment with no change. The data leaves at the expiry of the override timer.

#### BD-52: a retransmission timer that runs with nothing in flight delays the override timer

- **State (2026-11-25):** open. Added by the maintainer on 2026-11-25. Not in the scope of
  [baseline-catch-up-defect-repairs.md](baseline-catch-up-defect-repairs.md) until a revision of that plan takes it.
- **Place:** `TcpAlgorithmBase::sendData()`, the condition that starts the persist timer, and
  `TcpAlgorithmBase::processTimer()` ([TcpAlgorithmBase.cc](../../src/inet/transportlayer/tcp/flavours/TcpAlgorithmBase.cc)).
  From `e01c747832` (2026-10-09, "tcp: fix: no retransmission at a timeout with nothing in flight") and `6148d90738`
  (2026-10-13, "tcp: fix: the sender avoids the silly window syndrome (RFC 9293 MUST-38)"), new since v4.7.0. Found by
  the review of R2 against the RFC.
- **Fault:** after an ECN-Echo at a congestion window of one segment, RFC 3168 section 6.1.2 resets the retransmission
  timer also with nothing in flight. The persist timer starts only when the retransmission timer does not run. So data
  that the sender SWS avoidance holds in that time gets no override timer until the retransmission timer expires, and
  it can wait one RTO more than rule (4) of RFC 9293 section 3.8.6.2.1 permits. Found in the code; not run. BD-7
  (change R8) changes the same case of RFC 3168.
- **Baselines:** none known.
- **Repair:** start the override timer also while the retransmission timer runs with nothing in flight, or let the
  expiry of that timer send the held data. Decide it with BD-7.
- **Check:** a protocol test: an ECN-Echo at a cwnd of one MSS, then a write that the SWS avoidance holds. The held
  data leaves at the expiry of the override timer.

### 4.2 IEEE 802.11

#### BD-12: the text of the QoS showcase gives the delays of an old run

- **Place:** [showcases/wireless/qos/doc/index.rst](../../showcases/wireless/qos/doc/index.rst). From `fb7aeea94a`
  (step 14); the best effort value is older than G.
- **Fault:** the table "Average end-to-end delay" gives 1300 ms (background) and 345 ms (best effort) for the QoS run.
  The run gave 1312 ms and 414 ms before `fb7aeea94a`, and 750 ms and 233 ms after it. The description of the delay
  and jitter charts and the jitter table come from the same run. The commit updated the charts but not the text.
- **Baselines:** none (text).
- **Repair:** prior commits on `master` (`fa5de30a6d`, `42dbb75bfb`, `28f1dc534c`) updated the end-to-end delay
  table (`112, 112, 112, 115 ms` non-QoS; `698, 256, 1.5, 0.8 ms` QoS), background delay fluctuation onset
  (`1.6 s`), throughput chart y-axis (`7 Mbps`), and post-6 s throughput text (`increases sharply`). Commit
  `BD-12` (“showcases: fix: match the QoS jitter table and text to the current runs”) repairs the remaining stale
  jitter table (mean absolute instantaneous packet delay variation: `0.77, 0.68, 1.6, 5.9 ms` non-QoS and
  `6.7, 1.1, 0.75, 0.45 ms` QoS) and jitter prose (non-QoS voice spacing, post-6 s queue-drain negative
  delay variation, and the `-0.3 ms` background A-MSDU band in the zoomed plot).
- **Check:** verified against release runs of `NonQos` and `Qos` (`run 0`) on `master` `e74ac35bbf` (identical in
  code and baselines to `36cac8abf2`, PR #1489): all compared scalar and vector-summary metrics match the
  `statistics` pin `0efeed37319c` (`2,049` `NonQos`, `4,888` `Qos`, `0` diffs) and all 8 exported 150 DPI
  `Qos.anf` charts match the `media` pin `838086b3f96d` pixel for pixel (`0` differing pixels; no baselines move).

#### BD-25: the Duration/ID of a group frame can reserve a successor that does not come

- **Place:** the Duration/ID of a group frame that is not the final fragment. From the commit message of `c493ce2aa9`
  (step 35).
- **Fault:** with Block Ack support enabled, the legacy Duration/ID calculation can include an already extracted
  successor even when the group frame ends the TXOP. The regression reproduces a 96 µs reservation for a frame
  that transmits in a later TXOP.
- **Baselines:** none.
- **Repair:** apply IEEE Std 802.11-2024, 9.2.5.2 a) 9), by omitting the successor from the single-protection
  estimate when HCF ends the TXOP after the group frame.
- **Check:** the two group MPDUs each advertise zero Duration/ID in their own TXOPs with Block Ack disabled
  and with Basic or Compressed support enabled.
  [Regression evidence](../../tests/protocol/wifi/single-protection-group-duration.md).

#### BD-26: a zero-limit TXOP loses the rest of a fragment burst

- **Place:** [HcfFs.cc](../../src/inet/linklayer/ieee80211/mac/framesequence/HcfFs.cc) `hasMoreTxOps()`, and
  [Hcf.cc](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc) `transmitFrame()` and
  `canAdmitPreparedData()`. Older than `6e812198d4` (step 37).
- **Fault:** with a TXOP limit of 0 (the default of AC_BE and AC_BK), HCF sends one frame exchange in each TXOP.
  `HcfFs::hasMoreTxOps()` stops at `getRemaining() <= 0`, and from `6e812198d4` `Hcf::transmitFrame()` also refuses
  each later step at `txop->getLimit() == 0`. IEEE Std 802.11-2024 10.23.2.9 a) 1) permits "One or more SU PPDUs
  carrying fragments of a single MSDU or MMPDU" in such a TXOP, with the required acknowledgments. So each fragment of
  one MSDU needs a new channel access. Found in the code; not run.
- **Baselines:** the zero-limit repair changes the three default fingerprints and the statistical
  baseline in run 0 of `showcases/wireless/blockack / Fragmentation` and
  `examples/wireless/mactest / MacEdca`. The first uses AC_BE. In MacEdca, the fragmented
  FTP traffic uses AC_BK. The HCF fragmentation showcase uses port 4000 mapped to AC_VI.
  Its Basic Block Ack packet-size and SNIR charts move when HCF starts checking the lifetime
  of outgoing DATA in the Basic BAR forecast.
- **Preparation BD-26-IDENTITY**, “ieee80211: add: identify and forecast a fragment continuation”.
  Add the shared fragment match, accepted-ACK query and HCF airtime forecast before enabling continuation. The
  [identity unit test](../../tests/unit/Ieee80211FragmentContinuation_1.test) checks the immediate successor
  and rejects different receiver, TID, sequence, allocation, skipped fragment, finished MSDU and A-MSDU cases.
  The preparation leaves transmission behavior unchanged.
- **Commit BD-26**, “ieee80211: fix: continue fragment bursts in zero-limit TXOPs”. Use the accepted
  ACK and preceding transmit step to match the eligible in-progress successor in HCF selection and
  submission. Compare the receiver, type, sequence number, fragment number, and, for DATA, TID and local
  sequence allocation. A single-protection fragment advertises the next exchange. Multiple protection
  budgets the eligible fragment burst when it establishes its reservation. HCF rechecks TXNAV at submission.
- **Protocol tests:** Add [N_TxopFragments.test](../../tests/protocol/wifi/11n/N_TxopFragments.test)
  and [N_TxopFragmentRetry.test](../../tests/protocol/wifi/11n/N_TxopFragmentRetry.test), adapting the
  fixtures from `98cc0c7f99`. After infrastructure association, a sender queues two AC_BE MSDUs.
  Check SIFS between the ACK and the next fragment, Duration/ID through the final ACK, and fresh
  contention for the second MSDU. In the loss fixture, suppress one addressed ACK and check that the
  same fragment retries in a fresh grant, then the next fragment follows the successful retry at SIFS.
  The parent splits the fragments across grants. The tests cover Normal Ack at zero TXOP limit and
  exercise protection modes with fixed rates. The multiple-protection cases also use a 1 µs
  propagation delay to check ACK coverage with a 2 µs round-trip allowance and fresh contention
  when the allowance is zero.
- **Reviewer document:** [zero-txop-fragment-continuation.md](../../tests/protocol/wifi/zero-txop-fragment-continuation.md).
  Run `opp_run_protocol_tests --load . -p inet -m <mode> --no-build
  -w '^tests/protocol/wifi$' -f 'N_TxopFragment(s|Retry)\.test$'` on the release parent build
  and the debug and release builds of the repair.

- **Commit BD-26-BASIC-BAR-EXPIRY**, “ieee80211: fix: exclude expiring DATA from the Basic BAR forecast”.
  Following BD-26 and the BD-43 BAR forecast, make HCF pass the scheduled DATA completion time
  to the originator QoS ACK policy. Filter outstanding DATA and the transmitting frame at that time
  before deciding whether a BAR is due. In run 2 of
  [WifiBasicBarReservation](../../tests/protocol/wifi/common/WifiBasicBarReservation.test),
  the 920 µs lifetime ends during transmission. The parent advertises 128 µs for a BAR and reply
  that HCF skips. Require zero Duration/ID and no matching BAR. Keep the live-DATA controls.
  Run the fixture in debug and release and compare the Basic packet-size and SNIR charts.
  The [reviewer notes](../../tests/protocol/wifi/basic-bar-reservation.md) cover the exchange.

- **Commit BD-26-BASIC-SUCCESSOR-EXPIRY**, “ieee80211: fix: skip single-protection reservations for expiring successors”.
  This follows BD-26-BASIC-BAR-EXPIRY. In
  [Hcf::transmitFrame()](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc),
  the fallback still reserves 140 µs for the queued sibling of a Basic Block Ack fragment that expires
  during its PPDU. The sibling shares that deadline and HCF discards it before it can transmit.
  Check the already in-progress candidate at DATA completion plus SIFS before advertising the
  following exchange under IEEE Std 802.11-2024 §9.2.5.2(a)(9).
  The Basic series in the fragmentation showcase moves in the packet-size and SNIR charts.
  In the 6,000-byte run, the first changed frame advertises zero Duration/ID after the check.
  Extend [WifiBasicBarReservation.test](../../tests/protocol/wifi/common/WifiBasicBarReservation.test).
  In run 3, establish a Basic Block Ack agreement, fragment at 850 bytes, and give voice DATA a
  1,300 µs lifetime. Assert that fragment 0 starts live and expires during its PPDU. The parent
  fails the zero-Duration/ID assertion. The repair advertises zero, discards fragment 1, and sends
  no BAR for that sequence. Run 4 keeps the sibling live and checks coverage of SIFS plus DATA and
  same-grant continuation with RTS deferred and a BAR threshold of two.
  Record the cases in [basic-bar-reservation.md](../../tests/protocol/wifi/basic-bar-reservation.md#fragment-successor-lifetime).
  Use seed 0 with `opp_run_protocol_tests --load . -p inet -m <mode> --no-build
  -w '^tests/protocol/wifi$' -f 'WifiBasicBarReservation\.test$'` on the release parent
  and on the repair in debug and release. The assertions observe originator behavior.
- **Preparation BD-26-BASIC-RETRY-LOOKUP**, “ieee80211: refactor: expose timed in-progress DATA selection”.
  Extract the ordered lookup in [InProgressFrames](../../src/inet/linklayer/ieee80211/mac/queue/InProgressFrames.cc).
  Route peeking, live extraction and successor selection through one overridable in-progress
  ordering operation. Use the current simulation time for the ordinary peek and retain its pending-queue fallback.
  Check order, exclusion, ACK eligibility, and the exact expiry boundary in
  [Ieee80211BlockAckScheduling_1.test](../../tests/unit/Ieee80211BlockAckScheduling_1.test) with
  `opp_run_unit_tests --load . -p inet -m debug --no-build -f 'Ieee80211BlockAckScheduling_1\.test$'`.
  The default queue retains storage order. Custom queues implement their retry ordering once
  for forecasts and actual transmissions.
- **Commit BD-26-FRAGMENT-BATCH-ORDER**, “ieee80211: fix: honor queue ordering after fragment batch extraction”.
  When successor selection extracts several fragments, register the full batch and ask the queue
  which eligible fragment comes next. In `Ieee80211BlockAckScheduling_1`, a queue that prefers
  fragment 1 must return it both immediately after extraction and at live transmission selection.
- **Commit BD-26-BASIC-RETRY-SUCCESSOR**, “ieee80211: fix: reserve the next live Basic Block Ack retry”.
  This uses BD-26-BASIC-RETRY-LOOKUP and extends BD-26-BASIC-SUCCESSOR-EXPIRY. A missed Basic Block Ack leaves retries from two
  MSDUs in progress. During the first retry PPDU, the first MSDU's sibling expires, while a later
  retry stays live. The parent clears the reservation and the later retry contends again.
  Have HCF and [InProgressFrames](../../src/inet/linklayer/ieee80211/mac/queue/InProgressFrames.cc)
  forecast the first in-progress successor eligible at DATA completion. Reserve airtime only
  if that candidate still has lifetime at its earliest start after SIFS and the full exchange
  fits the TXOP. Include RTS/CTS and the immediate response when required. Check the complete
  exchange against the encoded single-protection field and the strict TXNAV admission rule.
  The Basic series in the SNIR chart moves. In its 0.07 mW run, a fitting replacement starts
  at SIFS in the original grant, 600 µs before the previous run starts that retry.
  Add [WifiBasicBarRetrySuccessor.test](../../tests/protocol/wifi/common/WifiBasicBarRetrySuccessor.test).
  Establish a real Basic Block Ack agreement and lose a reply covering two MSDUs. With no BAR due
  after the expiring retry, assert that the first retry advertises airtime for a live later
  sequence and that this sequence follows after SIFS in the original retry's grant.
  Repeat with a custom voice queue that prefers the second MSDU's shorter fragment 1.
  Require the opening Duration/ID to equal SIFS plus the airtime of the actual selected retry.
  Repeat with a 300 µs grant and a 2,710 µs lifetime. The live replacement needs
  160 µs including SIFS, while only 150 µs remain. Check zero Duration/ID at the sibling drop.
  The previous activation without the deadline check advertised 160 µs in this case.
  Retain the expired-sibling and live-sibling controls from BD-26-BASIC-SUCCESSOR-EXPIRY.
  Record the cases in [basic-bar-reservation.md](../../tests/protocol/wifi/basic-bar-reservation.md).
  Use seed 0 with `opp_run_protocol_tests --load . -p inet -m <mode> --no-build
  -w '^tests/protocol/wifi$' -f '(WifiBasicBarRetrySuccessor|WifiBasicBarReservation)\.test$'`
  on the release parent and on the repair in debug and release. The test observes originator
  selection, reservation and retry timing.
  In the same commit, extend [WifiBasicBarRetrySuccessor.test](../../tests/protocol/wifi/common/WifiBasicBarRetrySuccessor.test)
  to check response airtime. Use the opening retry's PHY mode for SIFS.
  In a 350 µs grant, have a fixture ACK policy choose Normal Ack for the identified replacement
  after the lost Block Ack. Check that DATA and the two SIFS intervals fit, that the configured
  immediate ACK pushes the exchange over the deadline, and that Duration/ID is zero.
  In a 4 ms grant, have a fixture RTS policy protect the identified replacement. Check that
  the complete exchange fits the TXOP and that the advertised reservation stays zero because
  single protection cannot encode enough airtime to satisfy strict TXNAV admission. Check the
  estimator's RTS, SIFS, CTS and DATA cost using 6 Mbps RTS and a 12 Mbps CTS override so the TXNAV
  refusal does not mask a missing airtime component or an ignored CTS override. Run the existing
  focused selector in debug and release. These are test cases for the existing estimator and do
  not change production behavior.

#### BD-27: with Basic Block Ack, HCF keeps the legacy admission of a further exchange

- **Place:** `Hcf::prepareDataTransmission()`. From the commit message of `6e812198d4` (step 37).
- **Fault:** with `isBlockAckSupported = true`, HCF keeps the legacy admission of a further exchange in a TXOP ("Block
  Ack support retains legacy admission"). The forecast of an RTS covers only RTS, SIFS and CTS. So an exchange can
  run past the remaining TXNAV and the TXOP limit (IEEE Std 802.11-2024 10.23.2.8 and 10.23.2.9). `6e812198d4` repairs
  this fault without Block Ack support. `ad59ba2ec1` admits prepared DATA with the complete cost of the exchange only
  for Compressed Block Ack: `Hcf::prepareDataTransmission()` answers LEGACY unless `blockAckVariant` is COMPRESSED.
  So Basic Block Ack keeps the fault (checked at `292582ba24` and at `bc39ec7571`).
- **Baselines:** five run-0 fingerprint cases change when a DATA continuation after an expired
  reservation waits for a fresh grant. See the [fingerprint comparison](../../tests/protocol/wifi/basic-txop-admission.md#fingerprint-comparison).
- **Repair:** the complete cost of the exchange for Basic Block Ack too.
- **Check:** a Basic DATA continuation fits the remaining TXNAV and the TXOP deadline, including
  protection and any immediate response. [Regression evidence](../../tests/protocol/wifi/basic-txop-admission.md).

#### BD-28: the centers of the arithmetic band "5 GHz" are 2.5 MHz off the grid

- **State (2026-11-24):** repaired by `d69fc0eaff` (2026-11-24).
- **Place:** `Ieee80211CompliantBands::band5GHz` in
  [Ieee80211Band.cc](../../src/inet/physicallayer/wireless/ieee80211/mode/Ieee80211Band.cc). Older than G; found at
  step 38 (`51721609f8`).
- **Fault:** the band has the start 5 GHz, the spacing 5 MHz and 200 channels. It puts the center of the channel index
  i at 5002.5 + 5i MHz (start + spacing/2 + spacing * i). IEEE Std 802.11-2024 equation (17-27) puts the centers at
  5000 + 5 * nch MHz. So every center of this band is 2.5 MHz off the grid. Users: `showcases/wireless/multiradio`,
  `examples/wireless/nic`, `tests/module/Ieee80211BeaconSchedule_1.test`.
- **Baselines:** `showcases/wireless/multiradio` (`General`) and `examples/wireless/nic` (`Ieee80211MacV2X`) move by
  ~0.10% in `minSnir:histogram` due to the `+2.5 MHz` carrier shift; wireless fingerprints do not move.
- **Repair:** set `start = MHz(5002.5)` so 0-based `channelIndex = i` maps to `n_ch = i + 1` (`5000 + 5 * (i + 1)`
  MHz, `5005..6000` MHz for `i = 0..199`), matching `band2_4GHz`'s zero-based convention.
- **Check:** [Ieee80211Band_1.test](../../tests/unit/Ieee80211Band_1.test) and
  [A_5ghzBandCenters.test](../../tests/protocol/wifi/11a/A_5ghzBandCenters.test)
  ([Arithmetic 5 GHz band centers](../../tests/protocol/wifi/5ghz-band-centers.md)).

#### BD-29: `BlockAckRecord::getAckState()` reports a missing MPDU as received across the wrap of the sequence number

- **Place:** [BlockAckRecord.cc](../../src/inet/linklayer/ieee80211/mac/blockack/BlockAckRecord.cc),
  `getAckState()`. Older than G; found at step 40 (`5891b964df`).
- **Fault:** the function takes `acknowledgmentState.begin()` as the earliest stored state, but the map orders by the
  plain sequence number. Across the wrap from 4095 to 0, it reports a missing MPDU as received: with the states
  {4093, 4095, 0}, the query 4094 gives true. So the originator does not send that MPDU again. Found in the code; not
  run.
- **Baselines:** none known.
- **Repair:** compare the sequence numbers modulo 4096, relative to the start of the window. Repair it with BD-30.
- **Check:** a unit test of `getAckState()` across the wrap.

#### BD-30: `BlockAckRecord::getAckState()` reports each MPDU as received when no state is stored

- **Place:** [BlockAckRecord.cc](../../src/inet/linklayer/ieee80211/mac/blockack/BlockAckRecord.cc), line 37
  (`return true; // TODO old?`). Older than G; found at step 40.
- **Fault:** when no state is stored, for example for a BAR before any Block Ack DATA arrived, the function reports
  each MPDU as received. Within the window, an MPDU that was not received must report 0. Found in the code; not run.
- **Baselines:** none known.
- **Repair:** within the window, report "not received" when no state is stored. Decide the answer for an MPDU before
  the window.
- **Check:** a unit test of `getAckState()` with no stored state.

#### BD-43: after `f874a26bd9`, Block Ack delivers less than Normal Ack

- **Place:** `Hcf::completeTransfers()` and the frame sequences of `f874a26bd9` (step 76).
- **Fault:** after `f874a26bd9`, Block Ack delivers less than Normal Ack in four of the five pairs of the statistical
  suite. Before, it delivered more in each pair:
  - `examples/wireless/qos` `MacQosWithBlockAck` against `MacQos`, in each access category: background 636 against
    902, best effort 4995, video 4861 and voice 4823 against 5000.
  - `showcases/wireless/fragmentation`: `PacketSizeHCFfragblockack` against `PacketSizeHCFfrag` 259 against 406,
    `ThresholdHCFfragblockack` against `ThresholdHCFfrag` 2238 against 2929, `OnlyPacketSizeHCFfragblockack` against
    `OnlyPacketSizeHCFfrag` 19795 against 19994.

  In the fragmentation runs, the BAR after each burst of fragments waits for a contention of its own ("BAR obtains a
  separate TXOP"). `PacketSizeHCFfragblockack` runs 1741 frame sequences in place of 665, and the medium has no frame
  sequence for 553 ms of 1000 ms in place of 142 ms. In the QoS run, `Hcf::completeTransfers()` also tears an agreement
  down each time the queue of its TID becomes empty (2418 agreements at the access point in place of 8). But an
  experiment without that teardown keeps the loss (557, 4036, 4965 and 5000 packets), so the loss comes from the
  changes of the frame sequence and the extraction. The EDCA rules let a TXOP holder send a further frame exchange,
  such as a BAR, within its TXOP limit, so the separate contention is a choice of the model, not of the standard. No
  Block Ack result moved after step 76, also not with the Compressed Block Ack commits `766ed7c015`, `0fe3286d55` and
  `ad59ba2ec1`.
- **Baselines:** `examples/wireless/qos` `MacQosWithBlockAck`, `showcases/wireless/fragmentation` (from step 76).
- **Repair:** let the TXOP holder send the BAR in its TXOP when the limit permits it. Examine the teardown of an
  agreement at each empty queue as a second change.
- **Check:** the BAR follows the burst in the same TXOP when the limit permits it. Compare the four pairs again.

### 4.3 The radio medium, the obstacle loss and the materials

#### BD-33: the default `maxTransmissionDuration` of the radio medium is shorter than a full-size 802.11 frame at 1 Mbit/s

- **State (2026-11-24):** the part of IEEE 802.11 is repaired by `0cb56f4a6f` (2026-11-24), which sets `mediumLimitCache.maxTransmissionDuration = default(35ms)` in `Ieee80211RadioMedium`. The generic default stays 10 ms, and `LMac`, `BMac` and `XMac` send at 19200 bps by default, so the rest stays open.
- **Place:** [MediumLimitCache.ned](../../src/inet/physicallayer/wireless/common/medium/MediumLimitCache.ned), line 30
  (`default(10ms)`). Older than G; found at step 47 (`ceb825e500`).
- **Fault:** a full-size 802.11 frame (about 1536 bytes with the headers) at DSSS 1 Mbit/s takes 12.3 ms, plus 192 µs
  of preamble. A run stops with "Maximum transmission duration is exceeded" when the rate control falls to 1 Mbit/s
  with full-size segments: `showcases/visualizer/canvas/instrumentfigures` at `ceb825e500`. The module test of the
  LMac repair (`tests/module/LMac_slot_too_short.test`) also sets the limit to 100 ms, because its frames at
  19200 bps are longer than 10 ms.
- **Baselines:** none.
- **Repair:** compute the limit from the radios of the network, or raise the default above the longest frame of the
  supported modes. Decide it with the maintainer. A different limit can move fingerprints.
- **Check:** an 802.11 host at DSSS 1 Mbit/s sends frames of 1500 bytes, and the run ends normally.

#### BD-46: the material model has constants that do not change with the frequency

- **Place:** [Material.cc](../../src/inet/environment/common/Material.cc) and
  [MaterialRegistry.cc](../../src/inet/environment/common/MaterialRegistry.cc). Older than G; found at step 91
  (`4c3c881815`).
- **Fault:** each material has a constant resistivity and relative permittivity, so its loss tangent is
  1 / (2 π f ρ εr ε0) at every frequency. Real materials change both values with the frequency. For example, real
  concrete takes much more loss at 60 GHz than the model gives (1.54 dB for 0.2 m). So `DielectricObstacleLoss` is
  only as good as these two constants at the frequency of the run.
- **Baselines:** none. This is a limit of the model.
- **Repair:** a decision of the maintainer: values that change with the frequency (for example the model of
  Recommendation ITU-R P.2040, which gives the permittivity and the conductivity as functions of the frequency), or a
  documented limit.
- **Check:** a unit test of the loss tangent at two frequencies.

#### BD-47: `DielectricObstacleLoss` ignores refraction, reflections inside the object and diffraction

- **State (2026-11-24):** open on master `d8a58564ee`. Also, `computeObjectLoss()` takes no reflection at the second surface: that code is a comment with `// TODO this returns NaN because n1 > n2` (`DielectricObstacleLoss.cc:116-121`).
- **Place:** `DielectricObstacleLoss::computeObjectLoss()`
  ([DielectricObstacleLoss.cc](../../src/inet/physicallayer/wireless/common/obstacleloss/DielectricObstacleLoss.cc)).
  Older than G; found at step 91.
- **Fault:** the function takes the loss along the straight line from the transmitter to the receiver through the
  object: the decay of a plane wave over the length of that line inside the object, and the reflection loss at the two
  surfaces. It ignores the refraction of the path, the reflections inside the object and the diffraction around it.
  Where the wavelength is comparable to the object or larger (below about 100 MHz for the walls of a building),
  diffraction dominates, and the model does not show it.
- **Baselines:** none. This is a limit of the model.
- **Repair:** a documented limit in the NED documentation of `DielectricObstacleLoss`, or a different model for low
  frequencies. A decision of the maintainer.
- **Check:** the documentation states the range of frequencies and sizes where the model applies.

#### BD-48: `computeReflectionLoss()` ignores the conductivity

- **Place:** `DielectricObstacleLoss::computeReflectionLoss()`. Older than G; found at step 91.
- **Fault:** the function takes the Fresnel equations with the real refractive index sqrt(εr μr) of the material and
  ignores its conductivity, that is the imaginary part of the permittivity. For a large loss tangent (a conductive
  material, or a low frequency: concrete has tan δ 3.99 at 10 MHz and 39.9 at 1 MHz), it gives too little reflection
  at the surfaces.
- **Baselines:** none.
- **Repair:** the Fresnel equations with the complex permittivity εr − jσ/(ω ε0).
- **Check:** a unit test against a value that is computed by hand for concrete at 10 MHz.

#### BD-49: copper and aluminium have NaN constants and wrong resistivities

- **Place:** [MaterialRegistry.cc](../../src/inet/environment/common/MaterialRegistry.cc). Older than G; found at
  step 91.
- **Fault:** copper and aluminium have the relative permittivity and permeability NaN, and the resistivities 1.68 and
  2.65 Ω m, which are 10^8 times the real values (1.68e-8 and 2.65e-8 Ω m). So their loss tangent and their
  propagation speed are NaN. `DielectricObstacleLoss` computes the factor NaN for an object of these materials, and
  `ASSERT(0 <= factor && factor <= 1)` fails where it is active. Found by reading the code; not run.
- **Baselines:** none. No example uses a metal object with `DielectricObstacleLoss`.
- **Repair:** the real resistivities, and a relative permittivity and permeability of 1.
- **Check:** a module test with a copper object and `DielectricObstacleLoss`. The factor is a number from 0 to 1.

### 4.4 Other protocols and modules

#### BD-24: the queue length of the compound `REDQueue` of netperfmeter becomes negative

- **State (2026-11-24):** open on master `d8a58564ee`. A code read finds the probable cause: the upper layer pushes through the gate `in`, which is connected to `red.in`, so `CompoundPacketQueueBase::pushPacket()` does not run and the compound emits no `packetPushStarted`, but the MAC pulls through the module, so the compound emits `packetPulled`. `queueLength` counts only the local signals of the compound (`CompoundPacketQueueBase.ned:33`). `RedDropperQueue` has the same connection. Change R12 of [baseline-catch-up-defect-repairs.md](baseline-catch-up-defect-repairs.md) checks the cause with a run and repairs it.
- **Place:** [netperfmeter.ned](../../examples/inet/netperfmeter/netperfmeter.ned) (in --> red.in, red.out --> fifo.in,
  fifo.out --> out, with the base `CompoundPacketQueueBase`) and
  [CompoundPacketQueueBase.cc](../../src/inet/queueing/queue/CompoundPacketQueueBase.cc). Older than G; found in the
  results of step 31 (`5118cd85ec`).
- **Fault:** `westernCoreRouter.ppp[0].queue` has `queueLength:last` -1977 and `queueBitLength:last` -4774528, which is
  minus the bits of all outgoing packets. The compound records `outgoingPacketLengths` but no
  `incomingPacketLengths`, so it counts the packets that leave but not the packets that enter. The cause (the example
  or `CompoundPacketQueueBase`) is not examined.
- **Baselines:** the three netperfmeter tests `TraceFile-Tcp`, `TraceFile-Sctp` and `TraceFile-Udp` (from before G).
- **Repair:** find the cause first, then count the packets that enter.
- **Check:** `queueLength:last` is 0 or more in the three runs; a module test of a compound queue.

#### BD-37: `PassivePacketSource::canPullPacket()` makes the packet

- **Place:** [PassivePacketSource.cc](../../src/inet/queueing/source/PassivePacketSource.cc), line 56 (`// KLUDGE`).
  Older than G; found at step 53 (`590ec7c85d`).
- **Fault:** `canPullPacket()`, a const query, makes the offered packet through a cast. So the creation time of a
  packet and the draw of its random length occur at the first query, not when the packet is pulled. This breaks the
  rule AR-QUEUE-QUERIES (a query changes nothing). `plan/done/queueing-pure-queries.md` records it as F1 and leaves
  it out of its scope. The lifetime of a packet depends on which module asks first: in `tutorials/queueing` `Filter2`,
  the collected packets have 0 s before step 53 and 1 s after it, with the same packets at the same collection times.
- **Baselines:** `tutorials/queueing` `Filter2` (from step 53).
- **Repair:** this needs its own design, because a filter after the source must see the packet before the pull.
  Start from F1 of `plan/done/queueing-pure-queries.md`.
- **Check:** a module test: a query does not change the creation time or the length of the next packet.

#### BD-38: BGP resolves a connection collision when it accepts the TCP connection

- **Place:** [BgpRouter.cc](../../src/inet/routing/bgpv4/BgpRouter.cc) `processMessageFromTcp()`, and
  [BgpFsm.cc](../../src/inet/routing/bgpv4/BgpFsm.cc) `OpenConfirm::BgpOpen` (a TODO). Older than G (`918c20d31c`,
  2026-06-30); found at step 57 (`55ed59fe28`).
- **Fault:** when the router accepts a TCP connection and the socket of the session is CONNECTING (BGP Connect state,
  TCP SYN_SENT) or CONNECTED, and the local address is higher, it aborts the incoming connection. RFC 4271 section 6.8
  detects a collision only when an OPEN arrives, against connections in OpenConfirm (or OpenSent). It says that "a
  connection collision cannot be detected with connections that are in Idle, Connect, or Active states", and it
  closes the loser with a NOTIFICATION (Cease), not with a TCP abort. In `examples/bgpv4/BgpLifecycle` after step 57,
  at 364 s router B aborts the open of the restarted router A, because the open of B (SYN_SENT since 240 s) still
  runs. The session comes up only at 367 s with the 7th SYN of B. If the SYNs of B could not reach A, the session
  would not come up until the open of B ends.
- **Baselines:** `examples/bgpv4/BgpLifecycle` `General` (from step 57).
- **Repair:** accept the connection, detect the collision when an OPEN arrives in OpenSent or OpenConfirm, and close
  the loser with NOTIFICATION Cease.
- **Check:** two routers open at the same time: one session stays, and the other connection gets Cease. Also the
  restart case of `BgpLifecycle`.

#### BD-41: QUIC records two vectors with the same name

- **Place:** [Statistics.cc](../../src/inet/transportlayer/quic/Statistics.cc) `createStatisticEntry()` and
  [Dplpmtud.cc](../../src/inet/transportlayer/quic/dplpmtud/Dplpmtud.cc) `initPmtuValidator()`. Older than G; found at
  step 70 (`666f0f0bc2`).
- **Fault:** `createStatisticEntry()` calls `getEnvir()->addResultRecorders()` at every call with the same signal
  name, and `initPmtuValidator()` makes a new `PmtuValidator` each time the DPLPMTUD search completes. So a run whose
  search completes twice records two vectors named `pmtuValidatorNumberOfLostPackets_cid=0_pid=0`, and two each of
  `pmtuValidatorTimeBetweenLostPackets` and `pmtuValidatorPersistentCongestions`, on `sender.quic`. The second recorder
  gets only the samples emitted after its creation: in `examples/quic/dplpmtud` `DecreasingPmtuWithoutPtb`, the first
  vector holds the lost packet of 5.0151 s, and the second is empty. The old statistical test tool of INET
  (`python/inet/test/statistical.py`, not on master now) merged the rows with `how='outer'`, so the duplicate rows
  paired crosswise and gave false differences when any other row of the file changed.
- **Baselines:** `examples/quic/dplpmtud` `DecreasingPmtuWithoutPtb` (from step 70).
- **Repair:** add the result recorders once for each signal name, or keep one `PmtuValidator` and reset it when the
  search completes again. Also check whether the comparison of the opp_repl runner has the same problem with
  duplicate rows.
- **Check:** the result file of the run has one vector of each name.

#### BD-44: `VlanReqMapper` sends priority-tagged frames on untagged ports

- **Place:** [VlanReqMapper.cc](../../src/inet/linklayer/vlan/VlanReqMapper.cc) `processPacket()`, and
  [PacketDirectionReverser.ned](../../src/inet/linklayer/ethernet/common/PacketDirectionReverser.ned)
  (`forwardPcp = true` by default). From `4a63e09916` (2023-01-17, older than G); found at step 78 (`688c3d74f2`) from
  its commit message.
- **Fault:** `processPacket()` removes the `VlanReq` of a frame when the port sends its VLAN untagged (a mapping to
  -1). But it still requests the 802.1Q encapsulation when a `PcpReq` is present
  (`ensureEncapsulationProtocolReq(..., hasTag<VlanReq>() || hasTag<PcpReq>())`). The tag inserter then writes a tag
  with the PCP and VID 0. So the frame leaves priority-tagged on a port where its VLAN must leave untagged. On the
  customer port of a provider edge bridge, the removed S-tag becomes an S-tag with VID 0. `PacketDirectionReverser`
  forwards the PCP by default, so each bridge with default settings has the fault on its untagged ports.
  `examples/ethernet/vlan` and the provider bridging example set `forwardPcp = false` to avoid it. Experiment E1 of
  step 78: with `forwardPcp = true` in the provider bridging example, no ping gets an answer (0 of 35 in 100 ms), and
  the customer bridges drop 12 frames at their inbound filters. Experiment E1 of step 79 (`d47fa914fb`, the form of
  the code on master): with `forwardPcp = true` in the shared core example, no ping gets an answer (0 of 80), and the
  bridges of the service providers drop 16 frames at their inbound filters.
- **Baselines:** none, because the examples set `forwardPcp = false`.
- **Repair:** do not request the 802.1Q encapsulation for a `PcpReq` alone when the VLAN leaves untagged. Then decide
  whether the examples keep `forwardPcp = false`.
- **Check:** a bridge with default settings sends a frame of an untagged VLAN with no tag. The pings of the provider
  bridging and shared core examples get answers with `forwardPcp = true`.

### 4.5 Examples whose results show chance

#### BD-31: `examples/wireless/lan80211` maps no random stream

- **Place:** `examples/wireless/lan80211/omnetpp.ini`, `omnetpp-ftp.ini` and `omnetpp-streaming.ini`. Older than G;
  found at step 43 (`45a6fb3016`).
- **Fault:** the `MassMobility` of the hosts and the MAC backoff draw from the same stream rng-0. Each change of the
  frame flow changes the number of backoff draws, and so the paths of the hosts and the times of their link breaks.
  The statistical baseline of `omnetpp-ftp.ini` `TwoHosts` moves by +116 %, -52 % and +347 % of the received bytes in
  steps 41 to 43. With the mobility on its own stream (`--num-rngs=2`, `**.mobility.rng-0=1`), the change of step 43
  is -0.6 %. So the test shows chance, not the effect of a commit.
- **Baselines:** the tests of `examples/wireless/lan80211` move with each repair. A repair re-records them once.
- **Repair:** give the mobility and the MAC their own streams in the three ini files.
- **Check:** the experiment of step 43 again: a small change of the frame flow moves the received bytes by a small
  amount.

#### BD-36: `examples/inet/shutdownrestart` maps no random stream

- **Place:** [examples/inet/shutdownrestart/omnetpp.ini](../../examples/inet/shutdownrestart/omnetpp.ini). Older than
  G; found at step 52 (`d1172eff41`).
- **Fault:** the random draws of the two `TelnetApp` clients (command counts, keystroke delays, think times) share the
  stream rng-0. Each change of the timing gives the clients other random values, and the statistical baseline of TCP
  moves: at step 52, the received bytes fall by 45 %. With one stream for each client (`--num-rngs=3`,
  `**.cli[0].app[0].rng-0=1`, `**.cli[1].app[0].rng-0=2`), the change is 232 -> 221 bytes at the server and none at
  the clients.
- **Baselines:** the tests of `examples/inet/shutdownrestart`. A repair re-records them once.
- **Repair:** one stream for each client.
- **Check:** the experiment of step 52 again.

## 5. What this plan does not establish

- The facts of each entry come from the catch-up, at the commit that the entry names. They are not checked again on
  `b436a63418`, except the spot check of section 1.
- The "Repair" and "Check" lines are proposals. They are not designs, and no code or test for them exists.
- The order of section 3 is a proposal. The maintainer decides the order and the scope of each change.
