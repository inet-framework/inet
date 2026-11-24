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

27 entries in 19 changes. "Origin" says whether the fault is new since v4.7.0. "Easy" gives the size that the code read
found on master.

| Change | Entries | Area | Origin | Easy | Place on master |
| --- | --- | --- | --- | --- | --- |
| R1 | BD-5, BD-13, BD-16, BD-6, BD-9 | TCP text and code with no result change | new (`b2313324ef`, `985edc2057`, `80357d0e09`, `d6da042a42`) | yes: 1 to 4 lines each | `TcpConnection.h:599-607`, `TcpReno.h:42-45`, `Rfc5681CongestionControl.cc:100`, `TcpClassicAlgorithmBase.h:66`, `Rfc6675Recovery.cc:332` |
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
- **D10. The files of the view.** The workflow, the table of the defects and the script that opens the view again
  are in `plan/evidence/baseline-catch-up-defect-repairs/` and go into git with the plan changes, as D6 of
  [ieee80211-chart-baseline-evolution.md](ieee80211-chart-baseline-evolution.md) decided for its workflow.

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
   `check-baseline-attribution.sh plan` and `run` in the job copy of the CI.
9. **Gate B.** Open the reviewer document, the moved values and the commits of the change in tabs. Ask the
   maintainer: "Do you approve the result of R<n>?" After the approval, mark the change done in this plan, write the
   repair commit into [baseline-catch-up-defects.md](baseline-catch-up-defects.md), and commit both as
   `plan: change: R<n> is repaired`. The maintainer lands the change.

### 6.2 The steps

- [ ] **Step 0. Set up and correct the list.**
  1. Make the worktree and the branch of D1. Copy the plan commit (D2). Run `opp_checkout_test_baselines` in the
     worktree. **Done 2026-11-24.** The two plan commits of `inet-julia` are `cf630c7310` and `f19249ad58` on the
     topic branch. `opp_checkout_test_baselines` is not on the PATH of this machine, so `statistics` and `media` are
     clones with `--reference-if-able` of the clones of the main checkout, at the pins `9a67a652ff` and `cd94c731e7`.
  2. In the editor, open the worktree as a project with `open_simulation_project!`, and build it in debug and release
     mode as tasks. **Started 2026-11-24**, each build with 16 jobs.
  3. In the job copy of the CI, run `tools/prun.sh build-omnetpp`, fetch the topic branch into
     `inet-cpp-baseline-catch-up`, and run one statistical test of `examples/inet/bulktransfer` at master. It must
     pass with the pin of master.
  4. Write the corrections of §2.4 and the closure of BD-40 into
     [baseline-catch-up-defects.md](baseline-catch-up-defects.md). **Done 2026-11-24** in C2, which also records BD-28
     as repaired by `d69fc0eaff`.
  5. Make the view of §9 and save its files. **Done 2026-11-24.**
  - **C1: `plan: add: repair the defects of the baseline catch-up that are new since v4.7.0 or easy, outside IEEE 802.11`.**
    This plan and the files of the view. No dependency.
  - **C2: `plan: change: the defects of the baseline catch-up after the check of 2026-11-24`.** The corrections of
    §2.4 and the closure of BD-40. Depends on C1.
- [ ] **Step 1. R1: TCP text and code with no result change (BD-5, BD-13, BD-16, BD-6, BD-9).** Check: the
  fingerprints of all TCP tests do not change, and the module tests of the places pass.
  - `tcp: comment: each comment is above its function, and no comment describes a removed function` (BD-5, BD-13,
    BD-16).
  - `tcp: refactor: calculateSsthreshForRto() uses calculateSsthresh() with the effective MSS` (BD-6).
  - `tcp: refactor: step (2) of RFC 6675 tests IsLost(HighACK + 1) at SND.UNA` (BD-9).
- [ ] **Step 2. R2: the override timer (BD-21, BD-23).** Gate A decides the override interval: a separate interval in
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
- [ ] **Step 20. Close.** Write the summary of the changes and the closed entries at the top of this plan. Move it to
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
3. D2 and D10 hold: the plan and the files of the view go into git, also on master.

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
| **Tasks** | Each build, test, update and attribution run as a group with a progress ring, counts and the log of each task; each group opens in a tab when it starts | `build_simulation_project!`, `run_protocol_tests!`, `run_module_tests!`, `run_unit_tests!`, `run_fingerprint_tests!`, `update_fingerprint_test_results!`; `run_tasks!` with `ProgramTask` for `tools/prun.sh` |
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
4. When a task group starts, open it in a tab. The workflow gets no entry for a task group.
5. After each change of the workflow, call `save_workflow_1()`.
6. When a program rewrites a file that a tab shows, load it again with `reload_file_from_disk!`.

### 9.3 The files

`plan/evidence/baseline-catch-up-defect-repairs/` holds:

- `workflow.pred`: the workflow.
- `defects.pred`: the table of the defects. The charts come from it, so no other file keeps the progress.
- `ide-session.jl`: the code that opens the four tabs of §9.1 again after a restart of the editor, and defines the
  helpers of §9.2. The assistant passes its text to one `execute_julia_code` call.

Before step 0, these files are in the main checkout. After step 0, they are in the worktree, and `ide-session.jl`
names the paths of the worktree.
