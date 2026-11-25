# Implementation plan: repair the defects of the baseline catch-up that are new since v4.7.0 or easy, outside IEEE 802.11

Checkout and revision: `/home/projectured/workspace/inet-cpp`, branch `inet-julia` at `4d3705af84`. That branch
is `master` `d8a58564ee` (2026-11-24) plus 4 commits; of them, only `b0d922e023` changes code
(`src/inet/common/ResultRecorders.cc`), and no repair of this plan touches that file. The facts of §2 were read from
`master` `d8a58564ee` through git. The untracked files `plan/evidence/ieee80211-chart-evolution/` and
`plan/pending/statistical-and-chart-baseline-catch-up.md` belong to other work.

Plan revision: 2026-11-24, second text: a simpler workflow and the charts of the progress (§9).

Approval: approved by the maintainer on 2026-11-24 ("I approve the plan. The workflow is a bit too much this way, keep
it simpler. I also need charts to see the progress."). The approval covers the scope of §8. The decisions D1 to D10
and the proposed answers to the three questions of §8 count as approved with the plan. The second text changes only
the view of §9.

On 2026-11-24 the maintainer asked for this plan: "I want to create a plan which fixes the non 802.11 related defects
that were added since inet version 4.7 or are easy to fix. [...] I want to be able to follow the implementation of the
plan in the IDE."

## 1. Problem and intended behavior

The baseline catch-up of 2026-11-12 to 2026-11-23 found 51 defects (BD-n) in the commits that moved a result. The plan
[baseline-catch-up-defects.md](baseline-catch-up-defects.md) holds the 44 open entries, with the place, the fault, the
baselines, a proposed repair and a proposed check of each. The baselines of master hold the values of the open
defects.

This plan repairs a part of that list. An entry is in scope when both conditions are true:

1. It is not about the IEEE 802.11 model or an IEEE 802.11 example.
2. Its fault came into INET after v4.7.0 (the tag of 2026-07-07), or its repair is easy.

These terms have one meaning in this plan:

- **New since v4.7.0.** The faulty code comes from a commit that is not an ancestor of `v4.7.0`, or such a commit
  turned the faulty code on by default. `git merge-base --is-ancestor <commit> v4.7.0` decides it.
- **Easy.** The entry gives a repair of a known form in one place, with no open design question that only the
  maintainer can answer, and the cause is known. A wide but mechanical edit, or a wide but mechanical re-record of
  baselines, still counts as easy.
- **Change.** One repair of one entry, or of one group of entries that share a cause, with its own commits, its own
  classification and its own approvals (rule 2 of [baseline-catch-up-defects.md](baseline-catch-up-defects.md) §2).
  The changes have the names R1 to R19.

The maintainer follows the work in the window of `omnet_ide`, while it runs (§9).

## 2. Current state and evidence

### 2.1 Verified facts

| Fact | Source |
| --- | --- |
| `v4.7.0` is `dfe270b21f`, 2026-07-07, an ancestor of `master`. The catch-up started at G `9ccb5c8e59` (2026-09-22), 558 commits after `v4.7.0`. | `git log -1 v4.7.0`; `git rev-list --count v4.7.0..9ccb5c8e59` |
| Of the 34 commits that the entries name as an origin or a cause, only `918c20d31c` (BD-38) and `4a63e09916` (BD-44) are ancestors of `v4.7.0`. | `git merge-base --is-ancestor <commit> v4.7.0` for each of the 34 commits |
| The faulty code of BD-24, BD-32, BD-33, BD-37, BD-39, BD-41, BD-44 and BD-49 is in `v4.7.0`. | `git grep` at `v4.7.0`: `netperfmeter.ned:41`, `TcpConnectionRcvSegment.cc:484-486`, `MediumLimitCache.ned:30`, `PassivePacketSource.cc:56`, `TcpTahoe.cc:113`, `quic/Statistics.cc:37`, `VlanReqMapper.cc:61`, `MaterialRegistry.cc:37-38` |
| No commit from `bc39ec7571` to `master` changes `src/inet/transportlayer/tcp`. | `git log bc39ec7571..master -- src/inet/transportlayer/tcp` is empty |
| BD-40 is repaired. `7beeb0dc88` (2026-11-04, "tcp: fix: TcpWestwood takes an RTT sample from each ACK of new data") reads the entry before `clearTo()`, and it is an ancestor of `bc39ec7571`. So the entry was stale when the list was written. | `TcpWestwood.cc:112` and `:136` on master; `tests/protocol/tcp/westwood-rtt-sample.md` |
| BD-33 is repaired for IEEE 802.11 only. `0cb56f4a6f` (2026-11-24) sets `mediumLimitCache.maxTransmissionDuration = default(35ms)` in `Ieee80211RadioMedium`. The generic default stays 10 ms, and `LMac`, `BMac` and `XMac` have the bit rate 19200 bps by default, so a frame of 10 bytes takes about 17 ms. | `Ieee80211RadioMedium.ned:29`, `MediumLimitCache.ned:30`, `LMac.ned:39` |
| BD-44 is still in `VlanReqMapper`. `6ccb36a0b8` (2026-11-02) added `Ieee8021adPolicy`, which removes the PCP request on an untagged port, but `VlanPolicyLayer` still uses `VlanReqMapper` by default. | `VlanReqMapper.cc:61`, `Ieee8021adPolicy.cc:290-296`, `VlanPolicyLayer.ned:53` |
| The other 26 entries of §2.2 are open on master, at the places that §2.2 names. | three code reads of 2026-11-24, checked by samples |
| No file of a repair is sealed. Only `common/packet/` and `common/INETDefs.h` are sealed. | [seal-list.md](../../doc/project/audit/seal-list.md) |
| The CI runs the statistical tests in Ubuntu 26.04.1 with clang 21.1.8. This machine has Ubuntu 26.04.1 with clang 23. Some rows (error rates, the z3 schedule of `gsandcs`) move between such environments. | `ghci-baseline-catch-up/evidence/HANDOVER.md` §6.2; the clang-23 example of [change-a-baseline.md](../../doc/project/guide/change-a-baseline.md) |
| The job copy of the CI, `/home/projectured/workspace/ghci-baseline-catch-up`, runs a command in the image `localhost/ghci-statistical:26.04-clang21` with `tools/prun.sh run`. Its `inet` is a worktree of `/home/projectured/workspace/inet-cpp-baseline-catch-up`. Its OMNeT++ build and its ccache were deleted on 2026-11-23. | `tools/prun.sh`, `tools/ghci.sh`; `podman images` |
| A `Plan:` line of a commit must name a file that exists in the tree of that commit. | [pull-request.md](../../doc/project/rule/pull-request.md), the rule of the `Plan:` line |

### 2.2 The entries in scope

27 entries in 21 changes. "Origin" says whether the fault is new since v4.7.0. "Easy" gives the size that the code read
found on master.

| Change | Entries | Area | Origin | Easy | Place on master |
| --- | --- | --- | --- | --- | --- |
| R1 | BD-5, BD-13, BD-16 | TCP comments | new (`b2313324ef`, `985edc2057`, `80357d0e09`) | yes: 1 to 4 lines each | `TcpConnection.h:599-607`, `TcpReno.h:42-45`, `Rfc5681CongestionControl.cc:100` |
| R2 | BD-21, BD-23 | TCP: the override timer of the sender SWS avoidance | new (`40b842fc88`, `6148d90738`) | no: needs the decision of the override interval | `TcpAlgorithmBase.cc:1006-1022`, `:751-757` |
| R3 | BD-44 | `VlanReqMapper` sends priority-tagged frames on untagged ports | older (`4a63e09916`, 2023) | yes: 3 to 6 lines | `VlanReqMapper.cc:61` |
| R4 | BD-49 | NaN constants of copper and aluminium | older | yes: 2 lines | `MaterialRegistry.cc:37-38` |
| R5 | BD-8, BD-39 | `TcpTahoe` and `TcpWestwood` at the third duplicate ACK | BD-8 new (`b1b214c002`); BD-39 older, shown by `07722bc21b` | yes: the guard of `TcpNewReno.cc:43-48`, 20 to 30 lines | `TcpTahoe.cc:99`, `:107`, `TcpWestwood.cc:211` |
| R6 | BD-35 | TCP: a loss that RACK found waits | new (default from `eca05a10b4`) | no: needs the place of the start | `Rfc6675Recovery.cc:554-555`, `:17-52` |
| R7 | BD-4 | TCP: a window update counts as a duplicate ACK | new (`45199ea23d`) | yes: 5 to 10 lines, with a change of `ITcpRecovery::isDuplicateAck` or a state variable | `TcpConnectionRcvSegment.cc:1726`, `Rfc5681Recovery.cc:46` |
| R8 | BD-7 | TCP: after an ECN-Echo at a cwnd of one MSS, new data leaves at once | new (`c875220148`) | no: needs the form of the hold | `TcpClassicAlgorithmBase.cc:199-210` |
| R9 | BD-14, BD-19, BD-20 | TCP: the count of the bytes in flight | new (`b2313324ef`, `5414ba0b34`, `958dee5cf3`) | no: one or two parameters | `TcpClassicAlgorithmBase.cc:259-265`, `TcpConnectionUtil.cc:3777` |
| R10 | BD-32 | TCP: a data segment of 1 byte gets an ACK at once | new (default from `fce9274b35`) | yes: delete 3 lines | `TcpConnectionRcvSegment.cc:666-668` |
| R11 | BD-15 | TCP: the ACK of the SYN-ACK grows cwnd on the passive side | older; its form changed in `a653d3bfbc` | yes: 3 to 8 lines | `TcpConnectionRcvSegment.cc:750-756` |
| R12 | BD-24 | the queue length of the compound `REDQueue` becomes negative | older | yes, if the run confirms the cause of §2.4 | `netperfmeter.ned:41-43`, `CompoundPacketQueueBase.cc:51-80` |
| R13 | BD-41 | QUIC records two vectors with the same name | older | yes: 8 to 12 lines | `quic/Statistics.cc:34-37` |
| R14 | BD-17 | `~Tcp` answers a repeated SYN in SYN_RCVD with a SYN-ACK | new (`e8c4285dcd`) | no: a new parameter | `TcpConnectionRcvSegment.cc:211-239` |
| R15 | BD-18 | the slow start of `~Tcp` has the limit of Linux | new (`f7ff78102b`) | no: a new parameter | `Rfc5681CongestionControl.cc:57`, `TcpCubic.cc:165` |
| R16 | BD-3 | TCP does not record the initial cwnd and ssthresh | older | yes: 3 to 6 lines; it adds rows to many baselines | `TcpAlgorithmBase.cc:223`, `TcpClassicAlgorithmBase.cc:50` |
| R17 | BD-10 | the `unacked` vector keeps an old value during SACK recovery | new (`d6da042a42`) | yes: 2 to 6 lines | `Rfc6675Recovery.cc:140`, `:369` |
| R18 | BD-36 | `examples/inet/shutdownrestart` maps no random stream | older | yes: about 4 lines of ini | `examples/inet/shutdownrestart/omnetpp.ini:75-81`, `:108-113` |
| R19 | BD-34 | two deprecated alias parameters of `Tcp.ned` still act | new (`ed18cbd7ce`, `88d161714a`) | yes: mechanical, about 100 lines in about 80 files | `Tcp.ned:469`, `:497`, `TcpConnectionUtil.cc:1164-1202` |

| R20 | BD-6 | TCP: the lower limit of the ssthresh after a timeout uses `snd_mss` | new (`b2313324ef`) | yes: 1 line; it moves 6 fingerprint runs | `TcpClassicAlgorithmBase.h:66` |
| R21 | BD-9 | TCP: step (2) of RFC 6675 tests `isLost(snd_una + 1)` | new (`d6da042a42`) | yes: 1 line; it moves 1 fingerprint run | `Rfc6675Recovery.cc:332` |

R20 and R21 were part of R1 until the revision of 2026-11-24 (§6.3, R1).

The entries of R2, R6, R8, R9, R14 and R15 are not easy. They are in scope because they are new since v4.7.0.

### 2.3 The entries left out

| Entries | Reason |
| --- | --- |
| BD-12, BD-25, BD-26, BD-27, BD-29, BD-30, BD-43 | the IEEE 802.11 model or an 802.11 showcase |
| BD-31 | the random streams of the 802.11 example `examples/wireless/lan80211`: the MAC backoff and the mobility share rng-0 |
| BD-11 | older than v4.7.0; the repair changes the window check of the central send path `TcpConnection::sendData()` (10 to 25 lines that `effectiveWin`, `bytesToSend` and the SWS check depend on) |
| BD-33, the part outside 802.11 | older than v4.7.0; needs a decision of the maintainer (a computed limit or a new default), and a new generic default moves the fingerprints of all wireless runs that do not use `Ieee80211RadioMedium` |
| BD-37 | older than v4.7.0; needs its own design (F1 of `plan/done/queueing-pure-queries.md`) |
| BD-38 | older than v4.7.0; a change of the BGP state machine of about 60 to 120 lines, and INET has no BGP protocol test yet |
| BD-46, BD-47 | older than v4.7.0; limits of the model, which need a decision of the maintainer |
| BD-48 | older than v4.7.0; needs a complex refractive index in `IMaterial` and moves the results of each example with reflection loss |
| BD-40 | repaired by `7beeb0dc88` (§2.1) |

BD-28 is an 802.11 entry. It is repaired by `d69fc0eaff` (2026-11-24).

### 2.4 Corrections to the entries

The code read of 2026-11-24 found these differences from the text of
[baseline-catch-up-defects.md](baseline-catch-up-defects.md). Step 0 writes them into that plan.

- **BD-15.** A pure ACK of the SYN-ACK does not reach the algorithm: `TcpConnectionRcvSegment.cc:1858` skips
  `receivedAckForUnackedData()` in SYN_RCVD. The fault occurs only when the ACK of the SYN-ACK carries data
  (`:750-756`). The experiment of step 17 found it in `tcpapp`, so those runs have that case. R11 checks it first.
- **BD-18.** The limit is also in `TcpCubic.cc:165`. CUBIC is the algorithm of Linux, so R15 proposes to keep it
  there without the parameter.
- **BD-23.** The first interval is at least `minRexmitTimeout`: 1 s in `~Tcp` and 200 ms in `~LinuxTcp`.
- **BD-24.** The cause is found by a code read, not by a run. The upper layer pushes through the gate `in`, which is
  connected to `red.in`, so `CompoundPacketQueueBase::pushPacket()` does not run and the compound emits no
  `packetPushStarted`. The MAC pulls through the module, so the compound emits `packetPulled`. The statistic
  `queueLength` counts only the local signals of the compound (`CompoundPacketQueueBase.ned:33`), so it falls by one
  for each outgoing packet. `RedDropperQueue` (`RedMarkerQueue.ned:16`) has the same connection.
- **BD-39.** The numbers of `inet-westwood` come from before `7beeb0dc88`, which changes the threshold of Westwood.
  R5 measures them again.
- **BD-47.** `computeObjectLoss()` takes no reflection at the second surface: the code is a comment, with
  `// TODO this returns NaN because n1 > n2` (`DielectricObstacleLoss.cc:116-121`). The entry stays out of scope.

## 3. Acceptance criteria

1. For each entry of §2.2, a test shows the fault before the repair and passes after it. If the test does not show
   the fault on the master of that day, the entry is closed with the result of the test (rule 3 of
   [baseline-catch-up-defects.md](baseline-catch-up-defects.md) §2).
2. Each change moves a recorded result only with a record of the reason for each group of moved values, in the
   commit that causes the move ([change-a-baseline.md](../../doc/project/guide/change-a-baseline.md)).
3. Each commit of a protocol repair has its protocol tests and its reviewer document
   ([TR-PLAN-PROTOCOL](../../doc/project/rule/testing.md#tr-plan-protocol)).
4. The defaults of `~Tcp` follow the RFCs. A behavior that only Linux has gets a parameter, and `~LinuxTcp` sets it
   (rule 6 of [baseline-catch-up-defects.md](baseline-catch-up-defects.md) §2). After R9, R14 and R15, `~LinuxTcp`
   behaves as before.
5. The gates of [run-the-gates.md](../../doc/project/guide/run-the-gates.md), "Before every push", pass for each
   change.
6. [baseline-catch-up-defects.md](baseline-catch-up-defects.md) names the commit that repairs each entry, and it closes
   BD-40.
7. The window of `omnet_ide` shows the state of each stage and each entry, the builds and the tests that run, and the
   evidence of each approval (§9).

## 4. Proposed design and reasons

The assistant proposed these decisions. The maintainer approved them with the plan on 2026-11-24.

- **D1. The places.** The code changes go into a new worktree, `/home/projectured/workspace/inet-cpp-defect-repairs`,
  on the branch `topic/baseline-catch-up-defect-repairs` from `master`. Reason: other sessions use the main checkout,
  and its branch `inet-julia` holds commits that are not on master.
- **D2. Where the plan lives.** This plan is committed now on `inet-julia`, the branch of the main checkout. Step 0
  copies that commit with `git cherry-pick` as the first commit of the topic branch, so the `Plan:` line of each
  repair names a file of its own tree. From then on, the plan changes only on the topic branch, in separate
  `plan: change:` commits. When the first change lands, master holds the same first commit, and a rebase of
  `inet-julia` drops its copy.
- **D3. One change at a time, with two approvals.** Each change goes through the cycle of §6.1. The maintainer
  approves its design before its code (gate A), and its result before it lands (gate B). The maintainer can approve
  the design of several prepared changes at once. Reason: rule 2 of
  [baseline-catch-up-defects.md](baseline-catch-up-defects.md) asks for an approval of each repair, and the values of
  a baseline need their own approval ([change-a-baseline.md](../../doc/project/guide/change-a-baseline.md) §2).
- **D4. The order.** R1 first: it moves no result, so it checks the cycle, the tools and the view of the editor at a
  low risk. Then the order of [baseline-catch-up-defects.md](baseline-catch-up-defects.md) §3, from priority 1 to
  priority 6, with the groups of that section. The maintainer can change the order at each gate A.
- **D5. The detail of each commit comes at the preparation of its change.** This plan gives, for each change, the
  commit subjects, the place, the repair, the idea of the test and the expected baselines. The preparation of a change
  writes the items of [TR-PLAN-PROTOCOL](../../doc/project/rule/testing.md#tr-plan-protocol) for each of its commits
  into §6, before gate A. Reason: the master of that day decides the facts
  ([PLR-GROUNDED](../../doc/project/rule/planning.md#plr-grounded)), and 19 changes over several weeks see master move.
- **D6. The record of moved results runs in the image of the CI.** The assistant runs the fingerprint and statistical
  updates and the attribution run in the job copy of the CI, with `tools/prun.sh`, as tasks of the editor. The focused
  tests run in the worktree on this machine. Reason: this machine has clang 23 and the CI has clang 21 (§2.1). A value
  that this machine records can fail in the CI.
- **D7. The parameters of Linux.** R9, R14 and R15 follow the pattern of `dropSynData`: a `bool` in `Tcp.ned` under
  "Linux behaviors (no RFC)" with `default(false)`, an item in the header text of `Tcp.ned`, and
  `<name> = default(true)` in `LinuxTcp.ned`. Gate A of each change fixes the name.
- **D8. Who runs the processes.** The assistant of the `omnet_ide` window starts each build, test, update and
  attribution run as a task of the editor (§9), so the maintainer sees it, reads its log and can stop it. The
  assistant pushes nothing. The maintainer lands each change and pushes the baseline branch.
- **D9. Delegation.** Sonnet subagents do the mechanical parts: the sweep of R19, the search for the tests of a place,
  and the scan of a long log. The assistant writes each repair, each test and each record, and checks each result of a
  subagent. The `simulation-domain-expert` agent reviews the reasoning against the RFC of each TCP change that is not
  easy.
- **D10. Only the plan goes into git.** The plan is the record of the work: each step updates it. The evidence stays
  outside git: the files of the view (§9.3), the logs of the runs in the image of the CI, and the outputs of
  experiments. Their folder is `/home/projectured/workspace/ghci-baseline-catch-up/evidence/defect-repairs/`, beside
  the evidence of the catch-up, which is outside git too. Changed on 2026-11-24 by the maintainer: "I think we should
  not commit into the repository the evidence, that's just too much noise. It's enough if we have the plan to begin
  with and each step we update the plan as we go." No project rule asked for the evidence in git; the first text
  followed D6 of [ieee80211-chart-baseline-evolution.md](ieee80211-chart-baseline-evolution.md). The plan commits of
  that day were written again without the files, so no commit of this plan holds them.

## 5. Affected components and compatibility

| Component | Changes | Compatibility |
| --- | --- | --- |
| TCP (`src/inet/transportlayer/tcp`) | R1, R2, R5 to R11, R14 to R17, R19 | R9, R14 and R15 change the defaults of `~Tcp` to the RFC behavior; `~LinuxTcp` keeps its behavior. R19 removes `increasedIWEnabled` and `ecnWillingness`: an ini file that sets them gets an error. Each gets a whatsnew entry in `doc/whatsnew/incompatible/`. |
| VLAN (`src/inet/linklayer/vlan`) | R3 | A bridge with default settings sends the frames of an untagged VLAN with no tag. |
| Environment (`src/inet/environment/common`) | R4 | Copper and aluminium get real values; no example uses them with `DielectricObstacleLoss`. |
| Queueing (`src/inet/queueing/queue`) and `examples/inet/netperfmeter` | R12 | Recorded statistics of a compound queue change; the behavior does not change. |
| QUIC (`src/inet/transportlayer/quic`) | R13 | A result file has one vector of each name. |
| `examples/inet/shutdownrestart` | R18 | The random values of the clients change once. |
| Fingerprints (`tests/fingerprint/store.json`), `statistics`, `inet.opp` | each change that moves a result | Re-recorded in the commit that causes the move. |
| [baseline-catch-up-defects.md](baseline-catch-up-defects.md) | step 0 and each gate B | The corrections of §2.4, the repair commit of each entry, BD-40 closed. |

No sealed file changes. No consumer outside INET changes.

## 6. Steps and future commits

### 6.1 The cycle of a change

1. **Re-check.** Rebase the topic branch on the master of that day. Read the place of each entry again, and run the
   check of the entry if the code changed. Record the result in the table (§9.2).
2. **Show the fault.** Write the test of the entry. Build in debug mode and run the test in the editor. It must fail.
   If it passes, record the result, close the entry and go to step 9.
3. **Prepare.** Write the commit entries of the change into §6.3: subject, files, the items 1 to 5 of
   [TR-PLAN-PROTOCOL](../../doc/project/rule/testing.md#tr-plan-protocol), the design decision and the expected
   baselines. Commit it as `plan: change: prepare R<n>`.
4. **Gate A.** Open the prepared entry in a tab, and ask the maintainer: "Do you approve the design of R<n>?"
5. **Repair.** Write the repair. Build in debug and release mode. Run the new test and the existing tests of the
   place in the editor (§9.2).
6. **Results.** Run the fingerprint tests and the statistical tests of the affected folders in the image of the CI
   (D6). For each group of moved values, write one line from the code change through the mechanism to the statistic,
   with its evidence. Re-record the fingerprints in the same commit. Record the statistics in the image of the CI,
   commit them to the branch `topic/baseline-catch-up-defect-repairs` of `statistics` with the record, and move the
   pin in `inet.opp` ([change-a-baseline.md](../../doc/project/guide/change-a-baseline.md)).
7. **Document.** Write the reviewer document beside the protocol test and the whatsnew entry.
8. **Gates.** Run "Before every push" of [run-the-gates.md](../../doc/project/guide/run-the-gates.md). Run
   `check-baseline-attribution.sh plan` and `run` in the job copy of the CI. Run all stored fingerprints with all features, as the
   workflow "Test: fingerprint" runs them. A gate runs the release mode, in the editor, on the worktree, which has
   `opp_featuretool enable all` since 2026-11-24 (the maintainer: "why don't you run the fingerprints just here?").
   The debug mode runs once when a change lands, in the image of the CI (`runs/fingerprints-in-ci-image.sh <label>
   debug` of the folder of D10), because the fingerprint verb of the editor has no mode and runs release only (the
   maintainer, 2026-11-24: "run only release fingerprint tests, they are faster and debug tests is enough when
   landing"). The image of the CI also runs a fingerprint whose result in the editor differs from the CI. Each run
   must pass, except a run that a marker declares as expected (the maintainer, 2026-11-24: "all should pass
   otherwise how can we create a PR?").
   **Correction (2026-11-25).** The fingerprint verb of the editor runs **debug**, not release: the fingerprint kind
   of omnet-julia defaults to `mode = "debug"`, as `opp_repl` does (`LegacyFingerprintTestTask.jl:401`), `inet.opp`
   names no mode for it, and the verb has no `mode` keyword. So the "release" gate runs of R2 in the editor were debug
   runs. The maintainer, 2026-11-25: "We should simply fix the runner to allow running both." omnet-julia gets the
   repair: the mode that the caller or the Runner gives is the mode of the runs and of the build, and debug stays the
   default, as in `opp_repl`. Then a gate runs release in the editor again. Until that repair lands, a gate runs the
   release fingerprints in the image of the CI (`runs/fingerprints-in-ci-image.sh <label>`).
9. **Gate B.** Open the reviewer document, the moved values and the commits of the change in tabs. Ask the
   maintainer: "Do you approve the result of R<n>?" After the approval, mark the change done in this plan, write the
   repair commit into [baseline-catch-up-defects.md](baseline-catch-up-defects.md), and commit both as
   `plan: change: R<n> is repaired`. The maintainer lands the change.

### 6.2 The steps

- [x] **Step 0. Set up and correct the list.** Done on 2026-11-24; item 3 ends in R2 (below).
  1. Make the worktree and the branch of D1. Copy the plan commit (D2). Run `opp_checkout_test_baselines` in the
     worktree. **Done 2026-11-24.** The two plan commits of `inet-julia` are `cf630c7310` and `f19249ad58` on the
     topic branch. `opp_checkout_test_baselines` is not on the PATH of this machine, so `statistics` and `media` are
     clones with `--reference-if-able` of the clones of the main checkout, at the pins `9a67a652ff` and `cd94c731e7`.
  2. In the editor, open the worktree as a project with `open_simulation_project!`, and build it in debug and release
     mode as tasks. **2026-11-24:** the debug build is done in 4 min 22 s. The first release build stopped after 0.4 s,
     because it started with the debug build and the makefiles did not exist yet; it runs again alone. Start the two
     modes of a new tree one after the other: they also share the generated message files.
  3. In the job copy of the CI, run `tools/prun.sh build-omnetpp`, fetch the topic branch into
     `inet-cpp-baseline-catch-up`, and run one statistical test of `examples/inet/bulktransfer` at master. It must
     pass with the pin of master. **2026-11-24:** `ghci-baseline-catch-up/inet` is at the head of the topic branch,
     with `statistics` and `media` at the pins of master; the build of OMNeT++ in the image runs as a task. The build of OMNeT++
     passed in release mode, and in debug mode for the landing of R1. The statistical test of `bulktransfer` moves
     to R2, the first change that moves statistics (2026-11-24, after the maintainer asked whether the set-up was done).
  4. Write the corrections of §2.4 and the closure of BD-40 into
     [baseline-catch-up-defects.md](baseline-catch-up-defects.md). **Done 2026-11-24** in C2, which also records BD-28
     as repaired by `d69fc0eaff`.
  5. Make the view of §9 and save its files. **Done 2026-11-24.**
  - **C1: `plan: add: repair the defects of the baseline catch-up that are new since v4.7.0 or easy, outside IEEE 802.11`.**
    This plan. No dependency.
  - **C2: `plan: change: the defects of the baseline catch-up after the check of 2026-11-24`.** The corrections of
    §2.4 and the closure of BD-40. Depends on C1.
- [x] **Step 1. R1: TCP comments (BD-5, BD-13, BD-16).** Landed on master on 2026-11-24 as `519e772c16`. Check: no fingerprint changes, and the module tests of the
  places pass. Until the revision of 2026-11-24, R1 also held BD-6 and BD-9; they are R20 and R21 now.
  - `tcp: comment: each comment is above its function, and no comment describes a removed function` (BD-5, BD-13,
    BD-16).
- [x] **Step 2. R2: the override timer (BD-21, BD-23).** Landed on master on 2026-11-25 as `54234c9502`,
  `8411e13607` and `4f05735a09` (§6.3, R2). Gate A decides the override interval: a separate interval in
  the range 0.1 s to 1.0 s of RFC 9293 section 3.8.6.2.1, or the RTO with a documented reason. Test: a sender holds a
  small segment, and the peer sends data at intervals shorter than the RTO with no change of the window; the held
  segment leaves when the override timer expires. Baselines: `examples/inet/bulktransfer` `inet_inet_2b`.
  - `tcp: fix: a segment that does not change the window keeps the override timer running` (BD-21).
  - `tcp: change: the override timeout of the sender SWS avoidance <decision of gate A>` (BD-23).
- [ ] **Step 3. R3: no priority tag on an untagged port (BD-44).** Test: a protocol test in `tests/protocol/vlan`
  with `VlanReqMapper` and `forwardPcp = true`; the frame of an untagged VLAN leaves with no tag. Gate A decides
  whether `examples/ethernet/vlan`, `showcases/ethernet/sharedcore`, `tests/module/lib/vlan-access-trunk.ini` and
  `tests/module/EthernetUnknownEtherType_1.test` keep `forwardPcp = false`.
  - `linklayer.vlan: fix: VlanReqMapper asks for no 802.1Q tag when the VLAN leaves untagged`.
- [ ] **Step 4. R4: the constants of copper and aluminium (BD-49).** Test: a module test with a copper object and
  `DielectricObstacleLoss`; the factor is a number from 0 to 1 in debug mode.
  - `environment: fix: copper and aluminium have their real resistivity and a relative permittivity and permeability of 1`.
- [ ] **Step 5. R5: Tahoe and Westwood at the third duplicate ACK (BD-8, BD-39).** Test: after a loss and a go-back,
  one fast retransmit occurs in each window; ssthresh equals max(FlightSize/2, 2*SMSS) with cwnd larger than the
  receiver window. Baselines: `examples/inet/tcpclientserver` `inet-tahoe` and `inet-westwood`.
  - `tcp: fix: TcpTahoe takes ssthresh from FlightSize, as RFC 5681 equation (4) says` (BD-8).
  - `tcp: fix: TcpTahoe and TcpWestwood start one fast retransmit in each window (RFC 6582 section 6)` (BD-39).
- [ ] **Step 6. R6: RACK starts the recovery at an ACK with data (BD-35).** Test: in a two-way transfer, a loss that
  RACK finds starts the recovery at that ACK. Baselines: `examples/inet/bulktransfer` (check R1 of step 50 of the
  catch-up).
  - `tcp: fix: a loss that RACK marks at an ACK with data starts the recovery (RFC 8985 section 6.2)`.
- [ ] **Step 7. R7: a window update is no duplicate ACK (BD-4).** Test: a peer sends three ACKs with the same number
  and a larger window during a transfer; the sender does not start a fast retransmit. Baselines:
  `examples/inet/bulktransfer` `inet__inet`, `inet_inet_2a`, `inet_inet_2b`.
  - `tcp: fix: condition (e) of a duplicate ACK compares the window before its update (RFC 5681)`.
- [ ] **Step 8. R8: the ECN-Echo at a cwnd of one MSS (BD-7).** Gate A decides the form of the hold of new data.
  Test: an ECN-Echo arrives at a cwnd of one MSS; no new segment leaves before the timer expires. Baselines:
  `examples/inet/dctcp` `TcpRenoIncast`.
  - `tcp: fix: at an ECN-Echo with a cwnd of one MSS, new data waits for the retransmission timer (RFC 3168 section 6.1.2)`.
- [ ] **Step 9. R9: one count of the bytes in flight (BD-14, BD-19, BD-20).** Gate A decides one parameter or two,
  and its name (D7). Test: a timeout, then three duplicate ACKs during the go-back; the segments after the fast
  retransmit fit in the room of RFC 5681; with SACK outside a recovery, `~Tcp` does not use the count of Linux; each
  value of the parameter. Baselines: the 6 tests of step 16 of the catch-up and `inet_inet_2b`.
  - `tcp: add: a parameter for the count of the bytes in flight of Linux, set by LinuxTcp` (no result moves).
  - `tcp: change: ~Tcp counts the bytes in flight as RFC 5681 and RFC 6675 do` (results move).
- [ ] **Step 10. R10: a data segment of 1 byte gets a delayed ACK (BD-32).** Test: with delayed ACKs, a segment of 1
  byte in an open window gets a delayed ACK, and a zero-window probe still gets its ACK at once.
  - `tcp: fix: a data segment of 1 byte in an open window gets a delayed ACK (RFC 9293 SHLD-18)`.
- [ ] **Step 11. R11: the SYN is no acknowledged data (BD-15).** First check §2.4 with a run of `tcpapp`
  `BurstyTraffic`. Test: on the passive side, cwnd after the ACK of the SYN-ACK with data equals the initial window.
  Baselines: `examples/inet/tcpapp` `BurstyTraffic`, `RequestResponseTraffic`.
  - `tcp: fix: the ACK of the SYN-ACK does not grow cwnd by the SYN`.
- [ ] **Step 12. R12: the compound queue counts the packets that enter (BD-24).** First check the cause of §2.4 with a
  run of `TraceFile-Udp`. If the run does not confirm it, the entry leaves this plan by a revision
  ([PLR-REVISION](../../doc/project/rule/planning.md#plr-revision)). Test: a queueing test of a compound queue whose
  `in` is connected to a submodule; `queueLength` is the count of the packets in it. Baselines: `netperfmeter`
  `TraceFile-Tcp`, `TraceFile-Sctp`, `TraceFile-Udp`, and the users of `RedDropperQueue`.
  - `queueing: fix: a compound queue counts the packets that enter through its input gate`.
- [ ] **Step 13. R13: one recorder for each QUIC signal name (BD-41).** Test: the result file of
  `examples/quic/dplpmtud` `DecreasingPmtuWithoutPtb` has one vector of each name. Check also whether the comparison of
  opp_repl has the problem of duplicate rows, and record the answer.
  - `quic: fix: a statistic entry adds its result recorders once for each signal name`.
- [ ] **Step 14. R14: the SYN-ACK of Linux for a repeated SYN (BD-17).** Test: a repeated SYN in SYN_RCVD gets an ACK
  from `~Tcp` and a SYN-ACK from `~LinuxTcp`. Baselines: `examples/bgpv4/BgpAndOspfv3`,
  `examples/manetrouting/multiradio` `MultiRadio`.
  - `tcp: add: a parameter for the SYN-ACK of Linux for a repeated SYN, set by LinuxTcp` (no result moves).
  - `tcp: change: ~Tcp answers a repeated SYN in SYN_RCVD with an ACK (RFC 9293)` (results move).
- [ ] **Step 15. R15: the slow-start limit of Linux (BD-18).** Test: a sender that the application limits, in slow
  start, for each value of the parameter. Baselines: the 42 tests of step 18 of the catch-up and `DcTcpIncast`.
  - `tcp: add: a parameter for the slow-start limit of Linux tcp_is_cwnd_limited(), set by LinuxTcp` (no result moves).
  - `tcp: change: the slow start of ~Tcp grows cwnd with no limit of Linux (RFC 5681)` (results move).
- [ ] **Step 16. R16: the initial cwnd and ssthresh are recorded (BD-3).** Test: the first samples of the two vectors
  have the time of the establishment. Baselines: many statistical tests get new rows; the record lists them.
  - `tcp: fix: a connection records its initial cwnd and ssthresh`.
- [ ] **Step 17. R17: the `unacked` vector during SACK recovery (BD-10).** Test: the vector changes at each send of
  the recovery. Baselines: `examples/inet/bulktransfer` `inet_inet_2b`.
  - `tcp: fix: the sends of the RFC 6675 recovery record unacked`.
- [ ] **Step 18. R18: a random stream for each client of `shutdownrestart` (BD-36).** Check: the experiment of step 52
  of the catch-up again. Baselines: the tests of `examples/inet/shutdownrestart`, re-recorded once.
  - `examples: fix: each TelnetApp of shutdownrestart draws from its own random stream`.
- [ ] **Step 19. R19: the deprecated alias parameters go (BD-34).** A Sonnet subagent changes each use to
  `initialWindow = "rfc3390"` and `tcpEcnMode = "rfc3168"`; the assistant checks each file. Check: no use of the old
  names remains, and the fingerprints do not change.
  - `tests: change: the tests and examples use initialWindow and tcpEcnMode` (no result moves).
  - `tcp: remove: the parameters increasedIWEnabled and ecnWillingness` (D-120 of 2026-10-15).
- [ ] **Step 20. R20: the lower limit of the ssthresh after a timeout (BD-6).** The first attempt, the commit
  `74882daa4e`, is the patch `patches/0020-*.patch` of the folder of D10. It moves 6 fingerprint runs (§6.3, R1). The cycle of §6.1 explains each moved value, writes a protocol test of the
  ssthresh after a timeout with the TS option on each segment, and re-records the fingerprints and the statistics.
  - `tcp: change: the ssthresh after a timeout takes the lower limit 2*SMSS with the effective MSS (RFC 5681)`.
- [ ] **Step 21. R21: step (2) of RFC 6675 at SND.UNA (BD-9).** The first attempt, the commit `38958b1fb8`, is the
  patch `patches/0021-*.patch` of the folder of D10. It moves the run 0 of
  `examples/inet/shutdownrestart` `TCP`, whose Telnet segments are 1 byte long. The cycle of §6.1 explains the move,
  writes a protocol test with a segment of 1 byte at SND.UNA, and re-records.
  - `tcp: change: step (2) of RFC 6675 tests IsLost(HighACK + 1) at SND.UNA`.
- [ ] **Step 22. Close.** Write the summary of the changes and the closed entries at the top of this plan. Move it to
  `plan/done/`.
  - **C(last): `plan: location: move the plan of the defect repairs of the baseline catch-up to done`.**

Each commit of steps 1 to 19 carries `Plan: plan/pending/baseline-catch-up-defect-repairs.md` and a `Change:` trailer
of [classification.md](../../doc/project/rule/classification.md). The subjects above are proposals; step 3 of §6.1
fixes them.

### 6.3 The prepared commit entries

Step 3 of §6.1 writes the entries of each change here before its gate A.

#### R1: TCP comments and code without effect (BD-5, BD-13, BD-16, BD-6, BD-9), prepared 2026-11-24

**Re-check.** No commit from `bc39ec7571` to master `d8a58564ee` changes `src/inet/transportlayer/tcp`. The five
places are as §2.2 says.

**The fault.** No test can fail for BD-5, BD-13 and BD-16, because they are comments. BD-6 and BD-9 give the same
results as their repairs, by the text of the entries. So step 2 of §6.1 has no failing test for R1. The check is that
no fingerprint and no module test changes.

**A risk found in the preparation.** BD-6 changes the lower limit of the ssthresh after a timeout from `2 * snd_mss` to
`2 * snd_effmss`. Since `5118cd85ec` a connection with timestamps sends the TS option on each segment, so `snd_effmss`
is 12 bytes less than `snd_mss` there. RFC 5681 section 2 defines SMSS without the TCP options, so `snd_effmss` is the
correct value, but the limit binds only when FlightSize/2 is below 2*SMSS, and then results can move. If a fingerprint
moves at C4, BD-6 leaves R1 and becomes a change of its own with its re-record, by a revision of this plan.

**Commits.** [TR-PLAN-PROTOCOL](../../doc/project/rule/testing.md#tr-plan-protocol) does not apply: no commit changes
a protocol behavior, and C4 leaves R1 if it does.

- **C3: `tcp: comment: each comment is above its function, and no comment describes a removed function`** (BD-5,
  BD-13, BD-16). `TcpConnection.h`: the comment "Utility: update window information (snd_wnd, snd_wl1, snd_wl2)"
  moves from above the comment of `getFlightSize()` to above `updateWndInfo()`. `TcpReno.h`: delete the 4 lines of the
  comment of the removed `isInFastRecovery()` above `public:`. `Rfc5681CongestionControl.cc:100`: the comment says that
  a timeout sets cwnd to 1 SMSS (RFC 5681 section 3.1) and the reaction to an ECN-Echo can leave it at 1 SMSS, so the
  division needs no guard against a small cwnd, only against 0. `Change: src.transportlayer.tcp | comment | - |
  baseline-catch-up-defect-repairs`.
- **C4: `tcp: refactor: the ssthresh after a timeout comes from calculateSsthresh() with the effective MSS`** (BD-6).
  `TcpClassicAlgorithmBase::calculateSsthreshForRto()` returns `calculateSsthresh(state->snd_max - state->snd_una)`,
  as `TcpCubic` does with its own count. `Change: src.transportlayer.tcp | refactor | - |
  baseline-catch-up-defect-repairs`.
- **C5: `tcp: refactor: step (2) of RFC 6675 tests IsLost(HighACK + 1) at SND.UNA`** (BD-9).
  `Rfc6675Recovery.cc:332` tests `isLost(state->snd_una)`, and its comment names HighACK + 1 as SND.UNA. `Change:
  src.transportlayer.tcp | refactor | - | baseline-catch-up-defect-repairs`.

**Checks.** Build debug and release. Run the module tests of the places in debug mode: `tcp_rto_cwnd_1`,
`tcp_fastrexmit_rto_1`, `tcp_algorithm_reno`, `tcp_algorithm_newreno`, `tcp_sack_1` to `tcp_sack_8`,
`tcp_sack_old_ack_1` and `tcp_limited_transmit_sack_1`. Run all 5784 stored fingerprints in release mode once at the
base and once after C5, as two task groups of the editor; both must pass. A run of the fingerprints after each commit
is not necessary: if the run after C5 moves, the runs of C3 and C4 alone find the commit.

**Gate A** approved by the maintainer on 2026-11-24 ("yes to both": the design, and the two fingerprint runs).

**Result of the checks (2026-11-24).** The commits are C3 `6f3eb8a25c`, C4 `74882daa4e` and C5 `38958b1fb8`.

- The 14 module tests pass in debug mode.
- The fingerprints in the editor, release mode, default features of the worktree: the base gives 1776 PASS, 1 expected
  ERROR and 70 ERROR; after C5, 1769 PASS, 7 FAIL, 1 expected ERROR and the same 70 ERROR. The 70 runs need the
  features `TcpLwip` and `VoipStream`, which the worktree did not enable and the CI enables; they can not start here.
  The 1847 runs check the 5784 stored fingerprints.
- Two runs of the 11 configurations of the 7 failures find the cause. Without C4, only `examples/inet/shutdownrestart`
  `TCP` run 0 fails. Without C5, the other 6 fail: `examples/inet/bulktransfer` `inet__inet`, `inet_inet_2a` and
  `inet_inet_2b`, `examples/inet/nclients/filetransfer.ini` `Filetransfer` run 0, and `examples/inet/tcpclientserver`
  `inet-newreno` and `inet-reno` run 6. So C4 (BD-6) moves 6 runs and C5 (BD-9) moves 1 run. C3 moves none.
- BD-6 moves results as the risk of the preparation said. BD-9 moves results in the case that the message of C5 names:
  `shutdownrestart` sends Telnet keystrokes, segments of 1 byte, and then `isLost(snd_una + 1)` and `isLost(snd_una)`
  count different SACKed bytes. The entry of BD-9 said that the results are the same; that is wrong for segments of 1
  byte.
- A stop of the task of `tools/prun.sh` stopped the client of podman, but not its container; `podman stop` stopped it.

**Revision ([PLR-REVISION](../../doc/project/rule/planning.md#plr-revision)), approved by the maintainer on 2026-11-24** ("yes, run only release fingerprint tests, they are faster and debug tests is enough when landing").
R1 keeps only C3 (BD-5, BD-13, BD-16) and moves no result. BD-6 and BD-9 leave R1 and become the changes R20 and R21,
each with the whole cycle of §6.1: the attribution of each moved fingerprint and statistic to the mechanism, the
re-record in the image of the CI, and a protocol test and its reviewer document, because each changes a protocol
behavior. C4 and C5 left the branch on the same day, so that R1 lands alone; they are the patches `patches/0020-*.patch` and
`patches/0021-*.patch` of the folder of D10, the first attempts of R20 and R21.

**The check of the gate (2026-11-24).** In the image of the CI, with all features, release mode, at C3
`6f3eb8a25c`: 1847 runs, 1846 PASS and 1 ERROR that a marker declares as expected (`tests/fingerprint`
`ethernet-nonstandardspeed.ini`, 5 Gbit/s half duplex), in 2 min 24 s. The log is `runs/r1-c3/` of the folder of
D10. The debug mode of the job copy needs a debug build of OMNeT++ first (`MODE=debug tools/prun.sh
build-omnetpp`); it runs at the landing.

**The mechanical gates (2026-11-24)**, at C3 in the job copy, with the base `d8a58564ee`: `check-pull-request.sh`,
`check-source-seals.sh` and `check-architecture.sh src/inet/transportlayer/tcp` pass. `check-naming.sh` reports 24
violations and `check-interfaces.sh` 16 classes; master `d8a58564ee` gives the same 24 and the same 16, all of the whole
repository (folder names such as `tcp_common` and `flavours`, icon files, `site.yml`). The part of `check-naming.sh`
for the changed files passes. So R1 adds no violation. R1 is no protocol change, so it has no protocol test and no
reviewer document ([TR-PLAN-PROTOCOL](../../doc/project/rule/testing.md#tr-plan-protocol)), and it moves no
baseline.

**Gate B** approved by the maintainer on 2026-11-24 ("yes"). The debug fingerprints of the landing run in the image
of the CI at `4e65771abb`, whose sources equal C3. Result: 1847 runs, 1846 PASS and 1 expected ERROR, in 12 min 28 s
(`runs/r1-landing/` of the folder of D10). R1 is ready to land.

**Landing (2026-11-24).** The maintainer said: "land it and continue". Master had moved to `ad34cc37dc` with 25
commits of other work (IPv6 Neighbor Discovery, 802.11 Block Ack, BD-31), none in `src/inet/transportlayer/tcp`. The
branch was rebased onto it with no conflict. C3 is the same patch, and it changes no compiled token, so the
fingerprint results of C3 hold on the new base. `check-pull-request.sh` passes on the rebased series. Master moves
to the head of the branch by a fast-forward.

#### R2: the override timer of the sender SWS avoidance (BD-21, BD-23), prepared 2026-11-24

**Re-check.** Master `08e3648c36` changes no TCP file after `d8a58564ee`. `TcpAlgorithmBase::processWindowUpdate()`
(`TcpAlgorithmBase.cc:1006-1026`) still treats each segment with SEG.ACK = SND.UNA, an open window and nothing in
flight as a window update: it cancels the persist timer, and `sendData()` starts it again with a full RTO
(`:724-729`, `startPersistTimer()` at `:751-757`). The persist timer has two roles: the zero-window probe of RFC 9293
section 3.8.6.1, and the override timeout of rule (4) of section 3.8.6.2.1 (`processPersistTimer()`, `:503-513`).

**The fault, shown.** The new module test `tests/module/tcp_sws_4.test` has the setup of `tcp_sws_2.test`, and the
server also sends 100 bytes every 0.3 s with the same window of 500 bytes. In debug mode on `08e3648c36` it fails:
the client holds 500 bytes from 1.207 s, each segment of the server cancels and starts the timer again ("The window
update opens the window: cancelling the PERSIST timer"), and the 500 bytes leave only at 5.053 s, one RTO after the
last segment of the server at 4.051 s. `tcp_sws_2.test` passes as the control. A server that never stops sending
holds the data for ever, as in `examples/inet/bulktransfer` `inet_inet_2b` (BD-21).

**Design questions for gate A.**

1. *How the override timer survives a segment that updates no window (BD-21).* (A) The persist timer gets its role
   when it starts: the override role when the window is open, the probe role when it is zero. `processWindowUpdate()`
   cancels the timer only in the probe role; in the override role it lets `sendData()` decide, and stops the timer
   when the held data leaves. (B) The connection keeps the window of the segment before this one, and
   `processWindowUpdate()` acts only when the window changed; R7 (BD-4) needs the same value. The assistant
   recommends (A): it stays in the algorithm and changes no processing of the connection; R7 makes its own choice.
2. *The override interval (BD-23).* RFC 9293: "The override timeout should be in the range 0.1 - 1.0 seconds." Now it
   is the RTO, at least 1 s in `~Tcp` and growing with the RTO. (i) The RTO, held in the range 0.1 s to 1.0 s, in
   both `~Tcp` and `~LinuxTcp`. (ii) The same in `~Tcp`, and a new parameter that keeps the RTO, set by `~LinuxTcp`,
   because Linux sends such a segment from its probe timer, whose interval is the RTO (`tcp_probe_timer()`), D7.
   The assistant recommends (ii), by rule 6 of [baseline-catch-up-defects.md](baseline-catch-up-defects.md) §2.

**Commits** (with the recommended answers).

- **C6: `tcp: fix: a segment that updates no window keeps the override timer of the SWS avoidance running`** (BD-21).
  `TcpAlgorithmBase.cc`, and a state field for the role of the persist timer. Protocol gap: RFC 9293 section
  3.8.6.2.1, rule (4), asks for a timeout that forces the transmission; the restart at each segment of the peer
  prevents it. Tests: `tests/module/tcp_sws_4.test` (new, fails before C6), and the new protocol test
  `tests/protocol/tcp/rfc/Rfc9293SwsOverrideTimeout.test` with three cases: a hold with no traffic of the peer (the
  override expires), a hold while the peer sends data with the same window (the override expires at the same time),
  and a window update that opens the window to a full segment (the held data leaves at once). Expected before C6:
  the second case fails, as `tcp_sws_4.test` does. Reviewer document: `tests/protocol/tcp/sws-override-timer.md`.
- **C7: `tcp: add: a parameter for the override timeout of Linux, the RTO, set by LinuxTcp`** (BD-23). `Tcp.ned`,
  `LinuxTcp.ned`, the read of the parameter; no result moves, because `~Tcp` still uses the RTO here.
- **C8: `tcp: change: the override timeout of the sender SWS avoidance is the RTO held in 0.1 s to 1.0 s (RFC 9293)`**
  (BD-23). `startPersistTimer()` in the override role. The protocol test gets a fourth case: with an RTO above 1 s,
  the override comes after 1.0 s in `~Tcp` and after the RTO in `~LinuxTcp`. Whatsnew entries for C6 and C8.

**Expected baselines.** `examples/inet/bulktransfer` `inet_inet_2b`, whose flow from the server to client1 stops at
11.27 s on master, and each TCP run with held data and peer traffic (C6) or with an RTO above 1 s while data is held
(C8). The runs of the gate find them. Each moved value gets its line of attribution, and the re-record runs in the
image of the CI (D6).

**Checks.** The module tests `tcp_sws_1` to `tcp_sws_4` and `tcp_persist_*`, the new protocol test and
`tests/protocol/tcp/parameter/send/TcpMaxPersistTimeout*.test`, in debug mode; all fingerprints in release mode in the
editor; the statistical tests of the moved folders in the image of the CI.

**Gate A** approved by the maintainer on 2026-11-24 ("both agreed"): (A) and (ii).

**State (2026-11-24).** C6 is written, not committed: the field `persistOverride` of `TcpAlgorithmBaseState.msg`,
set by `startPersistTimer()`, cleared when the timer becomes a zero-window probe; `processWindowUpdate()` cancels only
a probe timer, and stops an override timer when the held data left. The 12 module tests `tcp_sws_1` to `tcp_sws_4`,
`tcp_persist_*`, `tcp_window_update_read_1` and `tcp_wnd_update_data_1` pass in debug mode; `tcp_sws_4` failed before
C6. The three protocol tests `Rfc9293SwsOverride`, `Rfc9293SwsOverridePeerData` and `Rfc9293SwsOverrideWindowUpdate`
are written, but the editor can not run protocol tests on this branch: `inet.opp` of master names the defaults of the
protocol runner `suite_folders`, `lib_folder` and `lib_name`, and the runner of omnet-julia takes `suite_per_subfolder`,
`library_folder` and `library_name`. The commit `4d3705af84` of `inet-julia` renames them; it is not on master, and
the editor reads the description of a project only when it opens the project. The worktree has all features except
the two OSG features, because the OMNeT++ of this machine has no OSG.

**The runner of the protocol tests (2026-11-24).** The keys of master are right: they are the keywords of
`get_opp_test_tasks` of `opp_repl` (`opp_repl/test/opp.py:212`), which the workflow "Test: protocol" reads, and the
message of `4d3705af84` was wrong in that point. The maintainer approved a repair of the editor in place of a change of
INET ("yes"): omnet-julia `0d35ce9a6`, on `main` and pushed on 2026-11-24, translates the three names of `opp_repl` in
`get_kind_opp_test_tasks`, with a test of a description as INET master writes it. Its checks, the smaller set that the
maintainer chose ("the smaller set"): `OmnetLegacyTest test_legacy` and `OmnetIdeTest test_ide`, 160 parts, 6242 pass,
0 fail, 0 error, 134 marked broken. `4d3705af84` left `inet-julia`, and its three plan commits above it were written
again. The running editor needs a restart to load `0d35ce9a6`; then the protocol tests of R2 run in it.

**The protocol tests and C6 (2026-11-25).** After the restart, the editor runs the protocol tests of the branch. The
first run, on the code of master, found three faults of the tests, not of master:

- The TS option takes 12 bytes of each segment, so a full segment holds 988 bytes, and a window of 3500 bytes left
  536 bytes after three segments, not 500. The window is now 3464 bytes: 3 × 988 + 500.
- The steps of a `ProtocolTest` run in order, and an `act` step waits for its time. So in
  `Rfc9293SwsOverridePeerData` the check of the window waited for the last send of host2 at 2.45 s, and the test
  passed on master. The guard "no held data" now runs with `meanwhile(never(...))`, host2 sends at 0.35 s, 0.65 s and
  0.95 s after the check of the window, and the held data must leave before 1.3 s. With the fault, it leaves at
  about 1.95 s, one RTO after the last segment of host2.
- `windowUpdateAfterRead` is `false` by default, so the read of host2 in `Rfc9293SwsOverrideWindowUpdate` sent no
  window update. The test sets it, and its read comes after the checks of the hold.

The protocol test of C6 above is three files, not one file with three cases. On the code of master in
debug mode, `Rfc9293SwsOverridePeerData` fails at the held data and the other two pass; with C6, all three pass. The
40 protocol tests of the send parameters, of `TcpWindow*` of the receive parameters, of the RFC 9293 tests of the
window and of Nagle give 34 PASS and 6 that can not run here: the Nagle tests of `linux/` and `packetdrill/` need
`inet_run_packetdrill` on the `PATH`, which the runner of the editor does not set, and then `inet-gpl`, which this
machine does not have, so they skip. C6 was committed then; its reviewer document and its whatsnew entry came into
it by fixups after the runs of the gate (§6.1, step 6), and the series below gives its final hash.

**C7 and C8 (2026-11-25).** The parameter is `swsOverrideAfterRto`, the name of gate A: `default(false)` in
`Tcp.ned` under the Linux behaviors of "sending data", with an item in the header text, and `default(true)` in
`LinuxTcp.ned` (D7). `LegacyTcp` sets no value, because it sets only the old values of parameters that existed
before. `TcpAlgorithm::initializeAlgorithm()` reads it. Facts and decisions of the implementation:

- The catalog check (`tests/protocol/tcp/parameter/check_catalog.py`) stops a default of a preset that no row names,
  so C7 needs a row and a parameter test of the class of `LinuxTcp`. C7 adds the class `linux`
  (`TcpSwsOverrideAfterRtoLinux.test`, an RTO of 2 s: the data leaves after 2 s), which passes before and after C8.
  C8 adds the classes `rfc-upper` (an RTO of 2 s: the data leaves after 1.0 s) and `rfc-lower` (an RTO of 10 ms: the
  data leaves after 0.1 s). These two parameter tests are the fourth case of the protocol test of C8 above; with the
  code of C7 both fail, with C8 both pass. The catalog has 117 parameters and 342 rows.
- In C7 the value `false` still gives the RTO, so the NED comment of C7 describes the behavior that C8 gives it. The
  message of C7 says so.
- C8 changes only `startPersistTimer()`: in the override role and with `false`, the interval is the RTO held in
  0.1 s to 1.0 s, and then at most `maxPersistTimeout`. The interval of a zero-window probe does not change. With
  the defaults of `~Tcp` the RTO is at least 1 s, so the override is 1.0 s; a result moves only where data is held
  while the RTO is above 1 s.
- Each commit carries its own whatsnew entry, as [doc/whatsnew/README.md](../../doc/whatsnew/README.md) asks
  (TR-SHIP-WITH): C6 `fixes/tcp-sws-override-peer-segments.txt`, C8
  `incompatible/tcp-sws-override-interval.txt`, which also names the parameter of C7.
- Debug mode, with C8: the 11 protocol tests of the override, of the parameter, of `TcpMaxPersistTimeout*` and of
  `TcpZeroWindowProbe*` pass, and the 11 module tests `tcp_sws_*`, `tcp_persist_*`, `tcp_window_update_read_1` and
  `tcp_wnd_update_data_1` pass.
- A start of a test verb of the editor, such as `run_protocol_tests!`, opens the page of its group in a tab, although
  the assistant guide says that a start opens no tab: `evaluate_operation(editor, ::StartLegacyTasksOperation)` of
  omnet-julia opens it for a press of Run, and the verbs use the same operation. The assistant closes such a tab after
  each start (the maintainer, 2026-11-25: "it's enough if the runs appear in the plan specific explorer").
- The first run of all fingerprints at C8 gave 1836 PASS and no FAIL, but it did not check the repair: the step
  "building inet" of `run_fingerprint_tests!` reported DONE and built nothing in release mode, so the runs used
  `src/libINET.so` of 2026-11-24 17:26, and `out/clang-release` did not exist. A gate therefore builds the release mode
  first with `build_simulation_project!(project; mode = "release")`, checks that `src/libINET.so` holds the change, and
  starts the fingerprints with `build = false`. **Correction (2026-11-25):** the step built the debug mode, which the
  runs use, so it built correctly; the release library was not what the runs loaded (§6.1, step 8). omnet-julia now
  names the mode in the step: "building inet in debug mode".

**The review of R2 against the RFC (2026-11-25, D9).** The `simulation-domain-expert` agent read C6 to C8. Its
findings:

- Defect, repaired: when the window closes while the override timer runs, the timer kept what was left of the
  override, so the first probe came early, against SHLD-29 ("send the first zero-window probe when a zero window has
  existed for the retransmission timeout period"); with C8 the clamped interval also became the base of the probe
  backoff. C6 now adds `restartOverrideTimerAsProbe()` too: at a zero window, an override timer
  starts again in the probe role. The new protocol test `Rfc9293SwsOverrideZeroWindow.test` shrinks the window of host2
  to 0 at 0.5 s with a tap; before the fix the first probe came at 1.2 s, with it at 2.5 s, one RTO later. With the fix,
  the 14 protocol tests of the override, of the parameter, of `TcpMaxPersistTimeout*`, of `TcpZeroWindowProbe*` and of
  `Rfc9293ShrunkWindow*` and the 11 module tests pass in debug mode.
- Correct: the start, the keep and the stop of the override timer; the reading of "0.1 - 1.0 seconds"; the interval of
  `~LinuxTcp`; no interaction with Nagle, the delayed ACK or the loss recovery.
- Limit, recorded in the reviewer document: by the memory of the reviewer, Linux restarts its probe timer at a pure ACK
  that leaves the head of the queue outside the window (`tcp_ack_probe()`), but not at the in-order data segment of
  BD-21; `~LinuxTcp` never restarts it.
- Two older faults, not part of R2, for the maintainer to decide: (1) data that TCP_CORK holds below one segment, with
  a window below it, waits as long as the cork stays set, because the expiry of the override does not bypass the hold
  of the cork, and the cork timer starts again; Linux `tcp_write_wakeup()` ignores the cork. (2) A retransmission
  timer that runs with nothing in flight (the ECN case of `processRexmitTimer()`) blocks the start of the override
  timer, so held data can wait one RTO more.

**The author of the commits (2026-11-25).** The maintainer: "please use levy@omnetpp.org author for all inet-cpp git
commits now and also in the future. I don't want the projectured user to be used there". Each INET repository of the
workspace, with its `statistics` and `media` clones, has the local `user.name` "Levente Meszaros" and `user.email`
`levy@omnetpp.org`, the form of the commits of levy on master. The ten unpushed commits of the topic branch were
made again with `git commit-tree`, with the same trees, messages and dates, so the worktree did not change during the
fingerprint run.

**The results of R2 (2026-11-25).** No recorded result moves, so R2 re-records nothing.

- The release library holds C6 to C8 (`build_simulation_project!` in release mode, then a check of `src/libINET.so`).
  All 1847 stored fingerprint runs in the editor, with all features but OSG: 1836 PASS, the 1 ERROR that a marker
  declares, and the 10 runs of `showcases/visualizer/osg` that a build without OSG can not start. These runs were in
  **debug** mode, not release (§6.1, step 8, correction). The release fingerprints of R2 ran after the landing, on
  the sources of master `7f7f77a00f` in the image of the CI: 1847 runs, 1846 PASS and 1 expected ERROR, in 1 min 41 s
  (`runs/r2-landed-release/` of the folder of D10).
- All statistical tests in the image of the CI (`tools/prun.sh` of the job copy, at the head of the series, against
  the pins of master `statistics` `76d45e4e68` and `media` `2ab7ae005b`, which the job copy fetched for this run):
  945 TOTAL, 934 PASS, 11 SKIP (expected), 0 unexpected; the logs are `evidence/runs/28413967e9/` of the job copy.
- The entry BD-21 found the fault in `examples/inet/bulktransfer` `inet_inet_2b` at step 30 of the catch-up, with
  3223 starts of the persist timer. A debug run of that configuration at the head of the series starts the persist
  timer 0 times: master no longer holds data there, and the series touches no condition of a hold. The "Expected
  baselines" of the preparation above were therefore wrong; the protocol tests are the only check of the case.
- The fixups are squashed. The series of R2 is C6 `0cb5166482`, the plan commit `5627d82b0b`, C7 `7e0fd12257` and C8
  `31c275f882`. The trailers of C6 and C8 name the obligations `test whatsnew`, and their messages say that no
  stored fingerprint changes. The reviewer document is `tests/protocol/tcp/sws-override-timer.md`.
- The gates of "Before every push", with the base `master` `08e3648c36`: `check-architecture.sh
  src/inet/transportlayer/tcp`, `check-pull-request.sh`, `check-source-seals.sh` and `check-baseline-attribution.sh
  plan` ("No commit of the series moves a pin over a result or a fingerprint value") pass. `check-naming.sh` reports 24
  violations and `check-interfaces.sh` 16 classes; a worktree of `master` gives the same 24 and 16, all of the whole
  repository, so R2 adds none. `whatsnew.py check` and `check_catalog.py` pass, and no link of the new files is broken.

**Gate B** approved by the maintainer on 2026-11-25 ("yes to all of your questions"). The same answer decides two
more things:

- The two older faults of the review become the entries BD-51 (TCP_CORK) and BD-52 (a retransmission timer with
  nothing in flight) of [baseline-catch-up-defects.md](baseline-catch-up-defects.md). Both are new since v4.7.0, so
  they meet the criterion of this plan, but they stay outside its scope until a revision
  ([PLR-REVISION](../../doc/project/rule/planning.md#plr-revision)) takes them.
- The two faults of omnet-julia above are repaired in a separate change of omnet-julia: a start of a test verb opens
  a tab, and the build step of `run_fingerprint_tests!` builds nothing in release mode.

The repair commits of BD-21 and BD-23 are in the defect list. The gate B of R1 left out the repair commit of BD-5,
BD-13 and BD-16 (`519e772c16`); it is in the list now too. Next: the debug fingerprints of the landing in the image
of the CI, then the landing.

**The debug fingerprints of the landing (2026-11-25).** Master had moved to `b029add207` with 30 commits of other
work. None of them changes `src/inet/transportlayer/tcp`, but they re-record 36 values of
`tests/fingerprint/store.json`. So the check ran on the series rebased onto that master, as a detached head
`53ea79b3fd` of the job copy of the CI, with no conflict; the topic branch did not change. In debug mode with all
features: 1847 runs, 1846 PASS and 1 ERROR that a marker declares as expected, in 7 min 10 s (`runs/r2-landing/` of
the folder of D10). R2 is ready to land.

**Landing (2026-11-25).** The maintainer said: "land R2, master moved". Master had moved to `d88ae6c573` with 4
commits of IEEE 802.11 work (BD-27 and BD-43), none in `src/inet/transportlayer/tcp`. They re-record 42 values of
`tests/fingerprint/store.json`, move the pins of `statistics` and `media`, and change §3 and BD-27 of
[baseline-catch-up-defects.md](baseline-catch-up-defects.md). The branch was rebased onto it with no conflict: C6 is
`54234c9502`, C7 `8411e13607` and C8 `4f05735a09`. `check-pull-request.sh --base origin/master` (0 violations) and
`check-baseline-attribution.sh plan` pass on the rebased series. The debug fingerprints ran again on its head in the
image of the CI, because the new values of IEEE 802.11 can reach TCP: 1847 runs, 1846 PASS and 1 expected ERROR, in
11 min 48 s (`runs/r2-landing-2/` of the folder of D10). Master moves to the head of the branch by a fast-forward. The
plan commits of R3 to R5 land with it; their tests are not committed yet.

#### R3: no priority tag on an untagged port (BD-44), prepared 2026-11-25

**Re-check.** Master `b029add207` (2026-11-25) changes no file of `src/inet/linklayer/vlan`, of the 802.1Q layer, of
`PacketDirectionReverser` or of the four configurations of step 3 after `08e3648c36`. `VlanReqMapper::processPacket()`
(`VlanReqMapper.cc:44-63`) still removes the `VlanReq` when the map of the port sends the VLAN untagged (-1), and then
requests the 802.1Q encapsulation when a `PcpReq` is present (`:61`). `PacketDirectionReverser` forwards the PCP by
default. `Ieee8021adPolicy` (from `6ccb36a0b8`) removes the `PcpReq` and the `DropEligibleReq` on an untagged port and
requests no encapsulation (`Ieee8021adPolicy.cc:286-297`). `VlanPolicyLayer` still uses `VlanReqMapper` by default
(`VlanPolicyLayer.ned:53`). Since `835ec66add`, the provider bridging example uses `Ieee8021adPolicy` and sets no
`forwardPcp`, so the shared core showcase (`FiltersAndMappers`) is the only example with the case.

**The fault, shown.** The new protocol test `tests/protocol/vlan/Ieee8021qVlanReqMapperUntaggedNoPriorityTag.test` is
`Ieee8021qUntaggedSetNoPriorityTag.test` with the default policy `VlanPolicyLayer` on the bridge in place of
`Ieee8021adPolicy`. In debug mode on the branch, whose VLAN code is that of master, it fails: b sends the frame of
VLAN 10 on the access port with a C-tag, and a1, which has no 802.1Q layer, stops with "Unknown protocol: protocolId =
32, protocolName = ieee8021qctag" at 0.10002538 s. The other four tests of `tests/protocol/vlan` pass.

**Design questions for gate A.** Revised on 2026-11-25, after the maintainer asked for the more correct, more generic,
more standard-conformant and more lasting form.

INET keeps two kinds of priority on a frame. `UserPriorityReq` is the priority of the frame, the priority parameter of
the EISS (IEEE 802.1Q-2018 6.9): the tag checker sets the user priority from the PCP of a received tag
(`Ieee8021qTagEpdHeaderChecker.cc:52`, 6.9.1 h), and the direction reverser forwards it always, whatever `forwardPcp`
says (`PacketDirectionReverser.cc:99-100`). `PcpReq` and `DropEligibleReq` of a tag type ask for the fields of the
tag of that type: the inserter takes the PCP from `PcpReq`, and else from the user priority
(`mapUserPriorityReqToPcpReq()`), and removes the request when it writes the tag. So `forwardPcp = true` only copies
the PCP of the received tag as a request for the same tag at the egress.

1. *The form of the repair.* (A) `VlanReqMapper` requests no 802.1Q encapsulation when its map sends the VLAN of the
   frame untagged, and keeps the `PcpReq`. (B) It requests no encapsulation and removes all the requests of that tag
   type: the `VlanReq`, the `PcpReq` and the `DropEligibleReq`, as `Ieee8021adPolicy::processSentPacket()` does
   (`Ieee8021adPolicy.cc:286-297`). The assistant recommends (B), in place of its first answer (A):
   - It is more correct: a frame that leaves untagged has no tag, so a request for the fields of that tag has no
     object. The priority of the frame stays as `UserPriorityReq`, the parameter that 6.9.2 passes down unchanged.
   - It is more generic: one rule for an untagged egress, "no tag, and no request for that tag", in both VLAN
     policies of INET, for the C-tag and the S-tag.
   - It lasts: a later module that reads a `PcpReq` finds one only on a frame that carries the tag, so it can not
     take it for a tag that exists.
   - The first answer (A) rested on the wrong reading that the `PcpReq` is the only carrier of the priority.
   `PcpTrafficClassClassifier` reads only `PcpReq` and `PcpInd`, not `UserPriorityReq`, so on an untagged egress it
   gives no class from the priority, with (B) as today with `forwardPcp = false`. Whether the classifier must fall
   back to the user priority (8.6.6) is outside R3.
2. *The four configurations with `forwardPcp = false`:* `examples/ethernet/vlan/omnetpp.ini:44`,
   `showcases/ethernet/sharedcore/omnetpp.ini:124-128` (with a comment that describes the fault),
   `tests/module/lib/vlan-access-trunk.ini:55` and `tests/module/EthernetUnknownEtherType_1.test:209`. Each one exists
   only to avoid BD-44. (i) All four keep the setting, and only the comment of the showcase changes. (ii) The setting
   goes from all four, so every one runs the default, which the repair makes correct. The assistant recommends (ii),
   in place of its first answer, which kept the two module tests: without the setting, the module tests of
   `vlan-access-trunk.ini`, whose hosts have no 802.1Q layer behind access ports, check the repair too, and no
   setting hides a later return of the fault. A fifth setting stays: `forwardPcp = false` in
   `tests/module/Ieee8021qTagEpd_stacked_2.test:162` is no workaround; that test checks that forwarded tags take their
   PCP from the user priority.

**Commits** (with the recommended answers).

- **C9: `linklayer.vlan: fix: VlanReqMapper sends a frame untagged, with no request for the tag, when its VLAN leaves
  untagged`** (BD-44). `VlanReqMapper.cc`, `processPacket()`. The items of
  [TR-PLAN-PROTOCOL](../../doc/project/rule/testing.md#tr-plan-protocol):
  1. Requirement: IEEE 802.1Q-2018 8.8.2 (the untagged set), with 8.6.4 and 6.9.1 h and 6.9.2. Bug: a priority tag
     (VLAN ID 0) on a port whose untagged set holds the VLAN of the frame. Evidence: the test above, and the
     experiments E1 of steps 78 and 79 of the catch-up.
  2. Existing tests: `Ieee8021qUntaggedSetNoPriorityTag.test` covers `Ieee8021adPolicy` only. No test covers
     `VlanReqMapper` with a PCP request on an untagged port: the module tests of `vlan-access-trunk.ini` set
     `forwardPcp = false`.
  3. New: `tests/protocol/vlan/Ieee8021qVlanReqMapperUntaggedNoPriorityTag.test`, one case.
  4. Initial state: bridge b with `VlanPolicyLayer`, eth0 an access port of VLAN 10, eth1 a trunk of VLAN 10,
     `forwardPcp` at its default `true`. Stimulus: t sends one UDP datagram with a C-tag of VLAN ID 10 and PCP 5 at
     0.1 s. Production path: `b.eth[1]`, the relay, the direction reverser (a `PcpReq` of 5), `VlanReqMapper` (the map
     10 to -1), `b.eth[0]`. Observation: `packetSentToLower` of `b.eth[0]` and `packetReceivedFromLower` of `a1.udp`.
     Assertion: the frame on eth0 has the IPv4 EtherType 2048 in its Ethernet header, and a1 gets the datagram, each
     within 0.15 s.
  5. Before C9: b sends a C-tag, and a1 stops with the error above. After C9: the test passes, because a frame that
     leaves untagged carries no request for the tag and no request of the 802.1Q encapsulation.
  6. Code action: in `VlanReqMapper::processPacket()`, a map to -1 removes the `VlanReq`, the `PcpReq` and the
     `DropEligibleReq` of the tag type and requests no encapsulation; a frame with no map entry keeps the behavior of
     today. No state changes.
  7. Runner: `run_protocol_tests!(project; suite = "vlan", mode = "debug")` in the editor, after a debug build of the
     branch; the module tests `tests/module/Vlan*`, `Ieee8021*` and `EthernetUnknownEtherType_1`. Limit: the test
     covers the C-tag; the S-tag of a provider edge bridge with `VlanPolicyLayer` takes the same line with
     `vlanTagType = "s"` and has no test of its own.
  8. Reviewer document: `tests/protocol/vlan/vlan-req-mapper-untagged.md`, in C9, with a whatsnew entry.
- **C10: `examples: change: the bridges of the VLAN examples and tests forward the PCP, as a bridge does by default`**
  (answer (ii)). The four files above.

**Expected baselines.** None for C9: each stored configuration with `VlanReqMapper` and an untagged port sets
`forwardPcp = false`. C10 can move the fingerprints and the results of `examples/ethernet/vlan` and
`showcases/ethernet/sharedcore`, and the expected output of the module tests of `vlan-access-trunk.ini` and of
`EthernetUnknownEtherType_1`. On a trunk, the forwarded PCP equals the PCP that the inserter takes from the user
priority, so the frames can stay the same. The runs of the gate find each moved value, and each gets its line of
attribution.

**Checks.** The new protocol test and the other four tests of `tests/protocol/vlan` in debug mode; the VLAN module
tests; the pings of `showcases/ethernet/sharedcore` `FiltersAndMappers` get answers (E1 of step 79: 0 of 80 with
`forwardPcp = true` before); all fingerprints in release mode (§6.1, step 8); the statistical tests of the moved
folders in the image of the CI.

**The examples before and after (2026-11-25).** The maintainer asked for a complete example, a flow figure and a
diff. The change of design B and the removal of the four settings are in the working tree, not committed; the diff is
the preview commit `3ac195fce8` on the ref `preview/r3`, which does not land. Debug runs of
`showcases/ethernet/sharedcore` `FiltersAndMappers` and of five configurations of `examples/ethernet/vlan`
(`runs/r3-run.sh` and `runs/r3/` of the folder of D10):

- Today, with `forwardPcp = true` for every bridge: the showcase answers 0 of 80 pings, and the inbound filters of
  `sp1w`, `sp1e`, `sp2w` and `sp2e` drop 4 frames each. On the wire at `pebW` eth0, 4 frames carry an S-tag of VLAN 0
  and PCP 0 over the C-tag (`802.1Q-QinQ (0x88a8), vlan 0, p 0`). So the fault hits frames of priority 0 too: the
  copied PCP request makes the tag by its presence. In the VLAN example, `DoubleTagged` stops with "Unknown protocol
  ... ieee8021qstag" at host2, `Asymmetric` transfers 0 bytes, and the other three transfer all data.
- Today, with the workaround: all six runs work.
- After the change, with the default: all six runs work, and each scalar of the six runs equals the run of today with
  the workaround. The new protocol test and the other four VLAN protocol tests pass, and so do the 11 module tests
  that use the changed files (`EthernetUnknownEtherType_1`, `Ieee8021adPolicy_1`, `Ieee8021adPolicy_runtime_1`,
  `VlanIndMapper_1`, `VlanPolicyLayer_runtime_1`, `VlanPolicyMapsInvalid_1` to `_6`).
- The documentation page of the showcase explained the workaround and included the removed lines; the change replaces
  that paragraph.

#### R4: the constants of copper and aluminium (BD-49), prepared 2026-11-25

**Re-check.** Master `b029add207` changes no file of `src/inet/environment` or of the obstacle loss after
`08e3648c36`. `MaterialRegistry::getMaterial()` still makes copper with `Ohmm(1.68), NaN, NaN` and aluminium with
`Ohmm(2.65), NaN, NaN` (`MaterialRegistry.cc:37-38`), below the comment `// TODO verify values`. No environment file of
the repository uses either: its 1803 objects are of brick, concrete, forest and glass.

**The fault, shown.** Two new unit tests, `tests/unit/ObstacleLoss_DielectricCopper.test` and
`ObstacleLoss_DielectricAluminium.test`, have the form of `ObstacleLoss_Dielectric.test`: one path crosses a plate 2 mm
thick at 2.4 GHz, 10 MHz and 100 kHz, with no reflection loss. In debug mode on the branch both fail with "ASSERT:
Condition '0 <= factor && factor <= 1' does not hold in function 'computeDielectricLoss'"; `ObstacleLoss_Dielectric`
passes. The expected values come from the skin depth of a good conductor, delta = sqrt(2 rho / (2 pi f mu_0)), as the
check outside INET ([TR-VALIDATE-EXTERNAL](../../doc/project/rule/testing.md#tr-validate-external)): for copper
842.111 dB at 10 MHz and 84.211 dB at 100 kHz, for aluminium 670.504 dB and 67.050 dB. At 2.4 GHz the factor is below
the smallest double, so it is 0 and the test expects `inf dB`. The formula of the model gives the same values to the
third decimal, because the loss tangent of a metal is about 10^8 and more.

**A limit found by the test.** The first form of the tests had a sheet 10 um thick and gave 0 dB at each frequency:
`computeObjectLoss()` takes a loss only when `intersection1 != intersection2`, and `Coord::operator==` compares with
`EPSILON` (`INETMath.h:86`, 1 mm). So an object thinner than 1 mm along the path takes no loss at all, also a metal
foil that blocks the signal. The code is older than v4.7.0 (it is older than the reformatting `a903d00920` of 2020)
and not part of BD-49; the maintainer decides whether it becomes an entry.

**Design question for gate A.** The values: the resistivities at 20 °C, 1.68e-8 Ω m for copper and 2.65e-8 Ω m for
aluminium, with a relative permittivity and permeability of 1. The permittivity of a metal does not change the
result, because the conduction term is about 10^8 times larger, and the real permeabilities (0.999994 and 1.000022)
differ from 1 by less than 3e-5. The comment `// TODO verify values` stays, because the other materials are not
checked (BD-46). The assistant has no alternative to propose; gate A approves the values.

**Commit.**

- **C11: `environment: fix: copper and aluminium have their real resistivity and a relative permittivity and
  permeability of 1`** (BD-49). `MaterialRegistry.cc`, the two new unit tests, and a whatsnew entry. This is no
  protocol change, so [TR-PLAN-PROTOCOL](../../doc/project/rule/testing.md#tr-plan-protocol) asks for no protocol
  test and no reviewer document. Expected before C11: both tests fail at the ASSERT, as above. After C11: both pass.

**Expected baselines.** None: no stored simulation uses copper or aluminium.

**Checks.** The three `ObstacleLoss_Dielectric*` unit tests in debug mode; all fingerprints in release mode (§6.1,
step 8), which must not move.

#### R5: Tahoe and Westwood at the third duplicate ACK (BD-8, BD-39), prepared 2026-11-25

**Re-check.** Master `b029add207` changes no TCP file after `08e3648c36`. `TcpTahoe::resetToSlowStart()`
(`TcpTahoe.cc`), which the third duplicate ACK and the timeout call, still sets ssthresh to max(cwnd / 2, 2*SMSS).
`TcpTahoe::receivedDuplicateAck()` and `TcpWestwood::receivedDuplicateAck()` still test `dupacks == dupthresh` and
nothing else. Tahoe goes back to slow start from `snd_una` at its fast retransmit and at a timeout; Westwood
retransmits one segment and inflates cwnd at its fast retransmit, as Reno does, and goes back only at a timeout.
`recover` is a field of `TcpAlgorithmBaseState.msg` (`:71`), so both flavours have it already; only
`Rfc6582Recovery` and `TcpNewReno::ackProcessed()` use it, and `Rfc6582Recovery` starts it at ISS - 1
(`Rfc6582Recovery.cc:33`). The `recoveryPoint` of `TcpAlgorithmBase` at a timeout belongs to the SACK recovery.

**The faults, shown.** Three new protocol tests in `tests/protocol/tcp/rfc`, in debug mode on the branch:

- `Rfc5681TahoeThresholdFlightSize.test` (BD-8): host B offers 8 segments (11584 bytes), so cwnd grows beyond the
  window; the relay removes the 40th data segment. At the third duplicate ACK, cwnd is 33580 and FlightSize 10136:
  TcpTahoe sets ssthresh 16790, where equation (4) of RFC 5681 gives 5068. The test fails at that step.
- `Rfc6582TahoeOneFastRetransmitPerWindow.test` (BD-39): the relay removes the 40th, 42nd and 44th data segments.
  After the first fast retransmit and the go-back, the duplicates of the segments that host B already had start a
  second fast retransmit at t = 0.2089 s, with the cumulative ACK 117672 equal to the highest octet + 1 that host A had
  sent at the first; RFC 6582 needs an ACK above it. The guard fails.
- `Rfc6582WestwoodOneFastRetransmitPerWindow.test` (BD-39): the relay removes six data segments of one window, the
  retransmission timer expires at 1.208 s, and the go-back starts a fast retransmit at t = 1.2093 s, with the
  cumulative ACK 95952 below the highest octet + 1 that host A had sent at the timeout, 117672. The guard fails.

Two faults of the first forms of the tests, not of the model: the last data segment carries the FIN (`tClose = -1s`),
so no ACK covers exactly the last data octet, and the last step reads `tcp.ackNo >=`; and a first form of the
Westwood test with three losses and a guard from the fast retransmit had no second fast retransmit, because Westwood
does not go back there.

**Design questions for gate A.**

1. *The threshold of Tahoe (BD-8).* (A) `resetToSlowStart()` takes FlightSize as `~TcpReno` does: at the third
   duplicate ACK `conn->getFlightSize()`, which leaves out the data of limited transmit, and at a timeout the
   outstanding data `snd_max - snd_una`, as `calculateSsthreshForRto()` of `TcpClassicAlgorithmBase.h:66` does.
   RFC 5681 section 3.1 sets equation (4) for both. The assistant recommends (A). No alternative fits the RFC.
2. *Where Westwood sets recover (BD-39).* Tahoe sets recover at its fast retransmit and at a timeout, because both
   go back. For Westwood: (a) at a timeout only, the go-back that RFC 6582 section 6 names; its fast retransmit
   stays the one of Reno, so a second real loss in the window still gets a fast retransmit. (b) Also at its fast
   retransmit, as `~TcpNewReno` does; then a second loss in the window waits for the timer, because Westwood has no
   partial ACK of NewReno. The assistant recommends (a): it repairs the cause of BD-39 and changes nothing else of
   the recovery of Westwood.
3. *The test of recover.* Both flavours use the test of `Rfc6582Recovery`: a fast retransmit only when
   `snd_una - 1 > recover`, with `recover = snd_max - 1` when it is set, and `recover` starts at ISS - 1 when the
   connection is established. The heuristics of RFC 6582 section 4 (the ACK and the timestamp heuristics) stay out,
   as in `~TcpNewReno`.

**Commits** (with the recommended answers).

- **C12: `tcp: fix: TcpTahoe takes ssthresh from FlightSize, as RFC 5681 equation (4) says`** (BD-8).
  `TcpTahoe.cc`. The items of [TR-PLAN-PROTOCOL](../../doc/project/rule/testing.md#tr-plan-protocol):
  1. Requirement: RFC 5681 section 3.1, equation (4), and its note against cwnd; section 3.2, step 2. Bug: ssthresh
     from cwnd. Evidence: the test above; `inet-tahoe` run 0 at t = 0.412 s (BD-8).
  2. Existing tests: `TcpAlgorithmClassTahoe.test` checks the window of one MSS at a fast retransmit, not the
     threshold; `Rfc5681FastRetransmit.test` runs `TcpReno`. None exposes the gap.
  3. New: `tests/protocol/tcp/rfc/Rfc5681TahoeThresholdFlightSize.test`, one case.
  4. Initial state: TcpTahoe without SACK and without limited transmit, host B with an advertised window of
     11584 bytes. Stimulus: 300000 bytes, the relay removes the 40th data segment. Production path:
     `receivedDuplicateAck()`, `resetToSlowStart()`. Observation: the segments, the ACKs and the cwnd signal of host A,
     and its ssthresh signal after the third duplicate. Assertion: cwnd > FlightSize at the third duplicate, and
     ssthresh = max(FlightSize / 2, 2*SMSS).
  5. Before C12: ssthresh 16790, expected 5068. After C12: equal, because the formula takes FlightSize.
  6. Code action: `resetToSlowStart()` of `TcpTahoe`; ssthresh changes, cwnd is still one SMSS.
  7. Runner: `run_protocol_tests!(project; file = "Rfc5681Tahoe|Rfc6582|TcpAlgorithmClass", mode = "debug")` in
     the editor, after a debug build. Limit: the timeout path of the formula has no test of its own.
  8. Reviewer document: `tests/protocol/tcp/tahoe-threshold-flight-size.md`, in C12, with a whatsnew entry.
- **C13: `tcp: fix: TcpTahoe and TcpWestwood start one fast retransmit in each window (RFC 6582 section 6)`**
  (BD-39). `TcpTahoe.cc` and `TcpWestwood.cc`.
  1. Requirement: RFC 6582 section 6 and the steps "Three duplicate ACKs" and "Retransmit timeouts" of section
     3.2. Bug: a second fast retransmit from the duplicates of the go-back. Evidence: the two tests above; `inet-tahoe`
     run 0 does 164 fast retransmits in place of 7 after `07722bc21b` (BD-39).
  2. Existing tests: none for Tahoe or Westwood; `~TcpNewReno` has the guard and its tests.
  3. New: `Rfc6582TahoeOneFastRetransmitPerWindow.test` and `Rfc6582WestwoodOneFastRetransmitPerWindow.test`, one
     case each.
  4. Initial state: the flavour without SACK. Stimulus: 300000 bytes, the relay removes three (Tahoe) or six
     (Westwood) data segments of one window, once each. Production path: `receivedDuplicateAck()`, the go-back of
     `resetToSlowStart()` (Tahoe) or of `processRexmitTimer()` (Westwood). Observation: the ACKs and the segments of
     host A, and its `numRtos` signal. Assertion: no fast retransmit while the cumulative ACK does not cover more than
     the highest octet that host A had sent at the last fast retransmit (Tahoe) or timeout (Westwood), and the run ends
     with the ACK of all data.
  5. Before C13: both guards fail, at t = 0.2089 s and t = 1.2093 s. After C13: both pass, because the third
     duplicate below recover starts no fast retransmit.
  6. Code action: `receivedDuplicateAck()` of both flavours reads `recover`; `resetToSlowStart()` (Tahoe) and
     `processRexmitTimer()` (Westwood) set it.
  7. Runner: as for C12. Limit: the timeout path of Tahoe and the fast retransmit of Westwood after a second real
     loss have no test of their own.
  8. Reviewer document: `tests/protocol/tcp/one-fast-retransmit-per-window.md`, in C13, with a whatsnew entry.

**Expected baselines.** `examples/inet/tcpclientserver` `inet-tahoe` and `inet-westwood` (C12 moves `inet-tahoe`,
C13 both), and each fingerprint of a run with TcpTahoe or TcpWestwood. The numbers of `inet-westwood` in BD-39 come
from before `7beeb0dc88`, so the gate measures them again. Each moved value gets its line of attribution, and the
re-record runs in the image of the CI.

**Checks.** The three new protocol tests, `Rfc5681FastRetransmit`, `Rfc5681TimeoutResponse` and the
`TcpAlgorithmClass*` tests in debug mode; all fingerprints in release mode (§6.1, step 8); the statistical tests of
`examples/inet/tcpclientserver` in the image of the CI.

## 7. Verification

### 7.1 The checks of each change

| Criterion | Check |
| --- | --- |
| 1 | The new test of §6.2 fails before the repair and passes after it, in debug mode and, for a TCP change, in release mode. The log of both runs is a task group of the editor. |
| 2 | `check-baselines.sh origin/master..HEAD` and `check-baseline-attribution.sh run`, in the job copy of the CI. |
| 3 | The reviewer document exists beside the test, and the review checklist of [review-a-code-change.md](../../doc/project/guide/review-a-code-change.md) passes. |
| 4 | For R9, R14 and R15: the fingerprints and the statistical baselines of the `~LinuxTcp` tests do not move. |
| 5 | "Before every push" of [run-the-gates.md](../../doc/project/guide/run-the-gates.md). |
| 6, 7 | The maintainer reads the defect list and the view at gate B. |

The existing tests of each place, which the focused run of step 5 of §6.1 includes, are in the module tests
`tests/module/tcp_*.test`, the protocol tests `tests/protocol/tcp/rfc/` and `tests/protocol/tcp/parameter/`, and for
the other changes `tests/protocol/vlan/`, `tests/unit/ObstacleLoss_Dielectric.test`, `tests/queueing/CompoundQueue_1.test`
and `tests/protocol/quic/`. Step 3 of §6.1 names the exact files.

### 7.2 Limits

- A test shows one scenario of a fault. A repair can leave the fault in a case that no test reaches.
- A statistical baseline holds the configured runs and seeds only.
- The checks of this plan come from the code of 2026-11-24. A change of master can repair, change or break an entry
  later; step 1 of §6.1 finds that.
- The record in the image of the CI matches the CI of 2026-11-24. If GitHub changes `ubuntu-latest`, some rows move
  with no change of INET (§2.1).

## 8. Risks, assumptions and approval scope

**Assumptions that the steps check:**

- **A1.** The job copy of the CI builds OMNeT++ and INET again in the image (step 0).
- **A2.** The cause of BD-24 is the one of §2.4 (step 12).
- **A3.** The tests of `tcpapp` have an ACK of the SYN-ACK with data (step 11).

**Risks:**

- Many TCP changes move the same baselines, such as `examples/inet/bulktransfer`. Each change records its own move,
  so the statistics branch has one commit for each such change, in order. A rebase of a baseline branch follows
  [change-a-baseline.md](../../doc/project/guide/change-a-baseline.md), "After a baseline commit or the base changes".
- R9, R14 and R15 change the defaults of `~Tcp`. Their moves are large: step 18 of the catch-up moved 42 tests.
- R19 changes about 80 files. A wrong edit of a test file changes what the test checks; the assistant reads each
  file that the subagent changed.
- Other work can change TCP on master during this plan. Step 1 of §6.1 runs at the start of each change.

**Answered questions** (the approval of 2026-11-24 kept the proposed answers):

1. The scope is §2.2: BD-31 is out as an 802.11 entry; BD-33 is out because its 802.11 part is repaired and the rest
   needs a decision; BD-24 and BD-39 are in as easy entries.
2. The order is D4, with R1 as the first change.
3. D2 holds: the plan goes into git, also on master. D10 changed on the same day: the files of the view stay
   outside git.

**Approval scope:** an approval of this plan covers step 0 and the cycle of §6.1 for each change up to its gate A. The
code of each change needs gate A, and its landing needs gate B. The approval does not cover a push, a change of a
sealed file, or a change of another repository. This plan changes no sealed file.

## 9. The live view in the editor

The maintainer follows the work in the window of `omnet_ide`, where the assistant drives it (D8). The verbs below
exist in the editor of 2026-11-24.

### 9.1 What the maintainer sees

| View | It shows | Made with |
| --- | --- | --- |
| **Workflow**, a tab above the conversation | One line for each step of §6.2, with a short title such as "R2 TCP override timer" and its state (`:open`, `:active`, `:done`, `:dropped`). A step gets an entry only at gate A and at gate B, with the words of the maintainer, and when it ends with no repair. No sub-steps and no embedded files | `change_workflow_state!`, `add_workflow_entry!` |
| **Progress**, a tab of three charts | (1) the 27 entries in scope in four groups of states: not started, in work, repaired, landed or closed; (2) a stacked bar for each change R1 to R19, with its entries in the same four groups; (3) the count of the entries repaired and of the entries landed or closed, on each day of the work | `Dashboard` of three `DashboardCard`s, each a `Chart` (`ChartBarSeries`, `ChartLineSeries`) that the helper `refresh_progress_1()` draws again from the table |
| **Defects**, a tab with a table | One row for each of the 44 entries: BD, title, area, change, state, scope, reason, origin, repair commit, `repaired_on`, `landed_on`. The states of an entry in scope are `1 open`, `2 re-checked`, `3 fault shown`, `4 design approved`, `5 repaired`, `6 result approved`, `7 landed` and `8 closed`; an entry out of scope is `left out` or `closed`. The maintainer filters and sorts it | `RecordTable` in a `.pred` file; the helper `set_defect_state_1(number, state; commit)` writes a state and its date, saves the table and refreshes the charts |
| **Tasks**, in the explorer tab | Each build, test, update and attribution run as a group with a progress ring, counts and the log of each task, under "Tasks of the editor". A group gets no tab of its own (the maintainer, 2026-11-24: "you should not open each build in a new tab pane, that's too much"); only the runs of a gate open in its page | `build_simulation_project!`, `run_protocol_tests!`, `run_module_tests!`, `run_unit_tests!`, `run_fingerprint_tests!`, `update_fingerprint_test_results!`; `run_tasks!` with `ProgramTask` for `tools/prun.sh` |
| **Moved values** | The scalars that a statistical test found different, and the differences of two result files | the page of a statistical test task (`LegacyStatisticalTaskDocument`, `LegacyScalarDifferences`), `LegacyResultComparison`, `LegacyTestLog` for the log of a run in the image |
| **Commits** | The commits of the topic branch and of the statistics branch, each with its changed files and diffs | `GitCommitRange("…/inet-cpp-defect-repairs", "master..topic/baseline-catch-up-defect-repairs")`, `GitCommit` |
| **Explorer "Defect repairs"**, a tab | The workflow, the table, the charts, this plan, [baseline-catch-up-defects.md](baseline-catch-up-defects.md), the two commits that repaired entries before the plan, the TCP commits since v4.7.0, the project of the worktree, the two commit ranges, and the tasks of the editor | `ExplorerSection`, `Explorer`, `open_file_document!`, `get_session_task_group_list()` |
| **The page of a gate** | At gate A the prepared entry of §6.3; at gate B the reviewer document, the moved values and the commits, each in a tab | `open_pane!`, `show_pane!` |

A pivot table of the editor can draw the same charts, but it shows its field controls above each chart and no labels
of the categories, so the charts are plain `Chart` documents. A pivot and a chart keep a copy of their rows, so the
assistant draws the charts again after each change of the table.

### 9.2 How the assistant keeps the view current

1. When an entry changes its state, call `set_defect_state_1(number, state; commit)`. It writes the state and the
   date, saves the table and draws the charts again.
2. When a step starts, mark it `:active`. When it ends, mark it `:done`, or `:dropped` with an entry that says why.
3. At gate A and gate B, add one entry to the step with the words of the maintainer.
4. When a task group starts, open no tab for it: it shows under "Tasks of the editor" in the explorer tab. The
   workflow gets no entry for a task group.
5. After each change of the workflow, call `save_workflow_1()`.
6. When a program rewrites a file that a tab shows, load it again with `reload_file_from_disk!`.

### 9.3 The files

The folder of D10, `/home/projectured/workspace/ghci-baseline-catch-up/evidence/defect-repairs/`, outside git,
holds:

- `workflow.pred`: the workflow.
- `defects.pred`: the table of the defects. The charts come from it, so no other file keeps the progress.
- `ide-session.jl`: the code that opens the four tabs of §9.1 again after a restart of the editor, and defines the
  helpers of §9.2. The assistant passes its text to one `execute_julia_code` call. It reads the plan from the
  worktree and the two other files from this folder.
