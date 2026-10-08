# Land the TCP modernization on master in small stages

Status: **in progress** — S1 and S2 landed on master on 2026-10-02, S3, S4 and S5 on 2026-10-05, S5b and S5c on 2026-10-06. S5d is next.

Source: the branch `topic/tcp-new-audit-fixes`, local head `1826c3f4be` on master `86cede7986`.
Its own plan is `plan/pending/pr-1155-resolve-audit-findings.md` on that branch. An older copy of
that plan is on master.

## 1. Why

The branch has 90 commits, about +17,000 and −4,000 lines. Nobody can review it in one piece.
The owner decided on 2026-10-02 to land it on master in small groups. Each group is a branch that
the owner reviews alone. A group lands on master only after the owner approves it.

## 2. The rules for every stage

1. **One stage, one topic.** A stage holds the commits of one feature or one refactor, the tests
   of those commits, and nothing else.
2. **Master stays green after each stage.** Every commit builds. The tests of the stage pass.
   No test of master changes its result, unless a commit of the stage moves it on purpose and
   explains why.
3. **A moved fingerprint row has an explanation.** This is decision D-2 of the source plan, and
   its step 5. The stages do step 5 one stage at a time.
4. **A fix of new code goes into the stage of that code.** Two cases:
   - The defect came from an earlier commit of the same stage, and master does not have it: fold
     the fix into that commit. A refactor then stays a refactor, and master never holds the
     defect.
   - Master has the defect too: the fix stays its own commit, with its `Reproduce:` trailer.
5. **A later commit that only corrects text of an earlier commit goes with that earlier commit.**
   Example: `tcp: comment: drop test-corpus provenance from code comments` splits into the stages
   that add those comments.
6. **The commit messages name this plan.** The `Plan:` line changes to
   `plan/pending/tcp-staged-landing.md`. Old evidence lines (`Verified: all 516 fingerprint
   rows...`) and the "Rebased onto master" paragraphs are replaced by the evidence of the stage.
7. **The owner reviews before landing.** I build and check the stage, then stop. The stage lands
   (fast-forward of `master`) only after the owner says so.

## 3. The stages

The numbers are the positions of the commits on `1826c3f4be`. The order of the stages follows
the dependencies that `git blame` shows: each commit was compared with the earlier commits whose
lines it changes. A dependency of one to three lines is often only a neighbour line, and its
cherry-pick conflict is easy. The stage boundaries are provisional. A stage can merge with its
neighbour or split, when its cherry-picks or its builds show a reason.

| Stage | Branch | Commits | Size (src, no tests) | Contents |
| --- | --- | --- | --- | --- |
| S1 ✅ | `topic/tcp-header-options` | 1, 52 | +228 −26 | the TCP Fast Open and AccECN header options, the AE bit |
| S2 ✅ | `topic/tcp-tidy-ups` | 4, 5, 6, 21 | +193 −134 | RFC citations, the signals in one place, the ACK callback rename |
| S3 ✅ | `topic/tcp-socket-contract` | 2 | +475 −1 | the socket commands, tags and status fields (commit 3 left the stage, see D-4) |
| S4 ✅ | `topic/tcp-recovery-split` | 7, 8, 9, 10 + 50 | +1872 −4 | the recovery interfaces and the RFC 5681, RFC 6582 and RFC 6675 strategies, which nothing selects yet (D-5) |
| S5 ✅ | `topic/tcp-flavour-split` | 11, 12 | +152 −136 | the move and the rename of the two base classes (D-6) |
| S5b ✅ | `topic/tcp-flavour-strategies` | 13 (part), new | +450 −470 (about) | the refactor part of commit 13: B1 to B3 of D-6, with master's arithmetic |
| S5c ✅ | `topic/tcp-classic-recovery` | 13 (part), 16 (part), 39 (part), 41 (part), 54, 70 | +320 −40 (about) | the behavior changes B4, B6, B10, B11 of D-6, and the preparation of the pipe accounting |
| S5d ✅ | `topic/tcp-sack-recovery` | 10 (part), 13 (part) | +91 −77 | SACK loss recovery in `Rfc6675Recovery`, and DCTCP on it: B12, B13, B16 of D-6; SACK only for a flavour that can recover with it |
| S5e ✅ | `topic/tcp-pipe-recovery` | 13 (part), 34 (part), 39 (part), 56, 71, 72, 80 (part) | +248 −161 | Reno and NewReno without SACK recover by pipe accounting: B5, B7, B8 of D-6 |
| S6 ✅ | `topic/tcp-cubic` | 14, 45, 55 (`TcpCubic` part), 57, 60, 62, 65 | +840 −326 | `TcpCubic` with HyStart, and `DcTcp` on the shared ACK path |
| S7 ✅ | `topic/tcp-segment-sizing` | 13 (part), 15 (part), 16 (part) | — | segment sizing against the option space, the peak segments in flight and slow start (B9); before S6 |
| S8 ✅ | `topic/tcp-rack` | 13 (part), 16 (part), 17, 18, 19, 26 (part), 49, 51 (part), 64 (part) | +768 −47 | RACK loss detection (RFC 8985), STATUS counters, the reordering window; the Linux count of the bytes in flight with SACK (B8) |
| S9 ✅ | `topic/tcp-prr` | 13 (part), 20, 39 (part), 59, 64 (part) | +143 −14 | Proportional Rate Reduction (RFC 6937) |
| S10 ✅ | `topic/tcp-undo-frto-tlp` | 22, 24, 25, 37, 76, 85, 86, new | +1807 −25 | spurious-loss undo, F-RTO (RFC 5682), the tail loss probe (D-29 to D-35) |
| S11 | `topic/tcp-connection-lifecycle` | 15 (part), 16 (part), 22 (part), 23 (part), 26 (part), 30, 38, 39 (part), 73, 74 | +179 −45 | reset after a full close, SYN-ACK re-send, FIN read clamp, STATUS before open, the simultaneous open, a forked connection, the SYN-ACK TSecr check, `syncookiesAlways` |
| S12 | `topic/tcp-fast-open` | 15 (part), 16 (part), 26, 43, 44, 51 | +1187 −104 | TCP Fast Open (RFC 7413) |
| S13 | `topic/tcp-accecn` | 27 | +864 −61 | Accurate ECN; DCTCP counts its bytes also in a fast recovery (D-23) |
| S14 | `topic/tcp-receive-buffer` | 28, 29, 35, new work | +400 −3 | the receive buffer apart from the advertised window, zero-copy, `TCP_NOTSENT_LOWAT`, the receive buffer of a socket before open (D-4) |
| S15 | `topic/tcp-timers` | 31, 32, 33, 34 (part), 77 | +441 −94 | adaptive delayed ACK, timer parameters, keepalive, loss marking at a timeout, no new fast recovery before the recovery point of a timeout (D-34, proposed) |
| S16 | `topic/tcp-write-boundaries` | 15 (part), 16 (part), 36, 69 | +355 −9 | PSH at write boundaries, `TCP_CORK`, a window smaller than one MSS |
| S17 | `topic/tcp-connection-leftovers` | 39, 61 | +445 −185 | the remaining connection work, the SACK scoreboard scan, two repairs |
| S18 | `topic/tcp-pmtud-rcvbuf` | 46, 47, 42, 58 | +556 −65 | RFC 4821 path MTU discovery, receive buffer memory, the applications |
| S19 | `topic/tcp-modern-defaults` | 15 (part), 16 (part), 40, 53, 63, 68, 70, 71, 72, 78, 79, 80 (part), 81, 82, 87 | +863 −488 | the modern defaults, the RFC 6298 and RFC 5681 corrections, the tests under the new defaults |
| S20 | `topic/tcp-modernization-docs` | 48, 66, and the source plan | +700 −200 (about) | the RFC list of the module, the release note, the source plan to `plan/done/` |

The big test commit (41, `tests: add+change: cover the new behavior, and pin the old defaults`,
74 files, +4529 −250) splits into the stages. Each stage takes the test files of its own
features. A change of the shared test library (`tests/module/lib/`) goes with the first stage
that needs it. The changes that pin the old defaults (`tcp-legacy.ini` and the edits of existing
`tcp_*` tests) go with S19.

The plan commits of the source branch (67, 75, 83, 88, 89, 90) do not land one by one. Their
result is the source plan, and S20 moves it to `plan/done/`.

Known items for later stages, found by the blame:

- 55 (`count duplicate ACKs for every flavour again`) changes `TcpCubic` lines. That part goes
  to S6.
- 57 (`put TcpCubic on TcpClassicAlgorithmBase`) touches one or two lines of 25, 34 and 44,
  which land later. S6 resolves these conflicts by hand.
- 70, 71 and 72 repair `Rfc5681Recovery` (S4 code), but they change lines of 68 and of the test
  commit. They stay in S19 unless they apply cleanly earlier.
- The fold rule (rule 4) must be decided for each fix with evidence: does master show the defect?
- The connection's copy of the SACK recovery stays until its declarations go (D-5). Commit 26
  (S12) removes `TcpConnection::processSACKOption()` with its declaration, and commit 39 (S17)
  removes `TcpConnectionSackUtil.cc` with the other six declarations.
- `TcpBaseAlg::calculateSsthresh()` (commit 8) uses `state->snd_effmss` for the 2 * SMSS floor.
  Nothing sets that field before commit 15 (S7), so its value is 0. Commit 13 (S5) makes the
  flavours call this function. S5 must not land a missing floor: it either sets `snd_effmss`
  earlier or uses `snd_mss` until S7.

### The behavior changes inside commit 13

Against master, the folded commit 13 changes these behaviors. Each line names where the
behavior goes.

| # | Behavior | Master | Commit 13 | Goes to |
| --- | --- | --- | --- | --- |
| B1 | who counts duplicate ACKs | the connection | the algorithm | S5b, refactor |
| B2 | send times of Vegas and Westwood | a list in each flavour | one list in `TcpAlgorithmBase`, with a range guard | S5b, refactor if the guard never refuses |
| B3 | the flavours as a choice of strategies | logic in each flavour | `Rfc5681CongestionControl`, `Rfc5681Recovery`, `Rfc6582Recovery`, `Rfc6675Recovery` | S5b, refactor: the strategies carry master's arithmetic |
| B4 | what a duplicate ACK is | same ACK number, no data, data outstanding | RFC 5681: also the same window, no SYN and no FIN | S5c |
| B5 | Reno fast retransmit and recovery | `cwnd = ssthresh + 3*SMSS`; recovery while `dupacks >= dupthresh` | `cwnd = ssthresh`; recovery while `lossRecovery`, which a timeout ends | S5c |
| B6 | ssthresh at fast retransmit and timeout | `max(min(cwnd, snd_wnd)/2, 2*SMSS)` (a `FIXME` of master) | `max(FlightSize/2, 2*SMSS)` | S5c, with commit 70, the RFC 5681 equation (4) |
| B7 | NewReno | full ACK: `min(ssthresh, FlightSize + SMSS)`; partial ACK deflates cwnd | full ACK: `ssthresh`; partial ACK does not deflate; the head is marked lost | S5c |
| B8 | bytes in flight | `snd_max - snd_una` | minus SACKed and lost, plus retransmitted (Linux) | S5c, where B5 to B7 need it |
| B9 | slow-start growth | `+SMSS` per ACK | `+min(acked, SMSS)` (RFC 5681 byte counting) and the cwnd-limited gate | S7, with `maxPacketsOut` (commit 16) |
| B10 | ECN reaction | Reno only; `ssthresh = cwnd/2`, floor 1 octet | every classic flavour; `ssthresh = cwnd` after halving, floor 1 SMSS (commit 54) | S5c |
| B11 | Tahoe at the third duplicate ACK | ssthresh from B6, `cwnd = SMSS`, one retransmission | `ssthresh = cwnd/2`, the timeout path | S5c |
| B12 | SACK loss recovery | the connection's copy (D-5) | `Rfc6675Recovery`, with the differences of commit 10, and `ensureRexmitTimerArmed()` | S5d |
| B13 | immediate ACK while the own sender recovers | yes | no | S5d |
| B14 | a pure window update with nothing in flight | ignored; the persist timer probes | updates the window and sends (commit 84) | S15 |
| B15 | first zero-window probe | after `persist_timeout` | after one RTO (Linux) | S15 |
| B16 | DCTCP | its own SACK path on the connection | `Rfc6675Recovery`; `ecnMarkAll`; an AccECN branch | S5d, the AccECN branch in S13 |
| B17 | bookkeeping without effect | — | `minRtt`, `zeroWindowProbesSent`, `time_last_data_sent` at establishment, the Fast Open window | their feature stages: S8, S12 |

## 4. The procedure of one stage

1. Make the branch and its worktree from `master`: branch `topic/tcp-<name>`, worktree
   `/home/levy/workspace/inet-tcp-<name>`.
2. Cherry-pick the commits of the stage in the order of the table. Take the test files of the
   stage out of commit 41. Apply rules 4 to 6.
3. Compare: each file of the stage must be equal to its version on the source branch at the
   commit where the stage ends, or the difference must have a reason in this plan.
4. Run the gates: `check-classification.sh`, `check-commits.sh`, `check-links.sh`,
   `check-seals.sh`, `check-source-seals.sh`, and `check-series-builds.sh master..HEAD debug`.
5. Check that every commit builds and that `libINET` has no undefined `inet::` symbol (D-5).
   Build in debug against `omnetpp-6.x` (the pairing of `inet-master`) and run the tests of the
   stage: its unit and module tests, the module `tcp_` suite, the protocol tests `tcp/`, and
   the fingerprint ingredients `tplx` and `~tND` at the stage head. If a row moves, find the
   commit that moves it and explain the row in that commit (rule 3).

   **The CI job is the fingerprint oracle.** `/var/tmp/claude-staged/ghci-fp.sh` runs INET's own
   fingerprint job (`fingerprinttest -f tplx -f '~tNl' -f '~tND'`, every row of the CSV files,
   1752 tests) in the GitHub-like container of `~/workspace/ghci-statistical`, and compares with
   the stored values. A refactor must pass; a change writes `.UPDATED` files with the new values,
   which go into the commit that moves them, with an explanation of each row.

   The local runs (`/var/tmp/claude-staged/fp.sh`, debug, `omnetpp-6.x`) are a second check only:
   the Python runner reads the JSON store, which has fewer than half of the CSV rows (753 `tplx`
   rows; for example no `examples/inet/tcp_pmtud` row), and many stored values do not match in
   this environment even on master. They compare the calculated values of two commits.

   Run the TCP fingerprint rows in debug at the stage head (`inet_fingerprinttest -d -m tcp`).
   Only a debug run checks the `@signal` declarations of NED: the CI job runs release, and the
   module tests give `--check-signals=false`. S8 and S9 emitted `sndMax` and `delivered`
   without a declaration, and every TCP simulation in debug stopped at the first emit.
6. Stop. Give the owner the branch, the list of commits, and the evidence.
7. After the owner approves: fast-forward `master` to the stage. Push only when the owner says so.
8. Mark the stage done in this plan, with its results, in a last commit of the stage. Remove the
   stage branch and its worktree after the landing.

Do not rebase `topic/tcp-new-audit-fixes` after each stage. All stages cut from the same source
commit, `1826c3f4be`, so the commit numbers of section 3 stay valid. After the last stage, the
tree of master must be equal to the tree of `1826c3f4be`, except for the differences that this
plan gives a reason for.

## 5. Decisions

- **D-1 — The fold rule (rule 4).** Proposed on 2026-10-02. The owner approved S1, its first
  case, on 2026-10-02.
- **D-2 — The `Plan:` line names this plan (rule 6).** The source plan stays on its branch until
  S20.
- **D-4 — A socket option is not a module parameter.** Commit 3 (`let the window and timestamp
  parameters change at run time`) made `advertisedWindow`, `windowScalingFactor` and
  `timestampSupport` `@mutable`. Its only user is `PacketDrillApp` in inet-gpl. That app models
  `setsockopt(SO_RCVBUF)` before listen or connect with a change of the `tcp` module parameters.
  The owner rejected this on 2026-10-02: the change applies to every later connection of the
  host, not to one socket. The receive buffer of one socket must travel with the socket, in the
  open command, as `tcpAlgorithmClass` does. S14 adds this, because S14 also brings
  `receiveBufferSize`, which has the same problem. inet-gpl's `PacketDrillApp` then changes. A
  `@mutable` `timestampSupport` for the host-wide sysctl `net.ipv4.tcp_timestamps` is decided at
  S14 too; nothing in INET needs it before then. Commit 3 does not land.
- **D-5 — The connection keeps its SACK recovery until its last caller goes.** Source commit 10
  deleted `TcpConnectionSackUtil.cc`, but `TcpConnection` kept the declarations of its seven
  virtual functions, and the flavours and the connection kept calling them. `make` still passes,
  because a shared library links with undefined symbols, but `libINET` then does not load:
  `undefined symbol: ...TcpConnection17processSACKOption...`. On the source branch this holds for
  commits 10 to 38, so no simulation can run at 29 of its commits, and `check-series-builds.sh`
  does not see it. The stages keep the file: S4 adds `Rfc6675Recovery` next to the connection's
  copy, and each function of the copy goes in the commit that removes its declaration. The stage
  scripts check every commit for undefined `inet::` symbols
  (`nm -DC --undefined-only src/libINET_dbg.so`).
- **D-6 — Commit 13 is not a refactor, so S5 holds only the refactor (owner, 2026-10-05).**
  Commit 13 (`move the classic flavours onto the split architecture`) says `refactor`, but at S5
  it failed 17 module tests and 3 TCP standards tests that pass on master, and it moved 62 `tplx`
  and 57 `~tND` fingerprint rows. Slow start did not grow at all: the cwnd-limited gate of
  `Rfc5681CongestionControl` reads `maxPacketsOut`, which nothing sets before commit 16. The
  strategy classes hold their final code, with behaviors of later commits, and that code needs
  state that only later commits maintain. On the source branch nobody saw this, because
  `libINET` did not load at commits 10 to 38 (D-5). The owner chose option A: a stage that says
  refactor keeps master's behavior exactly, and every new behavior comes as its own change, with
  its own evidence. S5 therefore holds commits 11 and 12 only. The content of commit 13 and its
  folded repairs (54, 55, 56, 84) becomes the stages S5b to S5d below.
- **D-7 — Path MTU discovery updates the effective MSS.** Master's ICMP-based PMTUD changes
  `snd_mss` after the establishment, in three places: an ICMP "fragmentation needed" for IPv4 and
  for IPv6, and the probe that restores the original MSS. Source commit 15 sets `snd_effmss` at
  eight places, but not at these three, and nothing else does on the source branch. So after a
  PMTUD reduction the strategies, and later CUBIC, keep counting with the old MSS. The CI job found
  it: at the first S5b head, `examples/inet/tcp_pmtud` `PmtudEnabled` and `PmtudProbe` moved. S5b
  sets `snd_effmss = snd_mss` at the three places; when commit 15 lands (S7), they become
  `calculateEffectiveMss()`. This is a deliberate difference from the source tree.
- **D-8 — Tahoe keeps Limited Transmit.** Source commit 13 gives TcpTahoe its own
  duplicate-ACK path, which counts without the base class: it drops Limited Transmit although
  `limitedTransmitEnabled` names TcpTahoe, and it does not reset the counter on an old ACK that is
  no duplicate. S5c keeps master's path through the base class for Tahoe and changes only its
  reaction at the third duplicate ACK (step 4a). This is a deliberate difference from the source
  tree: `TcpTahoe` keeps `receivedDuplicateAck()` and has no `receivedAckForAlreadyAckedData()`.
- **D-9 — DumbTcp keeps counting duplicate ACKs.** The source branch leaves
  `DumbTcp::receivedAckForAlreadyAckedData()` empty with a `TODO`, so the `dupAcks` statistic of a
  DumbTcp connection stays 0. S5b keeps master's counter; this is a deliberate difference from the
  source tree.
- **D-10 — The gap-fill ACK also covers the first part of a gap, and a data segment can carry
  it.** RFC 5681 section 3.2 asks
  for an immediate ACK when a segment fills "all or part of a gap in the sequence space". The
  source branch sets `ack_now` only when `rcv_nxt` moves past the end of the segment, that is,
  when the segment reaches the buffered data. A segment that fills the first part of a gap, and
  leaves a hole before the buffered data, got a delayed ACK. With SACK, master's SACK branch of
  the connection already sends that ACK at once; without SACK, nothing did. S5d also sets
  `ack_now` when out-of-order data stays above `rcv_nxt` (`TcpReceiveQueue::hasOutOfOrderData()`),
  as Linux does while its out-of-order queue is not empty. The new test `tcp_gapfill_ack_1` shows
  the case: with the source condition, B sends the ACK 200 ms later. The source also sets
  `ack_now` at the end of the segment processing, after `receivedAckForUnackedData()`. A data
  segment that this call sends already carries the new ACK number, so the source then sends a
  second, pure ACK with the same number. RFC 5681 section 4.2 says that a receiver MUST NOT send
  more than one ACK for each incoming segment, except to update the window, and the sender can
  count the second ACK as a duplicate ACK. The CI job found such an ACK in `bulktransfer`
  `inet__lwip`: with the source placement, four rows moved only for these second ACKs
  (`arptest2`, and `bulktransfer` `inet__inet`, `inet_inet_2a`, `inet__lwip`). S5d sets `ack_now`
  directly after the insert into the receive queue, so the data segment clears it, and the four
  rows keep master's values. These are deliberate differences from the source tree.
- **D-11 — A timeout marks the outstanding data lost, also without SACK.** The Linux count of the
  bytes in flight takes the lost bytes out and adds the retransmitted bytes. Linux marks all
  outstanding data lost at a timeout (`tcp_timeout_mark_lost()`) and resets the inferred SACKs of
  Reno (`tcp_reset_reno_sack()`). The source branch marks only with SACK (commit 34). Without the
  marks, the count keeps `snd_max - snd_una` in flight after a timeout: in `tcp_newreno_rto_1`,
  the go-back-N then waits for a new timeout at each hole (2.2 s, 4.2 s, 8.2 s), where with the
  marks it sends at once. S5e marks with and without SACK; without SACK it also clears the inferred
  SACKs and the retransmitted marks.
- **D-12 — A partial ACK of NewReno marks the new head lost.** The source retransmits the new head
  without a mark, so until the next duplicate ACK the count holds the head two times: the original
  and the retransmission. Linux marks it lost (`tcp_newreno_mark_lost()`), as the source does at
  the fast retransmit.
- **D-13 — NewReno's "recover" follows `snd_una` once the cumulative ACK has passed it.** The source
  keeps "recover" at the value of the last fast retransmit or timeout, and it starts at 0, because
  the state is made before the connection chooses its ISS. After 2^31 bytes without a fast
  retransmit or a timeout, or with an ISS from 2^31 on, `seqGreater(snd_una - 1, recover)` turns
  false, and NewReno no longer enters fast retransmit. Master's "recover" trails `snd_una` after
  every ACK of new data, which also ends the protection after a timeout (RFC 6582 step 4). S5e
  starts "recover" at the ISS and moves it only after `snd_una` has passed it: the check of step 2
  then gives the result of a fixed "recover", without the turn.
- **D-14 — Limited Transmit without SACK stays master's RFC 3042 code.** Master sends one new
  segment for each of the first two duplicate ACKs, while the outstanding data stays within cwnd
  plus two segments. The source sends what cwnd minus the bytes in flight allows, which is more
  than one segment when cwnd has room.
- **D-15 — The duplicate-ACK counter keeps counting in a recovery.** The source stops the counter
  while `lossRecovery` is set. In S5e the recovery strategies decide by `lossRecovery`, so the
  counter needs no stop, and the `dupAcks` statistic keeps master's meaning.
- **D-16 — NewReno's full ACK keeps option (1) of RFC 6582.** The source sets cwnd to
  ssthresh (option (2)). Without Proportional Rate Reduction, the bytes in flight at the end of a
  recovery can be far below ssthresh, and option (2) then sends a burst; RFC 6582 asks for a measure
  against it. Master's option (1), `min(ssthresh, FlightSize + SMSS)`, has none. The plan expected
  PRR (S9) to keep the bytes in flight near ssthresh, but the source runs PRR only in the SACK
  recovery (`Rfc6675Recovery`), not in NewReno's recovery without SACK. So NewReno without SACK
  keeps option (1) after S9 too, and the final tree differs from the source here.
- **D-17 — The inferred SACKs follow Linux's counts.** Two parts of the emulation of S5c (from
  source commit 39) counted the pipe wrong once 2c reads it. First, when an ACK
  leaves the mark of a duplicate ACK on the first unacknowledged segment, `discardUpTo()` (S5c,
  from source commit 39) marked that segment lost and moved the mark to the next one. The pipe
  then counted one segment too few: once for the moved mark and once for the loss mark. After a
  timeout and one duplicate ACK of a go-back-N copy, the sender sent a segment more than cwnd
  allowed (`tcpclientserver` `inet-reno` run 0 at t=1.4179 s). Linux `tcp_remove_reno_sacks()`
  only reduces `sacked_out` by the acknowledged segments but one. S5e moves the mark and does not
  mark the head lost. Second, `addInferredSack()` put the mark on a lost segment and cleared its
  loss mark. After a timeout, when all outstanding data is lost, the reset of the inferred SACKs at
  the next ACK of new data then left that segment neither lost nor SACKed, and the pipe counted it
  again: the sender sent less than cwnd allowed (`arptest` at t=2.1033 s, `styling` `Annotation`
  at t=2.8848 s). Linux limits `sacked_out` so that it and `lost_out` do not exceed `packets_out`
  (`tcp_limit_reno_sacked()`). S5e puts an inferred SACK only on a segment that is neither SACKed
  nor lost.
- **D-18 — Outside a recovery, the inferred SACKs go with the duplicate-ACK counter.** INET counts
  duplicate ACKs as BSD does: an old ACK that is no duplicate, for example data of the peer, resets
  the counter. Linux does not reset its count there, and it removes the inferred SACKs only at an ACK
  of new data (`tcp_reset_reno_sack()`). With both rules together, a two-way transfer collected
  marks from several short series of duplicate ACKs, the counter did not reach the fast retransmit,
  and the pipe gave more room than the two segments of RFC 3042 (`bulktransfer` `inet__inet`: five
  marks at the third duplicate ACK at t=1.2825 s). S5e removes the marks outside a recovery both at
  an ACK of new data and when the counter resets; in a recovery the marks stay.
- **D-19 — S7 holds the sizing parts of source commits 15 and 16; their other parts go to the
  stages of their features.** Both commits are what remained of one commit that carried many
  features (their messages say so). S7 takes the effective MSS, the announced MSS, and the peak segments in
  flight with the cwnd-limited slow start. The other parts go to
  S8 (`seedRttFromHandshake`, the TCP_INFO time counters, the `sndMax` signal, the STATUS fields
  `sndEffMss` and `advmss`, with the tests of STATUS), S11 (the RST
  for a SYN-ACK with an invalid TSecr, the options of the crossing SYN-ACK of a simultaneous open,
  the `forked` flag, `syncookiesAlways`), S12 (the Nagle exemption of the SYN-ACK slot, the data
  retransmission in SYN_RCVD, `peerAdvertisedMss`, `alignOptions`, `sendMssOption`, the place of
  the timestamp option in the SYN, and the default send MSS when the SYN has no MSS option, RFC
  9293 MUST-15, which the source claims but handles only for an option value of 0), S16
  (`TCP_MAXSEG`, with the other socket options, whose tests are packetdrill tests; the
  silly-window hold of the sender, which needs `max_window` from source commit 69: without it, a
  peer whose window never reaches one MSS receives nothing) and S19
  (`mss = -1` with the default of the address family, the parameters `initialWindow` and
  `initialSendSequenceNumber` with the defaults of `initialWindow` and `seedRttFromHandshake`,
  and source commit 53). For `mss = -1`, the source leaves `snd_mss` unresolved when the SYN of
  the peer arrives, so a passive side sends segments as large as the peer announces; S19 must
  resolve the default first.
- **D-20 — The cwnd-limited gate of slow start uses the peak of each window of data.** The source
  keeps `maxPacketsOut` as the peak of the whole connection, so after one busy phase the gate stays
  open: a flow that is application-limited later grows cwnd in slow start without using it. Linux
  `tcp_cwnd_validate()` keeps `max_packets_out` per window of data. S7 keeps it per window and
  counts the segments with the effective MSS. In `tcp_cwnd_limited_2`, with the peak of the whole
  connection, a burst after an idle time and single segments starts with seven segments; per
  window it starts with two.
- **D-21 — The ACK that ends a fast recovery does not grow cwnd.** On the source branch,
  `TcpClassicAlgorithmBase` gives this ACK first to the recovery and then, because the recovery has
  ended, also to the window growth, as Linux does. RFC 5681 step 6 says that this ACK MUST set cwnd
  to ssthresh, and RFC 6582 step 3 sets cwnd at the full ACK in the same way. Master's routing
  does that, and S6 keeps it.
- **D-22 — DCTCP joins the shared ACK path in three behavior changes and a refactor.** Source
  commit 65 calls the move a refactor. On this tree, `DcTcp`'s own copy of the ACK path did not get
  the slow start of S7 or the reset of the inferred SACKs of S5e, and with SACK it ran the steps of
  RFC 3517 on top of a test of the duplicate-ACK counter, so cwnd grew in the recovery. S6 makes
  each difference one commit with its own test, in the copy, and then removes the copy, which
  changes no behavior.
- **D-23 — DCTCP counts the acknowledged bytes only outside a fast recovery until S13.** RFC 8257
  section 3.3 counts the acknowledged and the marked bytes at each ACK, and Linux updates alpha at
  each ACK (`dctcp_update_alpha()` as `in_ack_event`); only the cut of cwnd waits until no
  recovery runs. Master without SACK and the source branch count only outside a recovery. Master
  with SACK also counted the ACKs in a recovery after the first partial ACK, because of the counter
  test that S6 removes, which also grew cwnd there. S6 keeps the count of the source. The repair
  needs a hook of the shared path at each ACK in a recovery. The owner chose S13, where the AccECN
  branch changes the same count (2026-10-08).
- **D-24 — The S8 features land off; S19 turns them on.** The source turns RACK, the learning of
  the reordering degree and the handshake RTT seed on in their own commits. RACK and the learning
  then changed only connections with SACK, which the source turns on in commit 40 with the other
  modern defaults. S8 lands them with defaults that keep master's behavior:
  `lossDetectionMode = "dupthresh"`, `adaptiveReorderingEnabled = false`, and
  `seedRttFromHandshake = false` (D-19). The tests of S8 select each feature. S19 sets the source
  defaults, one commit for each, with its moved rows. A behavior that has no parameter, such as
  the count of the bytes in flight with SACK (B8), changes in S8.
- **D-25 — A retransmission that ends at `snd_max` keeps its last whole segment.** Source commit 17
  cuts a retransmission at the last original segment start inside one SMSS. When the
  retransmission ends at `snd_max`, it ends with a whole segment, but the source cut it there too:
  three dropped segments of 300 bytes came back as 600 and then 300 bytes. Linux joins whole
  segments that fit into one SMSS (`tcp_retrans_try_collapse()`) and sends 900 bytes. S8 cuts
  only when the retransmission ends before `snd_max`; `tcp_rexmit_boundaries_2` fails with the
  source form.
- **D-26 — In RACK mode, the duplicate-ACK count does not start a recovery.** RFC 8985 (step 4 of
  the detection) enters the fast recovery by RACK's own marks: when DupThresh segments are SACKed
  and no reordering was seen, the reordering window is 0, and once reordering was seen, "RACK does
  not trigger fast recovery based on DupThresh". Linux `tcp_time_to_recover()` skips the
  duplicate-ACK heuristic with RACK. The source kept step (1) of RFC 6675 (`DupAcks >= DupThresh`)
  in RACK mode, so a reordered segment was retransmitted although RACK waited for it. S8 skips
  that step in RACK mode; `tcp_reordering_1` fails without this.
- **D-27 — The handshake RTT seed reads its parameter on both sides.** Source commit 16 seeds the
  passive side when `seedRttFromHandshake` is set; source commit 26 (S12) seeds the active side
  always. So the source test `tcp_info_fields_1`, which sets the parameter false through
  `tcp-legacy.ini`, sees a first RTO of 1 s and expects `backoff=2` at t=4.2 s. S8 seeds both
  sides only with the parameter, and the test expects `backoff=1`, the one timeout at t=4 s. S12
  must keep the check of the parameter on the active side.
- **D-28 — The delivered bytes count each byte once (RFC 6937 DeliveredData).** PRR sizes cwnd
  by the bytes that each ACK delivers: the change of `snd_una` plus the signed change of the
  SACKed bytes. Master (S8) counted only the newly SACKed bytes, so a cumulative ACK delivered
  nothing, and the STATUS field `deliveredBytes` was too small. Source commit 39 adds the change of
  `snd_una`, but it counts again the bytes that a SACK reported before, so the ACK that fills a
  hole delivers the whole SACKed range a second time, and PRR sends too much on it. S9 subtracts
  the SACKed bytes that the cumulative ACK covers, as Linux counts each segment once.
- **D-29 — The undo counts each D-SACK and each retransmission of the episode.** Linux takes one
  retransmission back for each D-SACK of data above the undo marker, on any ACK
  (`tcp_check_dsack()`), and counts each retransmitted segment. The source counted a D-SACK only on
  an ACK of new data, but the D-SACK of a needless retransmission usually comes on a duplicate
  ACK, after the original. And the retransmissions of step (C) of RFC 6675 did not count, so one
  D-SACK could undo a recovery that also repaired a real loss. S10 counts both, as Linux does;
  with the source form, `tcp_undo_1` does not undo and `tcp_undo_2` undoes wrongly.
- **D-30 — F-RTO takes no data that the receiver SACKed before the timeout as new evidence.**
  Linux sets `FLAG_ORIG_SACK_ACKED` only for a segment that it sent once and that was not SACKed,
  and it keeps the SACK marks at a timeout. The timeout of INET clears them (RFC 2018 section 8).
  Source commit 86 adds `everSacked` for the SACK blocks; S10 applies it also to the cumulative
  ACK. `tcp_frto_2` fails without this.
- **D-31 — A timeout starts a new undo context.** The source kept the context while `undoMarker`
  was set, also the context of an older episode that no undo had ended, with its old cwnd and its
  unconfirmed retransmissions. Linux `tcp_enter_loss()` starts a new context, except during a fast
  recovery and at a repeated timeout with no new ACK. S10 does the same. For this,
  `TcpAlgorithmBase` no longer ends the fast recovery before the timeout hook of the recovery;
  `TcpClassicAlgorithmBase` ends it after the hook. `tcp_undo_3` fails with the source form.
- **D-32 — F-RTO follows steps 1 and 2.b of RFC 5682.** The source opened an episode at each
  timeout and kept it open until the ACK covered the whole flight. RFC 5682 step 1 opens none
  during a loss recovery, and step 2.b ends the episode at the first ACK of new data when the
  sender sends no new data; Linux `tcp_process_loss()` does the same. S10 sends no new data at
  step 2.b, so its episode ends there. With the source rules, a SACK of late segments after a real
  loss undid the reduction (`tcp_frto_3`), and so did a SACK after a timeout in a fast recovery
  (`tcp_frto_4`). A delay of the data path is then found only by the D-SACK undo; the new data of
  step 2.b can come in a later stage.
- **D-33 — The repair of the empty segment after a timeout comes forward from S16.** After a
  timeout with SACK, `sendSegment()` moves `snd_nxt` over SACKed and retransmitted data. At the
  end of the send queue, master made a segment of zero bytes and stopped with "Returning an empty
  chunk is not allowed". A usual loss of two segments causes this (`tcp_sack_8`), and the F-RTO
  tests of S10 meet it. Source commit 37 returns zero there, but then its send loops repeat for
  ever. S10 brings the zero return and stops each send loop at it.
- **D-34 — A duplicate ACK after a timeout in a fast recovery starts a new recovery; a later stage
  repairs it.** RFC 6675 section 5.1: after a timeout during a loss recovery, "a new recovery phase
  (as described in Section 5) MUST NOT be initiated until HighACK is greater than or equal to the
  new value of RecoveryPoint". Master and the source start a new fast recovery at the next
  duplicate ACK, because the duplicate-ACK count or `IsLost()` is still above the limit;
  `tcp_frto_4` shows it at t=4.002. The repair changes master's behavior with SACK, so it is not
  part of S10. The proposal is S15, which changes the loss marking at a timeout.
- **D-35 — The tail loss probe follows RFC 8985 sections 7.2 and 7.4.2.** Four deviations from
  source commits 25, 76 and 85. (1) An ACK of new data also arms the probe timeout (section 7.2,
  Linux `tcp_set_xmit_timer()`). In the source only a send armed it, and the RTO restart at each
  ACK cancelled it, so the RTO repaired a lost tail after the ACKs of a flight (`tcp_tlp_1`).
  (2) A retransmitted probe shows a loss only at an ACK above it; a D-SACK that ends at the probe,
  or a duplicate ACK without SACK, shows no loss, and each ACK counts (section 7.4.2, Linux
  `tcp_process_tlp_ack()`). The source decided at the first ACK that reached the probe, so a late
  original reduced cwnd (`tcp_tlp_2`). (3) The response uses the ssthresh of the flavour for a
  fast recovery ("equivalent to a fast recovery", Linux `tcp_init_cwnd_reduction()`), so CUBIC
  keeps 0.7 (`tcp_tlp_6`); the source halved the bytes in flight. (4) No probe timeout during an
  RTO recovery (section 7.2); the recovery point of the timeout, which F-RTO keeps, shows it. The
  processing runs in `TcpAlgorithmBase` for each ACK, so `TcpCubic` needs no call of its own
  (source commit 14, part).
- **D-3 — The socket contract lands alone (S3).** Commit 2 is contract surface only, and the
  features of S7 to S18 use it. A split of commit 2 into one part for each feature is possible,
  but it costs more than it gives.

## 6. Results

### S1 — `topic/tcp-header-options` — landed 2026-10-02

Commit 52 (`serialize the experimental TCP Fast Open option (kind 254)`) repairs a gap of
commit 1: commit 1 reads option kind 254 but cannot write it. Master does not have this code, so
rule 4 folds 52 into 1. The stage is then one commit with four unit tests:
`tcp_accecn_option_wire_1`, `tcp_ae_bit_wire_1`, `tcp_fastopen_wireformat_1`,
`tcp_fastopen_exp_wireformat_1`.

The owner reviewed S1 and approved it on 2026-10-02.

Evidence, debug build against `omnetpp-6.x`, at the stage head:

| Suite | Result |
| --- | --- |
| unit | 114 PASS, with the four new wire tests |
| serializer | 4 PASS, with `serializer_chunk_roundtrip` and the AE bit in its filler |
| module | 346 PASS |
| protocol `self/` | 21 PASS |
| protocol `tcp/` | 25 PASS, 2 FAIL (expected), 306 SKIP (expected: packetdrill needs inet-gpl) |
| static gates | classification, commits, links, seals and source seals pass |

`check-series-builds.sh` did not run. The plan commit changes no source, and the head build
covers the only source commit. The fingerprint suite did not run for S1. S1 and S2 both claim
that no row moves, so the fingerprint run at the head of S2 checks both stages.

### S2 — `topic/tcp-tidy-ups` — landed 2026-10-02

Four commits by Rudolf Hornig: commits 4, 5, 6 and 21. The owner reviewed and approved S2 on
2026-10-02, and asked to land it before the tests finished. The code is the same as on the source
branch. The messages changed (rule 6): the "re-cut" paragraphs and an old "Verified:" line left,
and the rename commit no longer says that an SCTP rename follows, because that commit left the
TCP branch (SCTP keeps the old callback names).

| Suite | Result |
| --- | --- |
| series builds | all four commits build |
| unit | 114 PASS |
| serializer | 4 PASS |
| module | 346 PASS |
| protocol `self/` and `tcp/` | as S1: 21 PASS; 25 PASS, 2 FAIL (expected), 306 SKIP (expected) |
| fingerprint | not compared; see step 5 of section 4 for the method that S3 starts |

### S3 — `topic/tcp-socket-contract` — landed 2026-10-05

One commit by Rudolf Hornig: commit 2, the socket contract. The message no longer says "later
commits" but names the later stages of this plan, and its old "Verified:" line left. Commit 3
left the stage (D-4).

| Suite | Result |
| --- | --- |
| series builds | commit 2 builds |
| unit | 114 PASS |
| serializer | 4 PASS |
| module | 346 PASS |
| protocol `self/` and `tcp/` | as S1 |

The suites ran with commit 3 on top. Commit 3 changed only NED properties, so the results apply to
commit 2 alone.

The owner approved S3 on 2026-10-05. Before the landing, S3 moved onto two QUIC fixes that reached
master (`f6a48554c3`, `8c29416d8d`). They change no TCP file.

Fingerprints, compared with master `dabcd78282` in the same environment: **no row differs** in the
753 rows of `tplx` and the 638 rows of `~tND`. The master baseline in this environment is
`tplx`: 475 PASS, 259 FAIL, 19 ERROR; `~tND`: 262 PASS, 364 FAIL, 12 ERROR. The ERROR rows need
features that this build does not have (for example `TcpLwip`) or a serializer that does not
exist (`GpsrBeacon`). The logs are `/var/tmp/claude-staged/fp-<head>-<ingredient>.log`, and
`/var/tmp/claude-staged/fpcmp.py` compares two of them.

The runner checks one ingredient at a time. With two ingredients in one run, INET's runner stops
at once: `python/inet/test/fingerprint/task.py:165` uses the undefined name `sim_time_limit`. This
defect is not TCP, so this plan does not repair it.

### S4 — `topic/tcp-recovery-split` — landed 2026-10-05

The owner reviewed and approved S4, and decision D-5, on 2026-10-05.

Four commits by Rudolf Hornig: commits 7, 8 and 9, and commit 10 with commit 50 folded in.

- **Commit 10 changes form (D-5).** It no longer deletes `TcpConnectionSackUtil.cc`. Its kind
  changes from `refactor` to `behavior.add`, because the class is new code that nothing calls
  yet, and the old code stays.
- **Commit 10 is not a move.** Against the connection's copy, `Rfc6675Recovery` differs in
  `isLost()` (the RFC 6675 threshold), `setPipe()` (the scan end), `nextSeg()` rule (2),
  `processSACKOption()` (delivered octets), `addSacks()` (the `rcv_nxt` condition, the AccECN
  room, no NOP options), and it runs the RFC 6675 steps of each ACK itself. Its message lists
  these points. They take effect in S5, when the flavours select the class, and the
  fingerprints of S5 must explain them.
- **Commit 50 folds into commit 10 (rule 4).** It repairs the wrap of `snd_wnd - pipe` in
  `nextSeg()` rule (2) of the new class. Master's connection copy has the same unsigned
  subtraction. The fix stays out of the connection's copy: the flavours stop calling that copy in
  S5. On master, the wrap makes rule (2) report unsent data as sendable; with the window guard of
  the new class, it stopped recovery until the timeout.
- **Messages.** The paragraphs about the source branch history left (rule 6). Commit 8 says that
  the flavours still have their own copies.
- **Review points that this stage does not change.** Commits 8 and 10 declare state fields that
  later features use: `maxPacketsOut`, `snd_effmss`, the AccECN fields, `deliveredBytes`,
  `lastRcvdTSecr`, `minRtt`, `prrDeliveredMark`. `Rfc6675Recovery::addSacks()` already holds the
  AccECN block reduction. These come from the source branch, where the strategy classes hold their
  final code.

Evidence, debug build against `omnetpp-6.x`, at the stage head:

| Suite | Result |
| --- | --- |
| series builds | all four commits build, and no commit leaves an undefined `inet::` symbol |
| unit | 114 PASS |
| serializer | 4 PASS |
| module | 348 PASS, with the two QUIC tests of master |
| protocol `self/` and `tcp/` | as S1 |
| fingerprint | no row differs from S3: 0 of 753 `tplx` rows, 0 of 638 `~tND` rows |

The first module run had two QUIC failures. The cause was the build, not S4: the reused build
tree got a fresh timestamp after the QUIC fixes of master were checked out, so `make` kept the old
QUIC objects. After a rebuild of the three QUIC sources, all 348 tests pass. The fingerprints of
both builds are equal, so the QUIC fixes move no row, and the S4 run is the baseline of S5.

### S5 — `topic/tcp-flavour-split` — landed 2026-10-05

The owner reviewed and approved S5, and decision D-6, on 2026-10-05. Each commit of S5b to S5d
that moves a fingerprint gets its new stored values from the CI environment (owner, 2026-10-05).

Two commits by Levente Meszaros: commits 11 and 12, the move and the rename of `TcpBaseAlg` and
`TcpTahoeRenoFamily`. The first cut of S5 held commit 13 with commits 54, 55, 56 and 84 folded in,
and with `snd_effmss = snd_mss` set before `established()`. That cut failed: see D-6. It is kept
as the local branch `s5-folded-13`, for the stages S5b to S5d. Commit 60 moves to S6, because its
subject is `TcpCubic`.

| Suite | Result |
| --- | --- |
| series builds | commit 12 builds, with no undefined `inet::` symbol; commit 11 does not build alone, and PR-SERIES-BUILDS exempts it, because it only moves files |
| unit | 114 PASS |
| serializer | 4 PASS |
| module | 348 PASS |
| protocol `self/` and `tcp/` | as S1 |
| fingerprint | no row differs from S4: 0 of 753 `tplx` rows, 0 of 638 `~tND` rows |

Commit 12 owes `whatsnew migration`: the old names stay as deprecated aliases, and the release
note (commit 66) lands with S20. The messages of both commits name this plan, and commit 12 no
longer says that "the architecture commit after it" touches the same files.
The move broke four links of the TCP evidence documents (`notes.md`, `results.md`) that point into
`TcpBaseAlg.cc`; commit 12 points them at `TcpAlgorithmBase.cc`, at the same line numbers, which
hold the same code.

### S5b — `topic/tcp-flavour-strategies` — landed 2026-10-06

The owner reviewed and approved S5b on 2026-10-06. Before the landing, S5b moved onto the new
rules of master (`03e927c18a`): the opening of each body now answers the reviewer's questions.

Six refactor commits by Levente Meszaros, written from the source commit 13 and the master code. The
new rules of master (`d9e14c5af2`, PR-SPLIT-ONE-CHANGE and PR-SPLIT-SIZE) divide the work wherever a
part stands alone:

| Commit | What it does | B of D-6 |
| --- | --- | --- |
| `tcp: refactor: count duplicate ACKs in the algorithm` | the connection passes every old ACK to `receivedAckForAlreadyAckedData()`; `TcpAlgorithmBase` counts with master's test, in master's order | B1 |
| `tcp: refactor: share the per-segment send times in TcpAlgorithmBase` | one `sentInfo` list in the base state, filled by the base class; Vegas and Westwood read it | B2 |
| `tcp: refactor: derive TcpTahoe from TcpAlgorithmBase` | Tahoe leaves the classic base class and keeps its own logic | B3 |
| `tcp: refactor: start the effective MSS as the negotiated MSS` | `snd_effmss = snd_mss` at establishment and where PMTUD changes `snd_mss` (D-7); nothing reads it yet | B3 |
| `tcp: refactor: run Reno on a congestion control and a recovery strategy` | `TcpClassicAlgorithmBase` drives `Rfc5681CongestionControl` and `Rfc5681Recovery`, which carry master's Reno | B3 |
| `tcp: refactor: run NewReno on the RFC 6582 recovery strategy` | `Rfc6582Recovery` carries master's NewReno | B3 |

The S5b code holds temporary forms. Each one ends in a later change commit, which brings the
code of the source branch and the evidence of that change:

| Temporary form in S5b | Source form | Ends in |
| --- | --- | --- |
| `TcpAlgorithmBase::sendData()` is public; the recovery strategies send through it | they send with `conn->sendData(cwnd)`; `sendData()` is protected | S5c |
| `isInFastRecovery()`: Reno answers `dupacks >= dupthresh` | `lossRecovery` for every flavour | S5c (B5) |
| `processEce()`: Reno only, master's arithmetic | every classic flavour, commit 54 | S5c (B10) |
| `ackProcessed()`: Reno's SACK step, NewReno's `recover = snd_una - 2` | gone: SACK recovery in `Rfc6675Recovery`; `recover` set only at a fast retransmit and a timeout | S5d (B12), S5c (B7) |
| `Rfc5681CongestionControl`: `+SMSS` per ACK in slow start | `+min(acked, SMSS)` and the cwnd-limited gate | S7 (B9) |
| `Rfc5681Recovery`, `Rfc6582Recovery`: master's Reno and NewReno | the RFC 5681 and RFC 6582 strategies of the source | S5c (B4 to B8) |
| ssthresh from `calculateSsthresh(min(cwnd, snd_wnd))` | from FlightSize | S5c (B6, commit 70) |
| `TcpTahoe` on `TcpAlgorithmBase` with master's logic | the Tahoe of commit 13 | S5c (B11) |
| `snd_effmss = snd_mss` at establishment and at the three PMTUD places | `calculateEffectiveMss()`, at the PMTUD places too (D-7) | S7 (commit 15) |
| `DumbTcp` counts duplicate ACKs for the `dupAcks` statistic | it does not count | S5c |
| the RTT estimator, `rttvar` starts at 3/4 | the estimator on the state fields, `rttvar` starts at 0, `minRtt` | S8 (commit 17) |

Two details of the output change, and the commit messages state them: NewReno's window
growth logs Reno's wording, and Reno no longer records an unchanged `ssthresh` at each growth
step. `ssthresh` is a vector statistic only; its values over time stay the same.

Evidence, debug build against `omnetpp-6.x`:

| Suite | Result |
| --- | --- |
| builds | each commit builds alone, with no undefined `inet::` symbol |
| unit, serializer | 114 PASS and 4 PASS at each commit |
| module | 348 PASS at each commit, with no trace re-recorded |
| protocol `self/` and `tcp/` | as master at each commit |
| fingerprint, CI | the job passes at every commit: 1752 tests, only the expected error; the first head, before D-7, failed `tcp_pmtud` `PmtudEnabled` and `PmtudProbe` |
| fingerprint, local | at the head before D-7, no row differs from master: 0 of 753 `tplx`, 0 of 744 `~tNl`, 0 of 638 `~tND` |

At master `649d4756ae` the CI job passed: 1752 tests, with only the expected
`ethernet-nonstandardspeed` error. The local `~tNl` baseline comes from `inet-tcp-tidy-ups` at
`dabcd78282`, which has master's TCP code.

### S5c — `topic/tcp-classic-recovery` — landed 2026-10-06

The owner reviewed and approved S5c on 2026-10-06, with D-8 ("the more important goal is
correctness and passing the protocol tests"), D-9 and the order S5d before S5e. Before the landing,
S5c moved onto an IEEE 802.11 fix of master (`93210f4955`), which also moves the row
`showcases/visualizer/canvas/styling` `Annotation`; the FlightSize commit carries its new value. The
CI job ran again at every commit of S5c on the new base: only that row differed, at the FlightSize
commit, with the same `~tNl` and `~tND` as on the old base.

The source branch changes the recovery of Reno and NewReno to the Linux model: the window is not
inflated, and duplicate ACKs make room in the pipe instead. That model needs parts that the source
branch brings much later: the connection sends against the bytes in flight (commit 16, S7), and a
duplicate ACK without SACK counts as an inferred SACK (commit 39, S17). S5c brings the behavior
changes that stand alone, and the preparation of the pipe accounting. Each `change` commit carries
the moved fingerprints of the CI job and an explanation of each moved row and trace:

| Commit | B of D-6 | Source | Moved CI rows |
| --- | --- | --- | --- |
| `tcp: change: count a duplicate ACK as RFC 5681 defines it` | B4 | 13 | 2: `bulktransfer` `inet__inet`, `inet_inet_2b` |
| `tcp: change: set ssthresh from FlightSize, as RFC 5681 equation (4) says` | B6 | 13, 70 | 42 Reno and NewReno runs; 4 stress test traces |
| `tests: add: let the TCP module tester mark a segment CE` | — | 41 (part) | — |
| `tcp: change: react to an ECN-Echo in every classic flavour` | B10 | 54 | 1: `dctcp` `TcpRenoIncast` |
| `examples: fix: let the inet-tahoe configuration run TcpTahoe` | — | new | 7: `tcpclientserver` `inet-tahoe` |
| `tcp: change: let Tahoe answer the third duplicate ACK as a timeout` | B11 | 13 | 6: `tcpclientserver` `inet-tahoe` |
| `tcp: refactor: send against the bytes in flight that the algorithm reports` | — | 16 (part) | — |
| `tcp: refactor: keep the retransmission scoreboard without SACK` | — | 39 (part) | — |
| `tcp: add: count lost, SACKed and retransmitted bytes, and infer a SACK` | — | 13, 39 (part) | — |

The pipe accounting itself (B5, B7, B8) moves to S5e, after S5d. With SACK, Reno and NewReno still
run master's SACK path in the same recovery classes; when S5d has moved SACK recovery into
`Rfc6675Recovery`, those classes run only without SACK, and S5e changes them without a condition
on `sack_enabled`.

New tests: `tcp_ecn_reno_1` and `tcp_ecn_newreno_1` (the NewReno one fails before the ECN commit),
and `tcp_tahoe_fastrexmit_1` (it fails before the Tahoe commit: the second loss waits for a
timeout). Each failure was checked on the commit before.

Evidence, debug build against `omnetpp-6.x`: each commit builds alone with no undefined `inet::`
symbol; unit 114, serializer 4 and the module suite (351 tests at the head) pass at each commit;
the protocol tests are as master; the CI fingerprint job passes at each commit that changes code,
after the commit's own new values.

The configuration `inet-tahoe` of `examples/inet/tcpclientserver` set `tcpAlgorithmClass =
"TcpReno"` since 2017, so no fingerprint row ran TcpTahoe. The owner agreed on 2026-10-06 to fix
it in S5c, before the Tahoe change, so that the Tahoe change moves rows of its own.

Source commits 55, 56, 71 and 72 do not land here: 55 repairs the duplicate-ACK counting that the
source commit 13 broke for Vegas and Westwood, which S5b never broke; 56, 71 and 72 belong to the
pipe accounting (S5e).


### S5d — `topic/tcp-sack-recovery` — landed 2026-10-07

The owner reviewed and approved S5d on 2026-10-07. Before the landing, S5d moved onto a
documentation commit of master (`c3ab2dda13`), which changes only `doc/project/`.

S5d moves the SACK loss recovery of Reno, NewReno and DCTCP into `Rfc6675Recovery` (S4 code), and
removes master's rule that sent an immediate ACK while the connection's own sender recovers. Each
`change` commit carries the moved fingerprints of the CI job and an explanation of each moved row
and trace:

| Commit | B of D-6 | Source | Moved CI rows |
| --- | --- | --- | --- |
| `tcp: add: the ssthresh of a SACK-based fast recovery` | B12 | 13 | — |
| `tcp: change: let Reno with SACK recover as RFC 6675 says` | B12, B16 | 13 | 1: `bulktransfer` `inet_inet_2b` |
| `tcp: change: let every flavour with SACK recovery use SACK` | B12 | 10, 13 | — |
| `tcp: change: send an immediate ACK when a segment fills a gap` | B13 | 13 | 1: `bulktransfer` `inet_inet_2b` |
| `tcp: refactor: remove the SACK path of the RFC 5681 recovery` | — | 13 | — |

`Rfc6675Recovery` replaces master's RFC 3517 steps on top of Reno's window inflation: it enters
recovery also when `IsLost()` holds, it sets cwnd to ssthresh without inflation, and it sends what
cwnd minus the pipe allows. It sends Limited Transmit data only when `limitedTransmitEnabled` is
set. After an ACK, `ensureRexmitTimerArmed()` starts the retransmission timer when data is
outstanding, because step (C) sends without it. DCTCP calls the same class.

Master stopped the simulation with an assertion when `sackSupport` was set for any flavour other
than TcpReno. `TcpAlgorithm::supportsSackRecovery()` (source commits 10 and 13) now decides:
Reno, NewReno and DCTCP recover with SACK; for Tahoe, Vegas, Westwood, DumbTcp and
TcpNoCongestionControl, the connection writes a warning and does not offer SACK, as on the source
branch.

The gap-fill ACK differs from the source tree (D-10). Master's rule looked at the wrong direction
of the connection; RFC 5681 asks for an immediate ACK when a segment fills all or part of a gap.

New tests: `tcp_newreno_sack_1`, `tcp_dctcp_sack_1` and `tcp_tahoe_sack_1` (each stopped at the
assertion before the third commit), and `tcp_gapfill_ack_1` (with the source condition, B sends
the ACK 200 ms later). `tcp_sack_3`, `tcp_fastrexmit_1`, `tcp_tahoe_fastrexmit_1` and the four
stress tests have new traces; the commit messages explain them.

Evidence, debug build against `omnetpp-6.x`: each commit builds alone with no undefined `inet::`
symbol, and the TCP module tests pass at each commit (352 at the first commit, 356 at the head);
unit 114, serializer 4 and the TCP protocol tests pass at the head, as on master. The CI
fingerprint job passes at each commit, after the commit's own new values.

### S5e — `topic/tcp-pipe-recovery`: the steps

Without SACK, master's Reno and NewReno inflate cwnd in fast recovery, as RFC 5681 and RFC 6582
describe. The source branch counts the pipe instead, as Linux does: each duplicate ACK takes one
segment out of the bytes in flight (an inferred SACK), the retransmitted head is marked lost, and
cwnd stays at ssthresh. The sending room, cwnd minus the pipe, is the same as the RFC's inflated
cwnd minus FlightSize. The later stages need this count: PRR (S9) reduces the window against it.

The count reads the loss marks of the retransmission queue. After a timeout, Linux marks all
outstanding data lost and clears the retransmitted marks (`tcp_timeout_mark_lost()`), so the
retransmissions of the go-back-N are the only bytes in flight. The source branch does this from
commit 34 (S15) on, and only with SACK. Without these marks, the count keeps `snd_max - snd_una`
in flight after a timeout, and the go-back-N stops until a cumulative ACK arrives. So S5e brings the
loss marking forward, for connections with and without SACK.

| Step | Commit | B of D-6 | Source |
| --- | --- | --- | --- |
| 1 | add: a timeout marks the outstanding data lost; nothing reads the marks yet | — | 34 (part) |
| 2a | change: the fast recovery is marked by its flag, not by the duplicate-ACK counter | B5 | 13, 56 |
| 2b | change: NewReno keeps "recover" from the fast retransmit and the timeout | B7 | 13 |
| 2c | change: Reno and NewReno without SACK count the pipe instead of inflating cwnd | B5, B7, B8 | 13, 39 (part), 72 |

Source commits 56, 71 and 72 repair the source's own recovery code (rule D-1), so their repairs are
in step 2 from the start. The order can change when a step shows that it depends on a later one.

With SACK, the count of the bytes in flight stays `snd_nxt - snd_una` in S5e. The Linux count needs
loss marks during the recovery, and with SACK these come from RACK (commit 17). So B8 with SACK
moves to S8.

### S5e — `topic/tcp-pipe-recovery` — landed 2026-10-08

The owner reviewed and approved S5e on 2026-10-08.

S5e changes the recovery of Reno and NewReno without SACK from the inflated window of RFC 5681 and
RFC 6582 to the pipe accounting of the source branch. Step 2 of the steps above became three
commits, because two of its behaviors stand alone and repair master defects:

| Commit | B of D-6 | Source | Moved CI rows |
| --- | --- | --- | --- |
| `tcp: add: mark the outstanding data lost at a timeout` | — | 34 (part) | — |
| `tcp: change: mark the fast recovery by its flag, not by the counter` | B5 | 13, 56 | 17 |
| `tcp: change: keep NewReno's "recover" after a timeout` | B7 | 13 | — |
| `tcp: change: count the pipe without SACK, instead of inflating cwnd` | B5, B7, B8 | 13, 39 (part), 72 | 4 |

The commit messages explain each moved row and trace. In short: master's Reno left the fast recovery
without deflation when data of the peer reset the duplicate-ACK counter, and after a timeout in fast
recovery it set cwnd to ssthresh at the next ACK; NewReno lost its protection against a fast
retransmit after a timeout, and at a partial ACK after lost duplicate ACKs its cwnd became 0. With
the pipe, cwnd minus the bytes in flight gives the room of the inflated window; only a fast
retransmit soon after a timeout gets less room, as in Linux.

The emulation of the inferred SACKs from S5c needed two repairs before the count could read it, and
the count needed loss marks at a timeout without SACK; decisions D-11 to D-18 give these and the
other differences from the source tree. B8 with SACK moves to S8 (RACK).

New tests, each checked to fail on the commit before: `tcp_fastrexmit_rto_1` (a timeout in Reno's
fast recovery), `tcp_newreno_rto_1` (duplicate ACKs of the go-back-N after a timeout) and
`tcp_newreno_partial_1` (a partial ACK after lost duplicate ACKs).

The standards test `rfc/Rfc5681FastRetransmit` read the published cwnd, which the pipe model
keeps at ssthresh. The pipe commit brings its form from source commit 80 (S19): a guard judges
steps 3 to 5 by the new data that host A sends in recovery. Its ini keeps Reno without SACK; the
parameters `lossDetectionMode` and `prrEnabled` of the source form come with S8 and S9. With a
bound one segment lower, the guard fires at t=0.2017 s.

Items for later stages: the ACK that ends a recovery also grows cwnd on the source branch (commit
13; with DCTCP's shared ACK path in S6); NewReno's full ACK takes option (2) with PRR (S9, D-16);
the source resets "recover" and `firstPartialACK` at a timeout in the base class, S5e in
`Rfc6582Recovery::onRexmitTimeout()`.

Before the review, S5e moved onto master `e04a0113b3`, which adds three commits about the radio
medium and a message class, and before the landing onto `c099761d74`, a fix of the radio medium;
the CI job passed again at each code commit on both bases.

Evidence, debug build against `omnetpp-6.x`: each commit builds alone with no undefined `inet::`
symbol, and the TCP module tests pass at each commit (on the first base 356 at the first commit
and 359 at the head; on `e04a0113b3` 361 at the head);
the TCP standards tests pass at each code commit (25 pass, 2 expected failures, the packetdrill
tests skip without inet-gpl); unit 114 and serializer 4 pass at the head. The CI fingerprint job
passes at each code commit, after the commit's own new values.

### S7 — `topic/tcp-segment-sizing`: the steps

S7 comes before S6. `TcpCubic` counts with the effective MSS, and in slow start it grows only
while cwnd is below twice the peak segments in flight, which source commit 16 records. Without
S7, CUBIC would not grow in slow start at all (the trap of D-6).

| Step | Commit | B of D-6 | Source |
| --- | --- | --- | --- |
| 1 | add: record the peak segments in flight in each window of data | — | 16 (part) |
| 2 | change: slow start grows by the acknowledged bytes (RFC 5681 equation (2)) | B9 | 13 |
| 3 | change: slow start grows only while cwnd is used | B9 | 13, 16 (part) |
| 4 | change: congestion control counts with the effective MSS | — | 15 (part) |
| 5 | refactor: retransmissions are cut to the effective MSS | — | 15 (part) |
| 6 | change: the MSS option announces this side's own receive limit | — | 15 (part) |

The `sendSegment()` of master already cuts each segment so that the data and the options fit in
`snd_mss`, so step 4 changes no segment. Decision D-19 lists where the other parts of commits 15
and 16 go. The order can change when a step shows that it depends on a later one.

### S7 — `topic/tcp-segment-sizing` — landed 2026-10-08

The owner reviewed and approved S7 on 2026-10-08.

S7 brings the parts of source commits 15 and 16 that size segments and slow start, before S6,
because `TcpCubic` needs them. Decision D-19 gives where the other parts of the two commits go.

| Commit | B of D-6 | Source | Moved CI rows |
| --- | --- | --- | --- |
| `tcp: add: record the peak segments in flight in each window of data` | — | 16 (part) | — |
| `tcp: change: grow cwnd in slow start by the acknowledged bytes` | B9 | 13 | 4 |
| `tcp: change: grow cwnd in slow start only while the sender uses it` | B9 | 13, 16 (part) | 3 |
| `tcp: change: count with the effective MSS in congestion control` | — | 15 (part) | — |
| `tcp: refactor: cut retransmissions to the effective MSS` | — | 15 (part) | — |
| `tcp: change: announce this side's own receive limit in the MSS option` | — | 15 (part) | — |

The commit messages explain each moved row and trace. Slow start now grows cwnd by the bytes that
an ACK acknowledges (RFC 5681 equation (2)), and only while the sender fills cwnd, with the peak
of each window of data (D-20). Congestion control counts with the MSS less the timestamp option,
and the MSS option announces this side's own receive limit. The steps moved out of S7 after a
check showed a dependency or no test (D-19): the default MSS of `mss = -1` (S19), `TCP_MAXSEG`
(S16), the silly-window hold (S16), and the parameters `initialWindow` and
`initialSendSequenceNumber` (S19).

New tests, each checked to fail on the commit before: `tcp_cwnd_limited_1` and
`tcp_cwnd_limited_2` (the second also fails with the peak of the whole connection),
`tcp_effective_mss_1` and `tcp_mss_option_1`. `tcp_nagle_2` and `tcp_timestamp_2` have new traces.

Evidence, debug build against `omnetpp-6.x`: each commit builds alone with no undefined `inet::`
symbol, and the TCP module tests pass at each commit (362 at the first commit, 366 at the head);
the TCP standards tests pass at each code commit (25 pass, 2 expected failures, the packetdrill
tests skip without inet-gpl); unit 114 and serializer 4 pass at the head. The CI fingerprint job
passes at each code commit, after the commit's own new values.

### S6 — `topic/tcp-cubic`: the steps

| Step | Commit | B of D-6 | Source |
| --- | --- | --- | --- |
| 1 | add: `TcpCubic` (RFC 9438) with HyStart, in its final form | — | 14, 45, 55 (part), 57, 60 |
| 2 | refactor: DCTCP on the shared ACK path of the classic flavours | B16 | 62, 65 |

Commits 45, 55, 57 and 60 repair `TcpCubic` before any of it landed, so by rule D-1 step 1 brings
the final form. It has two temporary forms: it does not call `processTlpAck()` until S10 brings
the Tail Loss Probe, and it calls `processEce()` without the acknowledged bytes until step 2.
Commit 62 only adds a comment that commit 65 removes. Commit 65 calls itself a refactor; the CI job
checks that claim on this tree. The source change of the ACK that ends a recovery does not land
(D-21).

### S6 — `topic/tcp-cubic` — landed 2026-10-08

The owner reviewed and approved S6 on 2026-10-08.

S6 adds `TcpCubic` (RFC 9438) with HyStart, in the final form of the source branch, and moves
DCTCP onto the shared ACK path of the classic flavours. Step 1 of the steps above became two
commits, because HyStart works alone and one commit with both had more than 400 lines. Step 2
became four commits (D-22): the CI job showed that the move of source commit 65 is no refactor on
this tree.

| Commit | B of D-6 | Source | Moved CI rows |
| --- | --- | --- | --- |
| `tcp: add: TcpCubic (RFC 9438)` | — | 14, 45, 55 (part), 57, 60 | — |
| `tcp: add: HyStart for TcpCubic` | — | 14, 45, 57 | — |
| `tcp: change: grow DCTCP's cwnd with the shared congestion control` | B9 | 65 | 1 |
| `tcp: change: end DCTCP's inferred SACKs at an ACK of new data` | B8 | 65 | — |
| `tcp: change: let DCTCP with SACK recover as Reno does` | B16 | 65 | — |
| `tcp: refactor: run DCTCP on the shared ACK path` | B16 | 62, 65 | — |

The commit messages explain the moved row and each trace. In short: DCTCP's own copy of the ACK
path grew cwnd by one SMSS at each ACK, kept the inferred SACKs of old duplicate ACKs and so sent
beyond cwnd, and with SACK grew cwnd in the recovery. After the three changes, DCTCP without ECN
gives the same traces as Reno, and the copy goes. Its ECN-Echo reaction is now the hook
`processEce()` of the shared path, which gets the acknowledged bytes. The ACK that ends a recovery
does not grow cwnd (D-21). DCTCP counts no bytes in a fast recovery until S13 (D-23).

New tests, each checked to fail on the commit before: `tcp_algorithm_cubic`, `tcp_cubic_beta_1`,
`tcp_cubic_hystart_1`, `tcp_dctcp_cwnd_limited_1`, `tcp_dctcp_inferred_sack_1` and
`tcp_dctcp_sack_recovery_1`.

Items for later stages: `TcpCubic` calls `processTlpAck()` from S10 on. After S6, nothing calls
`Rfc6675Recovery::sendDataDuringLossRecoveryPhase()` and `sendSegmentDuringLossRecoveryPhase()`;
the source branch keeps them too, and S17, which removes the connection's copy (D-5), can remove
them.

Evidence, debug build against `omnetpp-6.x`: each commit builds alone with no undefined `inet::`
symbol, and the TCP module tests pass at each commit (373 at the first code commit, 377 at the
head); the TCP standards tests pass at each code commit (25 pass, 2 expected failures, the
packetdrill tests skip without inet-gpl); unit 116 and serializer 4 pass at the head. The CI
fingerprint job passes at each code commit, after the commit's own new values.

### S8 — `topic/tcp-rack`: the steps

| Step | Commit | B of D-6 | Source |
| --- | --- | --- | --- |
| 1 | fix: keep the lost mark when a retransmission splits a queue region | — | 49, 17 (part) |
| 2 | add: the send times, the transmission counts and the original segment starts in the retransmission queue, and the minimum RTT | B17 | 17 (part), 13 (part) |
| 3 | change: a retransmission stays inside the sent data and inside one original segment | — | 17 (part) |
| 4 | change: count the bytes in flight with SACK as Linux does, with the DupThresh loss marks | B8 | 16 (part), 17 (part) |
| 5 | add: RACK loss detection (RFC 8985), selected by `lossDetectionMode` | — | 17, 64 (part) |
| 6 | add: the algorithm hears which segments an ACK acknowledges, before the queue discards them | — | 18 (part) |
| 7 | add: learn the reordering degree of the path | — | 19 |
| 8 | add: the STATUS counters | B17 | 18 (part), 16 (part) |
| 9 | add: seed the RTT estimator from the handshake | — | 16 (part) |

Master has the defect of step 1 since S5e, which brought the loss marks, so by rule 4 the fix
stays its own commit. Steps 5, 7 and 9 add features that stay off (D-24). The tests come from
commit 41 (`tcp_rack_1`, `tcp_info_fields_1` to `_3`, and the STATUS query of `TcpTestClient`),
without the parameters of later stages (`initialWindow`, `tcp-legacy.ini`) and without the
STATUS fields of later features (AccECN, Fast Open, the send buffer). The order can change when a
step shows that it depends on a later one.

### S8 — `topic/tcp-rack` — landed 2026-10-08

The owner reviewed and approved S8 on 2026-10-08.

S8 brings RACK loss detection (RFC 8985), the learning of the reordering degree, the STATUS counters
and the handshake RTT seed, which all stay off (D-24), and the count of the bytes in flight with
SACK (B8). The steps above changed in three places. The connection still read the SACK option in
its own copy (D-5), and RACK and the loss marks run in the recovery's copy, so a refactor routes
the option to `Rfc6675Recovery` first (from source commits 26 and 51). Step 6 joined step 7, its
only user. Step 8 became two commits, because master's STATUS reply had no `SocketInd` tag, so an
application that asked for STATUS stopped on an assertion.

| Commit | B of D-6 | Source | Moved CI rows |
| --- | --- | --- | --- |
| `tcp: fix: keep the lost mark when a retransmission splits a queue region` | — | 49, 17 (part) | — |
| `tcp: add: send times and segment starts in the retransmission queue` | B17 | 17 (part), 13 (part) | — |
| `tcp: change: retransmit inside the sent data and the original segments` | — | 17 (part) | 2 |
| `tcp: refactor: let the SACK recovery read the SACK option` | — | 26 (part), 51 (part) | — |
| `tcp: change: count the bytes in flight with SACK as Linux does` | B8 | 16 (part), 17 (part) | 1 |
| `tcp: add: RACK loss detection (RFC 8985), selected by lossDetectionMode` | — | 17, 64 (part) | — |
| `tcp: add: learn the reordering degree of the path` | — | 19, 18 (part) | — |
| `tcp: fix: tag the STATUS reply with the socket ID` | — | 18 (part) | — |
| `tcp: add: the STATUS counters of TCP_INFO` | B17 | 18 (part), 16 (part), 19 (part) | — |
| `tcp: add: seed the RTT estimator from the handshake` | — | 16 (part), 26 (part) | — |

The commit messages explain each moved row and trace. In short: a retransmission keeps the
original segments, as Linux does (the two `tcp_pmtud` rows, after PMTUD cut a segment); with
SACK, the SACKed and lost bytes leave the count of the bytes in flight (`bulktransfer`
`inet_inet_2b`). RACK, the learning and the seed change nothing until a test selects them.
Decisions D-25 to D-27 give the differences from the source tree: a retransmission that ends at
`snd_max` keeps its last whole segment, RACK does not enter a recovery by the duplicate-ACK
count, and the parameter of the handshake seed controls both sides.

New tests, each checked to fail on the commit before: the unit tests `TcpSackRexmitQueue_2` and
`TcpSackRexmitQueue_1`, and the module tests `tcp_rexmit_boundaries_1`, `tcp_sack_inflight_1`,
`tcp_rack_1` to `_3`, `tcp_reordering_1`, `tcp_status_1`, `tcp_info_fields_1` to `_3`, and
`tcp_seed_rtt_1` and `_2`. `tcp_rexmit_boundaries_2` fails with the source form of D-25. The
source tests come without `initialWindow` and `tcp-legacy.ini` (S19); `tcp_rack_1` and
`tcp_info_fields_1` use `increasedIWEnabled` for the larger initial window, and
`tcp_info_fields_2` asks for STATUS after this tree's probes, at t=6.5 and t=9.

Items for later stages: S11 resets the full-segment counter at the SYN (commit 18) and answers
STATUS before the open; S12 keeps the check of `seedRttFromHandshake` on the active side (D-27)
and fills `synDataAccepted`; S13 fills the AccECN fields; S14 fills `skRcvbuf` and
`sndbufLimited`; S15 fills `lastDataRecvTime`, and its persist timing re-times
`tcp_info_fields_2`; S17 moves the reset of the duplicate-ACK counter from the connection to the
algorithm (commit 18, B1 of D-6); S19 sets the source defaults of D-24 and brings the pins of
`Rfc5681FastRetransmit`. In DupThresh mode, the learned reordering degree reaches only IsLost();
the DupAcks trigger keeps DupThresh, as on the source branch, while Linux compares the count
with the learned degree.

Evidence, debug build against `omnetpp-6.x`: each commit builds alone with no undefined `inet::`
symbol, and the TCP module tests pass at each commit (377 at the first code commit, 390 at the
head); the TCP standards tests pass at each code commit (25 pass, 2 expected failures, the
packetdrill tests skip without inet-gpl); unit 118 and serializer 4 pass at the head. The CI
fingerprint job passes at each code commit, after the commit's own new values.

### S9 — `topic/tcp-prr`: the steps

| Step | Commit | B of D-6 | Source |
| --- | --- | --- | --- |
| 1 | fix: count the delivered bytes of a cumulative ACK once | — | 39 (part) |
| 2 | add: Proportional Rate Reduction (RFC 6937), selected by `prrEnabled` | — | 20, 59 (part) |
| 3 | comment: drop a test name from a comment of the SACK recovery | — | 64 (part) |

Step 1 repairs the STATUS field `deliveredBytes` of S8, which PRR reads as DeliveredData (D-28).
PRR lands off, as the features of S8 do (D-24): `prrEnabled = false` until S19. Source commit 59
only removes `prrInitCwndReduction()`, which has no caller, so S9 never adds it; its comment in
`Rfc6582Recovery` refers to a line that S5e already removed. NewReno without SACK keeps option (1)
of RFC 6582 (D-16). Step 3 is the last part of commit 64 for code on master: a comment of
`Rfc6675Recovery::stepC()` from S5d.

### S9 — `topic/tcp-prr` — landed 2026-10-08

The owner reviewed and approved S9 on 2026-10-08.

S9 brings Proportional Rate Reduction (RFC 6937) to the SACK recovery, off by default (D-24), after
a repair of the count of the delivered bytes, which PRR reads as DeliveredData (D-28).

| Commit | B of D-6 | Source | Moved CI rows |
| --- | --- | --- | --- |
| `tcp: fix: count the delivered bytes of a cumulative ACK once` | — | 39 (part) | — |
| `tcp: add: Proportional Rate Reduction (RFC 6937), selected by prrEnabled` | — | 20, 59 (part), 13 (part) | — |
| `tcp: comment: drop a test name from a comment of the SACK recovery` | — | 64 (part) | — |

The commit messages explain each trace. In short: a cumulative ACK now adds its bytes to the
delivered count, but not the bytes that a SACK reported before (the source form counts them twice,
79872 bytes for a transfer of 65536); PRR then paces the sending over the recovery. PRR needs the
sent bytes of the recovery, so `TcpClassicAlgorithmBase` gives `dataSent()` and
`segmentRetransmitted()` to the recovery too, as source commit 13 does. The source runs PRR only
with SACK, so NewReno without SACK keeps option (1) of RFC 6582 (D-16).

In the module test network, all ACKs of a flight arrive at the same time, so PRR and RFC 6675 send
the same segments: PRR spreads the sending inside one instant. `tcp_prr_1` therefore spaces the
duplicate ACKs 1 ms apart with the tester and delays the fast retransmission, and it checks the
first segments of the recovery. Later in that run, the ACKs of the new segments overtake the
delayed duplicate ACKs and SACK the whole flight at once, so both runs send a burst; the test does
not check that part.

New tests, each checked to fail on the commit before: `tcp_delivered_1` and `tcp_prr_1`. The
source's `tcp_prr_1` uses `initialWindow` (S19); here `increasedIWEnabled` gives the larger
initial window.

Items for later stages: S10 adds to `step4()` and `segmentRetransmitted()` the parts of the tail
loss probe and of the undo; S19 sets `prrEnabled` to true and brings the pins of
`Rfc5681FastRetransmit`.

Evidence, debug build against `omnetpp-6.x`: each commit builds alone with no undefined `inet::`
symbol, and the TCP module tests pass at each commit (391 at the first code commit, 392 at the
head); the TCP standards tests pass at each code commit (25 pass, 2 expected failures, the
packetdrill tests skip without inet-gpl); unit 118 and serializer 4 pass at the head. The CI
fingerprint job passes at each code commit, and no row moves.

### S10 — `topic/tcp-undo-frto-tlp`: the steps

| Step | Commit | B of D-6 | Source |
| --- | --- | --- | --- |
| 1 | fix: report every duplicate segment by D-SACK | — | 22 (part) |
| 2 | change: report the duplicated part of a segment by D-SACK | — | 22 (part) |
| 2b | fix: no SACK block for a segment that advanced the ACK | — | new |
| 2c | fix: send nothing when no data is left after the forward of `snd_nxt` | — | 37 (D-33) |
| 3 | add: undo a needless window reduction, selected by `lossUndoEnabled` | — | 22 (part) |
| 4 | add: F-RTO (RFC 5682), selected by `frtoEnabled` | — | 24, 86 |
| 5 | add: Tail Loss Probe (RFC 8985 section 7), selected by `tlpEnabled` | — | 25, 76, 85 |

Master sends a D-SACK for a duplicate segment only when it ends below `rcv_nxt`, so the most common
duplicate, a needless retransmission of the last received segment, gets none; by rule 4 the repair
is its own commit. The new features stay off (D-24) until S19: `lossUndoEnabled`, `frtoEnabled`
and `tlpEnabled` are false; `dsackEnabled` defaults to `sackSupport`, which keeps master's reports.
Commits 76, 85 and 86 repair the probe and F-RTO before they landed, so by rule D-1 they fold into
their steps; the plan's table had put 86 in S17. The D-SACK of a duplicate SYN-ACK in a
simultaneous open goes to S11.

Two repairs of master came up during the work. Step 2b: the receiver put the block of a segment
that filled a gap first in the SACK option, below the new cumulative ACK (RFC 2018 section 4), and
the sender read it as a D-SACK. Step 2c: after a timeout with SACK, the sender made a segment of
zero bytes and stopped (D-33); the F-RTO tests meet it. The work also found D-34, which a later
stage repairs. Step 5 processes each ACK in `TcpAlgorithmBase`, so `TcpCubic` needs no call of
its own (D-35).

### S10 — `topic/tcp-undo-frto-tlp` — landed 2026-10-08

The owner reviewed and approved S10 on 2026-10-08.

S10 brings the undo of a needless window reduction, F-RTO (RFC 5682) and the Tail Loss Probe
(RFC 8985 section 7), all off by default (D-24). Three repairs of master come first, because the
features need them: the D-SACK reports of the receiver, the order of the SACK blocks, and the send
path after a timeout.

| Commit | B of D-6 | Source | Moved CI rows |
| --- | --- | --- | --- |
| `tcp: fix: report every duplicate segment by D-SACK` | — | 22 (part) | 1 |
| `tcp: change: report the duplicated part of a segment by D-SACK` | — | 22 (part) | — |
| `tcp: fix: no SACK block for a segment that advanced the ACK` | — | new | 1 |
| `tcp: fix: send nothing when no data is left after the forward of snd_nxt` | — | 37 | — |
| `tcp: add: undo a needless window reduction, selected by lossUndoEnabled` | — | 22 (part) | — |
| `tcp: add: F-RTO (RFC 5682), selected by frtoEnabled` | — | 24, 86 | — |
| `tcp: add: Tail Loss Probe (RFC 8985 section 7), selected by tlpEnabled` | — | 25, 76, 85 | — |

The commit messages explain each trace. In short: the one moved row, `bulktransfer inet_inet_2b`,
moves two times. First the ACK of a needless retransmission gets a D-SACK block and is 12 bytes
longer; then an ACK that fills a gap loses its block below the cumulative ACK and is 8 bytes
shorter. The undo reads these reports: master sent no D-SACK for the most common duplicate, and a
false one at each filled gap.

S10 deviates from the source where the source is wrong, and a test fails with each source form.
The undo counts as Linux counts (D-29) and starts a new context at each timeout (D-31). F-RTO
takes no old SACK as evidence (D-30) and follows steps 1 and 2.b of RFC 5682 (D-32). The tail loss
probe follows sections 7.2 and 7.4.2 of RFC 8985 (D-35). Source commit 37 comes forward from S16,
with stops for the send loops (D-33). D-34 is a defect of master that `tcp_frto_4` shows; a later
stage repairs it.

In the module test network, all ACKs of a flight arrive at the same time. So the tests delay
single segments or ACKs with the tester, and an original arrives after a retransmission or a
probe.

New tests: `tcp_dsack_1` to `tcp_dsack_3`, `tcp_sack_8`, `tcp_undo_1` to `tcp_undo_3`,
`tcp_frto_1` to `tcp_frto_5`, and `tcp_tlp_1` to `tcp_tlp_6`. The repair of step 2b changes the
expectations of `tcp_sack_3`, `tcp_sack_inflight_1`, `tcp_newreno_sack_1`, `tcp_dctcp_sack_1` and
`tcp_dctcp_sack_recovery_1`: each of their ACKs that fills a gap loses the block below the ACK.

Items for later stages: D-34 (the proposal is S15); the new data of step 2.b of F-RTO; S16 no
longer has source commit 37; S19 sets `lossUndoEnabled`, `frtoEnabled` and `tlpEnabled` to true.

Evidence, debug build against `omnetpp-6.x`: each commit builds alone with no undefined `inet::`
symbol, and the TCP module tests pass at each commit (393 at the first code commit, 410 at the
head); the TCP standards tests pass at each code commit (25 pass, 2 expected failures, the
packetdrill tests skip without inet-gpl); unit 118 and serializer 4 pass at the head. Each repair
fails one commit earlier: `tcp_dsack_1` before step 1, `tcp_dsack_2` and `tcp_dsack_3` before
step 2, the new expectation of `tcp_sack_3` before step 2b, `tcp_sack_8` before step 2c. The CI
fingerprint job passes at each code commit; only `bulktransfer inet_inet_2b` moves, at steps 1 and
2b. The six gates pass.

### S11 — `topic/tcp-connection-lifecycle`: the steps

| Step | Commit | B of D-6 | Source |
| --- | --- | --- | --- |
| 1 | fix: clamp an explicit read at a received FIN | — | 30 |
| 2 | change: answer STATUS on a connection that never opened | — | 38 |
| 3 | fix: send the handshake options in the crossing SYN-ACK of a simultaneous open | — | 16 (part) |
| 4 | change: re-send the SYN-ACK for a retransmitted SYN | — | 23 (part) |
| 5 | change: report the duplicate SYN of a simultaneous open by D-SACK | — | 22 (part), 23 (part) |
| 6 | fix: close a forked connection on an RST in SYN_RCVD | — | 16 (part), 26 (part), 39 (part), 73 |
| 7 | change: reset the connection on new data after a full close | — | 23 (part), 39 (part) |
| 8 | change: reset a SYN-ACK whose TSecr the connection could not have sent | — | 16 (part), 74 |
| 9 | add: quantize the peer MSS as Linux SYN cookies do, selected by `syncookiesAlways` | — | 15 (part) |
| 10 | comment: cite RFC 9293, RFC 7323 and RFC 6675 in the receive path and the state | — | 23 (part) |

S11 gathers the parts of seven source commits that change the life of a connection: its setup,
its close and the commands around them. D-19 sends here the `forked` flag, the options of the
crossing SYN-ACK, the TSecr check of the SYN-ACK and `syncookiesAlways`; S10 sends the D-SACK of
the duplicate SYN of a simultaneous open. Step 6 takes from the Fast Open commit (26) only the
rule for a forked connection, without `fastopenAccelerated` (S12), and commit 73, which repairs
that rule before it landed (D-1). Step 7 takes from commit 39 the line that sets `rcvShutdown` at
a full close. Commit 74 repairs the TSecr check of step 8 before it landed, so it moves from S17
to S11 (D-1).

Commit 23 also holds parts of other stages: the alignment of the options with NOPs only when
configured and the padding of the option area (`alignOptions`, S12), the SACK option from the
recovery's copy of `addSacks()` (S17, D-5), and the end of the reset of the duplicate-ACK counter
in the data path (B1 of D-6, S17). Its AccECN comments go to S13. The reset of the full-sized
segment counter at the SYN (commit 18) and at the SYN-ACK (commit 39) has no effect: the counter
is 0 when the SYN goes out, and `ackSent()` resets it at the SYN-ACK. S11 leaves both lines out.
