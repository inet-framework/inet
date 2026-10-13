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
| S11 ✅ | `topic/tcp-connection-lifecycle` | 15 (part), 16 (part), 22 (part), 23 (part), 26 (part), 30, 38, 39 (part), 73, 74 | +198 −36 | reset after a full close, SYN-ACK re-send, FIN read clamp, STATUS before open, the simultaneous open, a forked connection, the SYN-ACK TSecr check, `syncookiesAlways` |
| S12 ✅ | `topic/tcp-fast-open` | 15 (part), 16 (part), 23 (part), 26 (part), 27 (part), 39 (part), 43, 44, new | +868 −72 | TCP Fast Open (RFC 7413), and the option writing and the MSS rules that it needs (D-42 to D-50) |
| S13 ✅ | `topic/tcp-accecn` | 13 (part), 23 (part), 27, 48 (part), 64 (part), 65 (part), new | +877 −122 | Accurate ECN (RFC 9768), the ECN repairs that it needs, `ecnMarkAll`; DCTCP counts its bytes also in a fast recovery (D-51 to D-60) |
| S14 ✅ | `topic/tcp-receive-buffer` | 28, 29, 35, new | +353 −9 | the receive buffer apart from the advertised window, the Linux receive memory behind parameters, the window and the buffer of one socket (D-4), MSG_ZEROCOPY, the send buffer chrono, SO_TIMESTAMPING, `TCP_NOTSENT_LOWAT` (D-61 to D-64) |
| S15 ✅ | `topic/tcp-timers` | 18 (part), 26 (part), 28 (part), 31, 32 (part), 33, 68 (part), 77, new | +483 −64 | the timer parameters, the RTO of Linux, the SYN retries, a FIN that keeps a running retransmission timer, adaptive delayed ACK, keepalive, no new fast recovery before the recovery point of a timeout (D-34) |
| S15b ✅ | `topic/tcp-zero-window` | 13 (part), 32 (part), 34 (part), 84, new | +186 −56 | the zero window: the persist timing (B15), the probe, the window update that reopens the window (B14) |
| S16 ✅ | `topic/tcp-send-rules` | 15 (part), 16 (part), 36 (part), 69, new | +90 −4 | the sender SWS avoidance of RFC 9293 with its override timeout, `TCP_NODELAY`, the Nagle of Minshall, `TCP_MAXSEG` (D-73 to D-75) |
| S16b ✅ | `topic/tcp-write-boundaries` | 36 (part), 46 (part), new | +297 −8 | `TCP_CORK`, `MSG_MORE`, `MSG_EOR`, the PSH of Linux at write boundaries behind a parameter (D-76 to D-78) |
| S17 ✅ | `topic/tcp-connection-leftovers` | 39 (part), 61 | +82 −754 | the copies of the SACK recovery that nothing calls, the SACK scoreboard scan (D-79, D-80) |
| S17b ✅ | `topic/tcp-timestamps` | 26 (part), 39 (part), new | +113 −37 | the timestamps of RFC 7323: `TS.Recent` from the handshake, PAWS, the update after the window test, the TS option on each segment; the update rule of Linux behind a parameter (D-81 to D-83) |
| S17c ✅ | `topic/tcp-segment-checks` | 39 (part), new | +72 −12 | the sequence number of a pure ACK and of the RST of an abort, the window update from data (D-72), the FIN in the window test, the Linux checks of RFC 5961 and of the window edge and the RST with ACK of Linux behind parameters (D-84 to D-86) |
| S17d | `topic/tcp-connection-setup` | 26 (part), 39 (part), new | +150 −30 (about) | the text of a SYN, the data queued in SYN_RCVD, the timers at a return to LISTEN, the ICMP checks of RFC 5927 and in SYN_RCVD |
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
  S14 too; nothing in INET needs it before then. Commit 3 does not land. S14 adds the open
  command and the `@mutable` `timestampSupport` (D-63).
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
  part of S10. The proposal is S15, which changes the loss marking at a timeout. S15 repairs it
  for an RTO during a loss recovery, the scope of section 5.1, at a duplicate ACK and at the RACK
  timer; Linux blocks a new recovery after each RTO (its Loss state), which S15 does not model.
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
- **D-36 — An explicit read stops at a received FIN in both read paths.** A FIN occupies a
  sequence number but puts no byte in the receive queue. Source commit 30 stops the delivery of
  waiting data at the FIN, but it reads `rcv_fin_seq`, which only a FIN that came out of order
  set; and a READ command after the FIN took the other path, which had no stop. So the stop never
  worked for the usual close. S11 records the FIN position also for an in-order FIN and stops
  both paths there; `tcp_read_fin_1` fails with the source form.
- **D-37 — A forked connection closes also on the timeout of the connection setup.** The source
  (commit 26) closes a forked connection in SYN_RCVD on an RST, but the timeout still sent it
  back to LISTEN, which gives the same second listener. S11 closes it on both; `tcp_syn_12` fails
  with the source form.
- **D-38 — The reset after a full close gives the application CLOSED.** The source ends the
  connection with `TCP_E_ABORT`, which sends no indication. The application asked for the close,
  and its socket waits in LOCALLY_CLOSED for CLOSED, for example to start its next session or to
  end the stop of its node. S11 sends CLOSED.
- **D-39 — The TSecr check of a SYN-ACK comes before the SYN-ACK changes any state.** The source
  checks after the SYN-ACK has set `rcv_nxt`, `irs` and `snd_una`, discarded the queues and queued
  its data, so a rejected SYN-ACK left the connection in SYN_SENT with that changed state. Linux
  checks first. S11 reads the TSecr from the option and checks before the SYN processing.
- **D-40 — Only a passive open re-sends its SYN-ACK for a repeated SYN.** Linux re-sends it in
  `tcp_check_req()`, which handles a request socket, that is, a passive open; after a
  simultaneous open, the socket in SYN_RECV answers a repeated SYN with a duplicate ACK. The
  source (commit 23) re-sends it in any SYN_RCVD. `tcp_syn_15` fails with the source rule.
- **D-41 — The reset on data after a close is a parameter, off by default.** The CLOSE of RFC
  9293, which `TcpSocket::close()` sends, means "no more to send", and the application may still
  receive; RFC 1122 section 4.2.2.13 asks for the RST only from a host that implements a
  "half-duplex" close, which is a MAY. Linux `close()` is such a close. The source resets after
  every CLOSE without `halfClose`, so INET's applications that read after their close lost their
  data: the clients of `ethernet/arptest` and `arptest2` reset the echo of their server, and two
  CI rows moved. S11 adds `abortOnDataAfterClose` (default false), which the packetdrill
  configuration of inet-gpl can select, as the owner's rule for the Linux behaviors says.
- **D-42 — The default send MSS applies to a SYN without an MSS option.** RFC 9293 MUST-15: "If
  an MSS Option is not received at connection setup, TCP implementations MUST assume a default
  send MSS of 536 (576 - 40) for IPv4 or 1220 (1280 - 60) for IPv6". Source commit 15 uses 536
  only for an MSS option of value 0, which the RFC does not name; without the option, the
  connection kept its own MSS. S12 applies the default of the address family when the SYN or the
  SYN-ACK of the peer has no MSS option; `tcp_mss_default_1` and `_2` fail with the source form.
- **D-43 — The order of the SYN options stays.** Source commit 16 writes the timestamp option
  before the window scale option of a SYN (and wrote it two times, until commit 27). No RFC sets
  the order, the packetdrill comparison of inet-gpl ignores it, and with SACK the master order
  keeps TSval on a 4-byte boundary, which the source order does not. S12 keeps the master order;
  the Fast Open option goes last, as in Linux.
- **D-44 — A Fast Open server checks the cookie by default.** RFC 7413 section 4.2.2: with a
  cookie that is not valid, the SYN-ACK acknowledges only the SYN. The source accepted each cookie
  of a valid length by default (`fastopenLenientCookieValidation = true`). S12 makes the check
  the default (the packetdrill configuration of inet-gpl already sets it), and the server also
  ignores a cookie of a length that RFC 7413 section 4.1.1 does not allow (odd, or not 4 to 16
  bytes); the source checked the length only on the client.
- **D-45 — The cookie function is SipHash-2-4 for each cookie.** The source used SipHash only with
  `fastopenKey` and IPv4, and otherwise a mix of `std::hash`, which gives other cookies with
  another C++ library, so a fingerprint with Fast Open differs between platforms. S12 uses the
  SipHash-2-4 MAC of the two addresses (IPv4 or IPv6, as Linux) under a key from the RNG or from
  `fastopenKey`; `tcp_fastopen_key_1` checks the reference vector and the Linux cookie of the
  packetdrill suite.
- **D-46 — The application gets a Fast Open connection at its SYN, and the algorithm starts
  there.** RFC 7413 section 4.2.2 ("Buffer the data and notify the application"; step 6, the
  initial window) and appendix A.2 ("accept() returns upon receiving a SYN with a valid Fast Open
  cookie"); Linux starts the transfer of the child socket (`tcp_init_transfer()`). The source
  delivered the data with the socket ID of a forked connection before TCP_I_AVAILABLE, which the
  application did not know; with `listenOnce()`, the socket stayed LISTENING and could not send;
  and the algorithm started at the ACK of the SYN-ACK, so the congestion window was 0 in
  SYN_RCVD. S12 sends TCP_I_AVAILABLE (forked) or TCP_I_ESTABLISHED at the SYN, starts the
  algorithm there, and sends no second indication at the ACK of the SYN-ACK; an RST in SYN_RCVD
  gives such a connection TCP_I_CONNECTION_RESET (`tcp_fastopen_fork_1`, `_srvdata_1`).
- **D-47 — Smaller deviations in the Fast Open core.** (1) A retransmitted SYN-ACK carries no
  cookie (RFC 7413 section 4.2.2, SHOULD); the source sent it again
  (`tcp_fastopen_synack_rexmit_1`). (2) Only an MSS option of the SYN-ACK updates the cached MSS
  (RFC 7413 section 4.2.2, client step 1); the source cached `snd_mss` without one. (3) Nagle does
  not hold the SYN data that the SYN-ACK did not acknowledge, so the first ACK carries it; the
  source relies on the Minshall form of Nagle (S16), and with the Nagle of master the data waited
  for ever (`tcp_fastopen_rexmit_1`). (4) The NOPs before a Fast Open option follow
  `alignOptions`. (5) The ACK of the SYN-ACK runs the ACK processing of the
  algorithm for each Fast Open connection that sent in SYN_RCVD; the source asked for SYN data.
  By D-1, the data on the SYN (commit 39; commit 26 recorded the data but sent the SYN without
  it) and the SYN-ACK that keeps `snd_max` over the data (commit 27) fold into the core.
- **D-48 — Fast Open blackhole detection is on by default, and follows Linux.** RFC 7413 section
  4.1.3.1: "The client MUST cache negative responses". The source default was 0 s (off, as Linux
  since 5.14). S12 uses 3600 s, the former Linux default; a SYN with only a cookie request counts
  too, as in Linux; an RST with a wrong acknowledgment number is no trigger, because neither the
  RFC nor Linux has one; a stopped connection sends no Fast Open option, as Linux (the source
  sent an empty request); and a SYN-ACK that acknowledges the SYN data resets the count.
- **D-49 — `fastopenExpOptionEnabled` selects both directions of the experimental option.** The
  client of the source fell back to kind 254 without the parameter, but ignored a kind 254 answer
  without it. S12 makes the parameter select the fallback too (default off: RFC 7413 section 9
  says implementations SHOULD use kind 34).
- **D-50 — A limit of pending Fast Open connections.** RFC 7413 section 4.2: over a preset limit
  of PendingFastOpenRequests, "the server MUST disable TFO for all new connection requests". The
  source has no limit. S12 adds `fastopenMaxPendingRequests` for each local port; its default of
  16 is an assumption, because Linux takes the limit from the TCP_FASTOPEN socket option, which
  INET's socket API does not have.
- **D-51 — ECT(0), and ECT only on new data unless `ecnMarkAll`.** RFC 8311 section 3: "A
  network node MUST NOT originate traffic marked with ECT(1) unless ... participating in a
  Congestion Marking Differences experiment"; master used ECT(1). RFC 3168 sections 6.1.1 to
  6.1.6 forbid ECT on a SYN, a SYN-ACK, a pure ACK, a retransmission and a window probe, and RFC
  9768 section 4 keeps that; master marked a SYN-ACK, a FIN without data and a window probe. S13
  follows the RFCs by default. `ecnMarkAll` (default false) sets ECT on the control segments of an
  AccECN or DCTCP connection, as Linux does (`tcp_ecn_send()`), and RFC 8257 section 4.1
  recommends such a knob for DCTCP. DCTCP sets ECT on its SYN, SYN-ACK and RST by default (RFC 8257
  section 3.6: SHOULD; Linux), through `needsEcn()` of the flavour (Linux TCP_CONG_NEEDS_ECN). The
  source set ECT on each segment of AccECN without a knob, and on each DCTCP segment, also on the
  SYN of each mode.
- **D-52 — A retransmission timeout with nothing in flight sends new data.** An ECN-Echo at a cwnd
  of one segment restarts the retransmission timer (RFC 3168 section 6.1.2). With nothing in
  flight, master then retransmitted nothing and failed an assertion. S13 sends new data at such a
  timeout. RFC 3168 section 6.1.2 also says that the sender "will then be able to send a new packet
  only when the retransmit timer expires"; master, the source and Linux send at once, and S13
  keeps that.
- **D-53 — `tcpEcnMode` replaces `ecnWillingness`.** Linux has five modes (`tcp_ecn`), and a
  passive one is its default. `tcpEcnMode` is "off", "passive", "rfc3168", "accecn" or
  "accecn-passive"; it is @mutable, as a harness can change the sysctl between connections.
  `ecnWillingness` stays, deprecated, and a run that sets both stops with an error.
- **D-54 — Each flavour responds to the AccECN feedback.** RFC 9768 section 3.1.5 obliges a host in
  AccECN mode "to respond appropriately to AccECN feedback", and section 4 maps the response to an
  increase of s.cep, without CWR. In the source only DCTCP read the counts. S13 decodes the ACE
  field before the algorithm processes the ACK, and an increase acts as an ECN-Echo in each
  flavour. CE feedback in the handshake also gets a response (section 3.2.2.2); Linux only counts
  it, and no Linux script sees the difference, so S13 has no parameter for it.
- **D-55 — The AccECN handshake and counters follow RFC 9768 where the source did not.** An unknown
  combination of the SYN flags negotiates AccECN (section 3.1.3; the source: no ECN). A passive
  or classic server answers an AccECN SYN with classic ECN (Table 2, as Linux; the source needed
  AE=0 for a passive server). The handshake encoding of the ACE field goes only on a pure ACK
  without SACK blocks (section 3.2.2.1; the source: also on data). Only acceptable packets count
  (section 4; the source counted at the input of the module). `accEcnBareTimeWaitAck` selects the
  Linux ACK of TIME_WAIT without the ACE field (section 3.2.2.2 asks for it; the source always
  left it out).
- **D-56 — The AccECN option of the receiver has two policies.** `accEcnOptionEnabled` (default
  true; RFC 9768 section 3.2.3: RECOMMENDED). `accEcnOptionPolicy="rfc"` (default) follows the
  recommended scheme of section 3.2.3.3: the option after a byte count changed, with the fields
  that ever changed, in the order that needs fewer fields, and with each field on the first
  SYN-ACK and the first segments of the client (section 3.2.3.2.1). "linux" follows
  `tcp_ecn_option=2`: each segment, kind 174, each field that fits in the Linux layout of the
  options, only after the peer sent an option (section 3.2.3.2.3 asks to send it also then), and
  the counts of a duplicate that ends at RCV.NXT (Linux `tcp_sequence()`). Both policies cut the
  option to the fields that changed and give up SACK blocks down to two (section 3.2.3.3). The
  source sent the option on each fourth ACK and alternated the kinds, which neither the RFC nor
  Linux does. inet-gpl needs "linux".
- **D-57 — The AccECN option of the sender follows Linux.** A stretch ACK is decoded as
  `__tcp_accecn_process()` does, which holds the rule of Appendix A.2.2; the first option is
  checked for zeroed fields (section 3.2.3.2.4, a MAY); the option stops after an ACK with the
  option and a D-SACK block got lost (section 3.2.3.2.2, a MAY). DCTCP counts the CE bytes of the
  option. The source chose the marks of a large ACK by the nearest byte estimate. S13 does not
  detect CE bytes without CE marks (section 3.2.3.2.5 asks for the reaction only after a
  detection, and Linux has none), and it does not estimate the bytes of an ACK without the option
  (Linux `est_ecnfield`).
- **D-58 — The AccECN fallback of a retransmitted SYN and SYN-ACK.** An AccECN SYN sets (1,1,1)
  on its first retransmission and (0,0,0) on the later ones (RFC 9768 section 3.1.4.1). A
  retransmitted AccECN SYN-ACK has no option, from the second retransmission (0,0,0), and after
  that the server sets no ECT (sections 3.2.3.2.2 and 3.1.5). As Linux. Classic ECN keeps its
  flags: RFC 3168 section 6.1.1.1 lets a client clear them on a retransmitted SYN (a MAY), and
  Linux does, but a SYN lost to congestion then costs the connection its ECN; with it, the DCTCP
  incast example ran some connections without ECN, and both of its CI rows moved. The source
  cleared the flags of a classic SYN and SYN-ACK too, and it kept ECT after its own (0,0,0)
  SYN-ACK.
- **D-59 — The ECN state before the first segment after the SYN-ACK.** Master set the classic ECN
  state of the client after `established()`, which already sends the ACK of the SYN-ACK or waiting
  data, so that data went out Not-ECT. S13 sets it before, as the source does since commit 27.
- **D-60 — What S13 does not do.** RFC 9768 section 3.1.5: a server MUST reset a connection whose
  handshake has both a classic ECN-setup SYN and an AccECN SYN. Section 3.2.2.1: the client SHOULD
  send a pure ACK before data on the ACK of the SYN-ACK. Section 3.2.2.5.1: an ACK at once when a
  CE mark follows a packet without one (SHOULD), and an ACK of ACKs after n CE marks on pure ACKs
  (MUST; only ECT pure ACKs can carry them, which needs `ecnMarkAll`). Section 3.1.4.1: a cache of
  failed AccECN attempts (SHOULD). DCTCP requests ECN only by `tcpEcnMode`, while Linux requests it
  for DCTCP by itself.
- **D-61 — The receive buffer keeps the default offer of master.** Commit 28 separates the receive
  buffer (`receiveBufferSize`, Linux `sk_rcvbuf`) from the advertised window. It also changed the
  default offer to the window less the unread in-order data, so that out-of-order data no longer
  made the offer smaller. RFC 9293 section 3.8.6.2.2 counts the data that waits in the buffer, and
  Linux counts the out-of-order queue (`sk_rmem_alloc`), so S14 keeps the offer of master. With
  `receiveBufferSize`, the buffer limits the acceptance and the offer. The source took the larger
  of the buffer and the window as the limit, so a smaller buffer had no effect. The parameter is
  not `@mutable` (the source: `@mutable`): a socket sets its own buffer in its open command (D-63).
- **D-62 — The Linux receive memory is off by default.** Each Linux behavior of commit 28 has a
  parameter, default off, which inet-gpl turns on: `acceptBeyondBufferAtEmptyQueue`
  (`tcp_data_queue()`; the source accepted such a segment always, and master drops it),
  `windowAutoTuning` (`tcp_grow_window()`) and `windowShrinkAllowed` (`tcp_shrink_window=1`). The
  growth of the buffer of an owned connection (`tcp_clamp_window()`) comes with
  `windowAutoTuning`, as in Linux; the source grew the buffer in each mode, so
  `receiveBufferSize` did not limit a plain connection. The automatic window scale covers the
  clamp of the auto-tuning; the source used the advertised window, so the offer stopped at 65535
  bytes. The charge of the buffer at the Linux skb truesize runs in each mode, as in the source;
  only `windowShrinkAllowed` and the growth read it. Later stages bring the rest of commit 28: the
  flag of a segment beyond the offered edge (`overWindowAcceptPending`) comes with its reader, the
  adaptive delayed ACK of commit 31 (S15); the acceptance up to the highest edge ever offered
  (`rcv_mwnd_seq`, commit 47), the upper limit of the growth (`tcp_rmem[2]`) and the lock by
  SO_RCVBUF come with the receive buffer memory (S18).
- **D-63 — The window and the buffer of one socket travel in its open command (D-4).**
  `TcpSocket::setAdvertisedWindow()` and `setReceiveBufferSize()` set them for the next
  `connect()` or `listen()`, as `setTCPAlgorithmClass()` does; -1 takes the module parameters. A
  connection with its own window takes an automatic window scale, because Linux sets the scale
  from the buffer of the socket. A connection that a listening socket forks takes the values of
  that socket. A change of the buffer of an open connection waits for S18. `timestampSupport` is
  `@mutable` for the host-wide sysctl `net.ipv4.tcp_timestamps`, and each connection reads it at
  its open, as `tcpEcnMode` (D-53). The source made the window and the scale `@mutable` too.
- **D-64 — The socket options of commits 29 and 35 follow Linux on a tun device.** A zero-copy
  completion goes to the application when its last byte goes out, not at its ACK: the tun device
  of packetdrill copies the data (`SO_EE_CODE_ZEROCOPY_COPIED`), and INET has no pages to keep.
  The receive timestamp is the time of the delivery to the application, as the contract of S3
  says. The send buffer chrono also stops at the ACK of all data, as Linux stops each chrono when
  its queues are empty (`tcp_chrono_stop()`); the source stopped it only at a send or a command, so the time ran on. The
  mark of `TCP_NOTSENT_LOWAT` comes from the socket option, else from the parameter
  `notsentLowat`, as in the source.
- **D-65 — The timer parameters keep the values of master.** Commit 32 turned the timer constants
  into parameters and changed three defaults to those of Linux: a minimum RTO of 200 ms, a maximum
  of 120 s, and the RTO formula of Linux; commit 68 put the formula of RFC 6298 back, behind
  `rtoVarianceFloor`, and the minimum of 1 s. S15 keeps the values of master (`initialRto` 3 s,
  `minRexmitTimeout` 1 s, `maxRexmitTimeout` 240 s), and S19 changes the defaults: the initial RTO
  of 1 s of RFC 6298 section 2.1 (the source: 1 s) and the maximum (the source: 120 s, which RFC
  6298 section 2.5 allows). `synLinearTimeouts` is 0 by default, as RFC 6298 section 5.5 doubles
  the timer at each expiry (the source: 4, as Linux). `synRetries` has the plain default 12 and is
  not `@mutable` (the source: -1 for 12, and `@mutable` for TCP_SYNCNT). `maxRexmitTimeout` also
  limits the SYN timeout. The rule of RFC 6298 section 5.7 (commit 77) acts only below an initial
  RTO of 3 s.
- **D-66 — TCP_SYNCNT and SO_KEEPALIVE are socket calls (D-4).** inet-gpl changed the parameter
  `synRetries` at run time for TCP_SYNCNT. `TcpSocket::setSynRetries()` puts the value in the open
  command, as `setReceiveBufferSize()` does (D-63). RFC 9293 MUST-24 asks for keepalive on or off
  for each connection, which the source did not have: `TcpSocket::setKeepAlive()` sends a command
  that acts before and after the open, as SO_KEEPALIVE. A listening socket gives both values to
  the connections that it forks, as in Linux.
- **D-67 — Keepalive as RFC 9293 section 3.8.4 and Linux `tcp_keepalive_timer()`.** The defaults are
  those of Linux (7200 s, 75 s, 9 probes; MUST-28: at least two hours). TIME_WAIT sends no probe,
  as Linux keeps only a time-wait socket there; the source could send one. At the end of the
  probes the connection aborts as after too many retransmissions, without a RST, as the source
  does; Linux sends a RST.
- **D-68 — The delayed ACK of Linux is the model of the source.** The source tuned
  `adaptiveDelayedAcks` against the packetdrill scripts of inet-gpl. It simplifies Linux in five
  places: each ACK uses one ACK of the quickack budget, where `tcp_dec_quickack_mode()` counts only
  an ACK of new data and resets the ATO at the end of the budget; `__tcp_ack_snd_check()` also asks
  that the right edge of the window advances; the send MSS stands for the received MSS; a FIN
  without data updates the ATO; a retransmission is not a reply. S15 changes only that a FIN
  without data is not a reply, as in Linux (`tcp_event_data_sent()`); the other places can change
  when a run of the scripts against master shows a reason, after S18.
- **D-69 — STATUS reports the last in-order data.** `lastDataRecvTime` is the arrival of the last
  in-order data, and before any data the end of the handshake, as Linux `icsk_ack.lrcvtime`, which
  `tcpi_last_data_recv` reports. The source reported the last segment of any kind.
- **D-70 — The zero-window probe of RFC 9293 by default, the probe of Linux by parameter.** RFC 9293
  section 3.8.6.1 asks for "at least one octet of new data (if available), or retransmit". The
  probe of S15b is the first unacknowledged octet, the same octet at each probe, as BSD sends it in
  its persist state; it starts no retransmission timer, and its ACK is no duplicate ACK, so the
  connection stays open while the peer answers (MUST-37). `zeroWindowProbe = "linux"` selects the
  probe of Linux without data at SND.UNA − 1, which the source has as its only probe. The source
  also keeps `minPersistTimeout`, which nothing reads; S15b does not add it.
- **D-71 — The persist timer runs only while data waits for a zero window.** Master started it at
  each zero-window ACK, also without waiting data, and then it never stopped; it did not start when
  a zero window stopped a later send. S15b starts it at a zero window only when data waits, and
  after a send that the zero window stopped, and an expiry without data stops it, as Linux
  `tcp_check_probe_timer()` and `tcp_probe_timer()`. The source has the same defects.
- **D-72 — A pure ACK with SEG.ACK = SND.UNA updates the send window.** RFC 9293 section 3.10.7.4:
  "If SND.UNA =< SEG.ACK =< SND.NXT, the send window should be updated". Master updated it only from
  a pure ACK with data in flight; S15b, as the source, also with nothing in flight. A window update
  ends the persist state, also with the probe octet in flight, which goes again as unsent data
  (`TcpSackRexmitQueue::discardFrom()`), and it is no duplicate ACK (RFC 5681 section 2, condition
  (e)). The rule of RFC 9293 also covers a data segment of the peer, but a first version that took
  it moved six CI rows: a pure ACK of INET carries `snd_nxt`, which goes back during a
  retransmission (BSD and Linux put the highest sequence number sent), so after a data segment of
  the peer the window of its later pure ACKs looked old (SND.WL1), condition (e) failed, and a fast
  retransmit did not come. Both repairs go together to a later stage.
- **D-73 — The sender SWS avoidance of RFC 9293, with its override timeout.** Master had none
  (MUST-38). S16 follows section 3.8.6.2.1: a segment of new data smaller than the effective MSS
  goes out only if it carries all queued data (rule (2)) or at least Fs = 1/2 of Max(SND.WND)
  (rule (3)); else it waits, and the persist timer of S15b times the override (rule (4)), as the
  RFC suggests and as Linux sends from its probe timer (`tcp_write_wakeup()`). As BSD, the usable
  window is also limited by the congestion window, and a retransmission is never held. The source
  has rules (1) to (3), with the Fs condition of commit 69, but no override, so a window that
  stayed below both limits blocked the data for ever.
- **D-74 — PSH in the last buffered segment goes to S19.** INET has no PUSH flag on SEND, so RFC
  9293 section 3.9.1.2 asks for PSH "in the last buffered segment (i.e., when there is no more
  queued data to be sent) (MUST-61)"; master and the source set it on no segment by default (the
  source only with the Linux option of S16b). The rule changes the end of each train of data
  segments: the packet dump of 164 module tests and the packet data of each TCP fingerprint row.
  S19 moves the tests to the new defaults and pins the old ones, so the rule lands there, with a
  parameter whose default follows the RFC, and each test changes once.
- **D-75 — The socket options of the send rules.** `TCP_NODELAY` and `TCP_MAXSEG` act through the
  socket calls of the contract (D-4), before and after the open, and a forked connection takes the
  values of its listener. `TCP_NODELAY` sends the data that Nagle holds at once, as Linux
  `tcp_push_pending_frames()`. `TCP_MAXSEG` limits the send MSS, and before the open also the MSS
  option, together with the `mss` parameter, as Linux limits `user_mss` by the MSS of the device;
  the source announced the value of the option also when it was larger. `minshallNagle` selects the
  Nagle of Minshall that Linux uses; the default is the Nagle of RFC 9293 section 3.7.4, where the
  source made the variant of Minshall the only Nagle.
- **D-76 — `TCP_CORK` and `MSG_MORE` as Linux.** Master ignored both; S3 landed only the contract.
  A partial segment waits while `TCP_CORK` is set or while the last write has `MSG_MORE`; a
  full-sized segment and a FIN go. An uncork sends the held data, but Nagle applies again, as Linux
  `__tcp_sock_set_cork()`. With nothing in flight, the cork timer sends the held data with PSH one
  RTO after the start of the hold, and a later hold does not start it again, as Linux
  `tcp_check_probe_timer()` and `tcp_write_wakeup()`: the hold has a ceiling. `TCP_NODELAY` sends
  the held data once and keeps the cork (Linux `TCP_NAGLE_PUSH`). The persist timer does not start
  while the cork timer runs, but a zero window starts it. The source starts the cork timer again at
  each hold, so a train of small writes can hold the data for longer than one RTO, and it sends the
  held data at an uncork also when Nagle holds it.
- **D-77 — `MSG_EOR` sets no PSH, and Nagle holds only the last record.** No segment spans a record
  boundary, also not a retransmission. Linux sets PSH at the end of a write without `MSG_MORE`
  (`tcp_mark_push()`), not at a record boundary, so S16b gives a boundary no PSH of its own; with
  the Linux parameter of D-78, the end of the write sets it. A record that ends inside the partial
  segment at the end cannot become larger, so Nagle and the cork hold only the data after the last
  boundary, as Linux `tcp_write_xmit()`, which tests them only on the last skb of the queue. The
  source sets PSH at each boundary and holds a whole record. The boundaries are in a queue in
  sequence order, where the source has a `std::set`, whose order is wrong after a wrap-around of
  the sequence numbers.
- **D-78 — The PSH of Linux is an option.** No RFC asks for PSH at the end of a write, at the end of
  a GSO burst, in a long write (`forced_push()`) or on the data of a Fast Open SYN, so
  `pushSegmentsOnWriteBoundary` selects these rules and its default is false; inet-gpl sets it. The
  rules and their geometry come from the source, which fitted them to the packetdrill scripts;
  only the Linux headers are on hand, so the scripts of inet-gpl measure them. `pushed_seq` starts
  at the ISS, as Linux `tcp_connect_init()`, where the source set it at the first write, which
  compares wrongly when the ISS is more than 2^31 from zero. The PSH in the last buffered segment
  of RFC 9293 (MUST-61) stays with S19 (D-74).
- **D-79 — The SACK option stays with the connection.** The SACK option reports what the data
  receiver holds (RFC 2018), so `TcpConnection::addSacks()` writes it from the receive queue,
  whatever loss recovery the sender uses; S17 removes the copy in `Rfc6675Recovery`, which had no
  caller. This reverses the move of the option that D-5 and the items of S11 and S13 planned. The
  source writes the option through a `dynamic_cast` to `Rfc6675Recovery` and guards it (commit
  51), so a connection in SYN_RCVD, whose recovery does not exist yet, or with another recovery,
  sends no SACK option; and its copy lacks the first-block rule of RFC 2018 section 4 (S8) and the
  room for the AccECN option (S13). `TcpConnectionSackUtil.cc` stays, with this one function.
- **D-80 — Four parts of commit 39 stay out.** The connection keeps the reset of the duplicate-ACK
  counter after the ACK handlers of the algorithm (B1 of D-6): the source resets it at the end of
  `TcpAlgorithmBase::receivedAckForUnackedData()`, before the code of the flavour, so the
  `dupacks = dupthresh` that `TcpVegas` sets after its fine-grained retransmission stays, which
  changes Vegas. `validationInfo()` has no caller in the source. The statistic `sndNxt` keeps its
  name, which the lwIP TCP shares; the source renames it to `sndSeq`. The range that
  `retransmitOneSegment()` reports differs from the source's only in SYN_RCVD, where the list of
  send times ignores it.
- **D-81 — The handshake sets `TS.Recent` only in LISTEN and SYN_SENT.** RFC 7323 appendix D saves
  the TSval of a SYN in LISTEN, and of a SYN or a SYN-ACK in SYN_SENT; master took `TS.Recent`
  only from a later segment, so its SYN-ACK and its handshake ACK echoed TSecr 0. The source sets
  `TS.Recent` from each segment with SYN, also in a synchronized state, where PAWS protects it
  (section 5.2).
- **D-82 — PAWS and the update of `TS.Recent` follow RFC 7323 section 5.3.** Master and the source
  dropped a segment with an older TSval only after 24 days of idle time, the opposite of R1, so
  PAWS never dropped a segment; S17b drops it while `TS.Recent` is valid, measured from the last
  update of `TS.Recent` (section 5.5, as Linux `ts_recent_stamp`), and exempts a RST, which
  changes no timestamp state (section 5.2). `TS.Recent` changes only after the window test (R2
  before R3), where master and the source follow the order of appendix D. RFC 7323 keeps R1 for
  each non-RST segment; Linux also accepts an older TSval within one tick and on a reordered pure
  ACK (`tcp_disordered_ack()`), which S17b does not model. The update of Linux after the ACK check
  is the parameter `tsRecentAfterAckCheck`, whose default follows the RFC.
- **D-83 — The TS option on each segment after the handshake.** RFC 7323 section 3.2 asks for the
  option in each non-RST segment, and the RST of a user abort SHOULD carry it; master's FIN and its
  segments in CLOSING, LAST_ACK and TIME_WAIT had none, and its receiver refused the option in
  those states and in CLOSE_WAIT. The abort RST carries TSecr 0, because it has no ACK bit. A RST
  that answers a segment stays without the option: section 5.2 asks for TSval 0 there "to enable
  research", which Linux does not do. The source puts the option on each RST with ACK, with the
  TSecr of `TS.Recent`.
- **D-84 — A pure ACK and the RST of an abort carry SND.NXT of RFC 9293.** RFC 9293 sends
  `<SEQ=SND.NXT><ACK=RCV.NXT><CTL=ACK>` and, at an ABORT, `<SEQ=SND.NXT><CTL=RST>`; its SND.NXT is
  the next new sequence number, `snd_max` in INET, because INET's `snd_nxt` goes back to `snd_una`
  during a retransmission. BSD and Linux put the highest sequence number sent on a pure ACK. With
  `snd_nxt`, the peer refused the window of a pure ACK sent during a retransmission (SND.WL1), so
  its duplicate ACKs failed condition (e) of RFC 5681, and it dropped a RST below its RCV.NXT.
  With the repair, a data segment of the peer updates the send window too (RFC 9293 section
  3.10.7.4), which closes D-72. The source has neither repair. The segment log shows the sequence
  number of a segment without data, so that a test can read it.
- **D-85 — The FIN counts in the window test of a segment with data.** RFC 9293 counts the SYN and
  the FIN in SEG.LEN, so a segment of old data with a FIN at RCV.NXT is acceptable, and its FIN is
  processed; only its data beyond RCV.NXT goes to the receive queue. A segment without data keeps
  master's test, as the source does, so a bare FIN at a zero window stays acceptable, which the
  RFC would refuse.
- **D-86 — Three rules of Linux are options.** The ACK check of RFC 5961 section 5.2, which RFC 9293
  allows (MAY-12), the drop of a segment beyond the highest window edge offered (Linux
  `tcp_sequence()`), and the RST with ACK at an abort (Linux `tcp_send_active_reset()`) are not
  required by the RFCs, so `rfc5961AckCheck`, `dropBeyondWindowEdge` and `abortRstWithAck` select
  them, with defaults that keep master's behavior. The source makes the three rules
  unconditional. Like the source, S17c models neither the blind RST and SYN mitigations of RFC
  5961 (sections 3 and 4) nor the rate limit of challenge ACKs.
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
| 3 | fix: read and send the handshake options in a simultaneous open | — | 16 (part), 26 (part) |
| 4 | change: re-send the SYN-ACK for a retransmitted SYN | — | 23 (part) |
| 5 | change: report the duplicate SYN of a simultaneous open by D-SACK | — | 22 (part), 23 (part) |
| 6 | fix: close a forked connection on an RST or a timeout in SYN_RCVD | — | 16 (part), 26 (part), 39 (part), 73 |
| 7 | add: reset on data after a close, selected by `abortOnDataAfterClose` | — | 23 (part), 39 (part) |
| 8 | change: reset a SYN-ACK whose TSecr the connection did not send | — | 16 (part), 74 |
| 9 | add: the SYN cookie MSS of Linux, selected by `syncookiesAlways` | — | 15 (part) |
| 10 | comment: cite the current RFCs in the receive path and the state | — | 23 (part) |

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

Three more items came up during the work. Step 3 also reads the options of the crossing SYN,
which the source does in the Fast Open commit (26): without that, a simultaneous open used no
option of the peer. The tests need three additions to the test library: `readScript` (timed READ
commands) and `fork` (a listener that forks) in `TcpTestClient`, and the command `tsecr` of the
scripted tester. Master's SYN-ACK echoes TSecr 0, because the connection takes `ts_recent` only
from data (commit 39 repairs this in S17); so the TSecr check of step 8 never fires on master's
own SYN-ACKs, and its tests set the TSecr with the tester.

### S11 — `topic/tcp-connection-lifecycle` — landed 2026-10-09

The owner reviewed and approved S11 on 2026-10-09.

S11 brings ten changes to the setup and the close of a connection: three repairs of master, four
changes, two features that are off by default, and a comment. Where the source is wrong, S11
deviates from it, and a test fails with each source form (D-36 to D-41).

| Commit | B of D-6 | Source | Moved CI rows |
| --- | --- | --- | --- |
| `tcp: fix: clamp an explicit read at a received FIN` | — | 30 | — |
| `tcp: change: answer STATUS on a connection that never opened` | — | 38 | — |
| `tcp: fix: read and send the handshake options in a simultaneous open` | — | 16 (part), 26 (part) | — |
| `tcp: change: re-send the SYN-ACK for a retransmitted SYN` | — | 23 (part) | 2 |
| `tcp: change: report the duplicate SYN of a simultaneous open by D-SACK` | — | 22 (part), 23 (part) | — |
| `tcp: fix: close a forked connection on an RST or a timeout in SYN_RCVD` | — | 16 (part), 26 (part), 39 (part), 73 | — |
| `tcp: add: reset on data after a close, selected by abortOnDataAfterClose` | — | 23 (part), 39 (part) | — |
| `tcp: change: reset a SYN-ACK whose TSecr the connection did not send` | — | 16 (part), 74 | — |
| `tcp: add: the SYN cookie MSS of Linux, selected by syncookiesAlways` | — | 15 (part) | — |
| `tcp: comment: cite the current RFCs in the receive path and the state` | — | 23 (part) | — |

The commit messages explain each trace. In short: only the SYN-ACK resend moves CI rows. In
`bgpv4/BgpAndOspfv3` (t=11 s) and `manetrouting/multiradio` (t=4 s), a passive open in SYN_RCVD
gets a repeated SYN and re-sends its SYN-ACK at once, where it sent an ACK. The source form of
step 7 moved two more rows, `ethernet/arptest` and `arptest2` (D-41).

New tests: `tcp_read_fin_1`, `tcp_status_2`, `tcp_syn_10` to `tcp_syn_15`, `tcp_close_data_1` and
`_2`, and `tcp_syncookies_1`. Step 3 changes the expectations of `tcp_syn_2`, and step 4 those of
`tcp_syn_5`, `tcp_syn_6` and `tcp_syn_9`, from the source test commit 41 without its
`synLinearTimeouts` lines (S15).

Items for later stages: S12 takes from commit 23 the alignment and the padding of the options
(`alignOptions`), and from commit 26 the rest of the rules of a forked or a Fast Open connection
(`fastopenAccelerated`, the data on a crossing SYN); S13 the AccECN comments of commit 23; S17 the
SACK option from the recovery's `addSacks()`, the end of the reset of the duplicate-ACK counter in
the data path, and `ts_recent` from the handshake segment (commit 39).

Evidence, debug build against `omnetpp-6.x`: each commit builds alone with no undefined `inet::`
symbol, and the TCP module tests pass at each commit (411 at the first code commit, 421 at the
head); the TCP standards tests pass at each code commit (25 pass, 2 expected failures, the
packetdrill tests skip without inet-gpl); unit 118 and serializer 4 pass at the head. The tests of
steps 2 to 5, 7 and 9 fail one commit earlier; the tests that need the test library additions of
their own commit (steps 1, 6 and 8) were checked by hand against the code of the commit before,
and the tests of D-36 to D-41 against the source forms. The CI fingerprint job passes at each code
commit with the two rows of step 4. The TCP fingerprint rows run in debug at the head with no
runtime error (the `tyf` ingredient differs, as on master; the lwIP rows need a build with lwIP).
The six gates pass.

### S12 — `topic/tcp-fast-open`: the steps

| Step | Commit | B of D-6 | Source |
| --- | --- | --- | --- |
| 1 | fix: send nothing for a SEND of zero bytes | — | 43 (part) |
| 2 | fix: pad the option area to a multiple of 4 bytes | — | 23 (part) |
| 3 | add: options without NOP alignment, selected by `alignOptions` | — | 15, 16, 23 (parts) |
| 4 | add: a SYN without the MSS option, selected by `sendMssOption` | — | 15 (part) |
| 5 | fix: the default send MSS when the SYN has no MSS option (RFC 9293 MUST-15) | — | 15 (part) |
| 6 | fix: the text and the FIN on a SYN-ACK | — | 26 (part) |
| 7 | add: TCP Fast Open (RFC 7413), selected by two parameters | — | 26 (part), 27 (part), 39 (part), 43, 44, 15, 16 (parts) |
| 8 | add: Fast Open blackhole detection, selected by `fastopenBlackholeTimeout` | — | 26 (part) |
| 9 | add: the Fast Open option kind 254, selected by `fastopenExpOptionEnabled` | — | 26 (part), 43, 44 (parts) |
| 10 | add: the Linux Fast Open cookie key, selected by `fastopenKey` | — | 26 (part) |
| 11 | add: Fast Open without a cookie, selected by two parameters | — | 26 (part) |
| 12 | add: the limit of pending Fast Open connections (RFC 7413) | — | new (D-50) |
| 13 | comment: the connection states in the words of RFC 9293 | — | 26 (part) |

Commit 26 is the Fast Open feature, but it also holds changes that do not belong to Fast Open.
S12 divides the feature: the core (step 7) has both roles, because each role needs the other for
a test in INET, and the server that sends in SYN_RCVD, because its application gets the
connection at the SYN (D-46); blackhole detection, the experimental option, the Linux key and the
modes without a cookie follow in their own steps. The core takes from commits 15 and 16
`peerAdvertisedMss`, the Nagle exemption of the SYN-ACK slot and the data retransmission in
SYN_RCVD (D-19). Commits 43 and 44 repair Fast Open before it landed, so by D-1 they fold into
steps 7 and 9, except the zero-length SEND of commit 43 (step 1). By D-1, the core also takes the
data on the SYN from commit 39 and the SYN-ACK that keeps `snd_max` from commit 27 (D-47). The
core keeps the check of `seedRttFromHandshake` on the active side (D-27). Step 6 repairs the text
and the FIN of a SYN-ACK, which only another stack sends; the scripted tester gets the command
`data` for its test. The planned step for the order of the SYN options (commit 16) is dropped
(D-43).

The other parts of commit 26 go to the stages of their topics: the FIN of a close that arms the
retransmission timer only when it does not run and arms the loss probe (S15, timers); the check
of the sequence number that an ICMP error quotes (RFC 5927) and the hard ICMP errors in SYN_RCVD
(S17); the immediate SYN retransmission after an ICMP fragmentation-needed error in SYN_SENT (S18,
path MTU); the `ts_recent` update only from a segment that acknowledges no unsent data (S17, with
commit 39). Commit 51 needs nothing here: master already drops an unusable SACK option on receive
(S8), and its guard of the send path goes with S17, which moves the SACK option to the recovery's
`addSacks()`.

Items for later stages: (S17) master puts the text of a SYN without Fast Open into the receive
queue one byte early and does not move `rcv_nxt` over it; a peer that sends the bytes again
overwrites them, so INET's own peers see no effect. (S17) A passive connection does not send the
data that a SEND queued in SYN_RCVD when it reaches ESTABLISHED (RFC 9293: "Queue the data for
transmission after entering ESTABLISHED state"); only an application that sends raw commands can
reach it, because the socket of INET is not CONNECTED in SYN_RCVD. (inet-gpl) Its packetdrill
configuration maps no sysctl to `fastopenBlackholeTimeout` (D-48), and its application gets the
indications of D-46.

### S12 — `topic/tcp-fast-open` — landed 2026-10-09

The owner reviewed and approved S12 on 2026-10-09.

S12 brings TCP Fast Open (RFC 7413) and the option and MSS rules that it needs: four repairs of
master, eight features that are off by default, and a comment. Where the source is wrong or
incomplete, S12 deviates from it (D-42 to D-50).

| Commit | B of D-6 | Source | Moved CI rows |
| --- | --- | --- | --- |
| `tcp: fix: send nothing for a SEND of zero bytes` | — | 43 (part) | — |
| `tcp: fix: pad the option area to a multiple of 4 bytes` | — | 23 (part) | — |
| `tcp: add: options without NOP alignment, selected by alignOptions` | — | 15, 16, 23 (parts) | — |
| `tcp: add: a SYN without the MSS option, selected by sendMssOption` | — | 15 (part) | — |
| `tcp: fix: the default send MSS when the SYN has no MSS option` | — | 15 (part) | — |
| `tcp: fix: the text and the FIN on a SYN-ACK` | — | 26 (part) | — |
| `tcp: add: TCP Fast Open (RFC 7413), selected by two parameters` | — | 26 (part), 27 (part), 39 (part), 43, 44, 15, 16 (parts) | — |
| `tcp: add: Fast Open blackhole detection, selected by fastopenBlackholeTimeout` | — | 26 (part) | — |
| `tcp: add: the Fast Open option kind 254, selected by fastopenExpOptionEnabled` | — | 26 (part), 43, 44 (parts) | — |
| `tcp: add: the Linux Fast Open cookie key, selected by fastopenKey` | — | 26 (part) | — |
| `tcp: add: Fast Open without a cookie, selected by two parameters` | — | 26 (part) | — |
| `tcp: add: the limit of pending Fast Open connections (RFC 7413)` | — | new (D-50) | — |
| `tcp: comment: the connection states in the words of RFC 9293` | — | 26 (part) | — |

No commit moves a CI row: the features are off by default, and the paths of master that the
repairs and the core change give the same results in the CI scenarios.

New tests: `tcp_send_empty_1`, `tcp_options_pad_1`, `tcp_options_align_1`, `tcp_mss_option_2`,
`tcp_mss_default_1` and `_2`, `tcp_synack_data_1` and `_2`, and 16 tests `tcp_fastopen_*` (`_1`,
`optin_1`, `nosend_1`, `fork_1`, `srvdata_1` and `_2`, `rexmit_1`, `mss_1`, `synack_rexmit_1`,
`blackhole_1` and `_2`, `exp_1` and `_2`, `nocookie_1` and `_2`, `pending_1`); the unit tests
`tcp_fastopen_cookie_1`, `_cache_1` and `_key_1`. The test library gets an empty SEND, a second
and a third connection, `fastOpen` and `logIndications` in `TcpTestClient`, the commands `data`
and `strip` of the scripted tester, and the Fast Open options in the packet dump.

Evidence, debug build against `omnetpp-6.x`: each commit builds alone with no undefined `inet::`
symbol, and the TCP module tests pass at each commit (422 at the first code commit, 445 at the
head); the TCP standards tests pass at each code commit (25 pass, 2 expected failures, the
packetdrill tests skip without inet-gpl); unit 121 and serializer 4 pass at the head. The new
tests of steps 2 to 5, 8, 11 and 12 fail one commit earlier (`tcp_fastopen_blackhole_2`, the
control with the detection off, passes there too); the tests that need test library additions
of their own commit (steps 1, 6, 7 and 9) fail with the source of the commit before, and so do
the unit tests of steps 7 and 10. The CI fingerprint job passes at each code commit with no moved
row. The TCP fingerprint rows run in debug at the head with no runtime error (the `tyf` ingredient
differs, as on master; the lwIP rows need a build with lwIP). The six gates pass.

### S13 — `topic/tcp-accecn`: the steps

| Step | Commit | B of D-6 | Source |
| --- | --- | --- | --- |
| 1 | fix: originate ECT(0), not ECT(1) | — | 27 (part) |
| 2 | fix: no retransmission at a timeout with nothing in flight | — | new (D-52) |
| 3 | fix: no ECT on a SYN-ACK, a segment without data or a window probe | — | 27 (part) |
| 4 | fix: an ECN-setup SYN-ACK only for an ECN-setup SYN | — | 27 (part) |
| 5 | add: `tcpEcnMode`, which replaces `ecnWillingness` | — | 27 (part) |
| 6 | add: AccECN negotiation and the ACE field (RFC 9768) | B16 (part) | 27 (part), 65 (part) |
| 7 | add: `accEcnBareTimeWaitAck`, the Linux ACK in TIME_WAIT | — | 27 (part) |
| 8 | fix: read an AccECN option of any length | — | new |
| 9 | add: the AccECN option of the data receiver (RFC 9768) | — | 27 (part) |
| 10 | add: the AccECN option of the data sender (RFC 9768) | — | 27 (part) |
| 11 | add: no AccECN option after its ACKs got lost twice | — | 27 (part) |
| 12 | add: the Linux counts of a duplicate for `accEcnOptionPolicy` | — | 27 (part) |
| 13 | add: the AccECN fallback of a retransmitted SYN and SYN-ACK | — | 27 (part) |
| 14 | fix: ECN in force before the first segment after the SYN-ACK | — | 27 (part) |
| 15 | add: `ecnMarkAll`, and ECT on the SYN, SYN-ACK and RST of DCTCP | B16 (part) | 13 (part), 27 (part) |
| 16 | change: DCTCP counts its bytes also in a fast recovery (D-23) | — | new |
| 17 | comment: Accurate ECN in the module and header comments | — | 23, 48, 64 (parts) |

Commit 27 is the AccECN feature, but it also repairs classic ECN (D-51, D-59). Each repair is its
own step before the feature; step 2 (D-52) repairs a crash that the test of step 3 found. The
feature divides into the negotiation with the ACE field (step 6), the Linux ACK of TIME_WAIT
(step 7), the option of the receiver and of the sender (steps 9 and 10), the end of the option
after lost ACKs (step 11), the Linux counts of a duplicate (step 12) and the fallback (step 13).
Step 8 repairs the option parser of master for the lengths of RFC 9768 before the option uses it.
Step 15 adds `ecnMarkAll` of B16 and the ECT of DCTCP, and step 16 repairs D-23. The checks of
S13 against RFC 9768, RFC 8311, RFC 3168 and RFC 8257 found the differences of the source that
D-51 to D-60 record.

The non-AccECN parts of commit 27 that belong to other stages: the counts of `sendData()`
(`allowedToSend`, S16), and the second TS block of a SYN, which master never had (D-43).

Items for later stages: (S15, S16) the zero window probe of master sends new bytes beyond a zero
window, so `snd_max` grows, and the probes continue after the window opens. (S17) The option
writer skips CLOSING, LAST_ACK and TIME_WAIT, and `sendFin()` writes no options, so these
segments carry no TS option. (S17) Of the two `addSacks()` of master, only that of
`TcpConnection` runs; that of `Rfc6675Recovery` has no caller. (inet-gpl) Its packetdrill
configuration follows S13 with `accEcnOptionPolicy = "linux"` in place of `accEcnOptionBeaconAcks`
and `accEcnOptionKindAlternates`, `accEcnBareTimeWaitAck = true` and `ecnMarkAll = true`; a run
can check it only after S18.

### S13 — `topic/tcp-accecn` — landed 2026-10-12

The owner reviewed and approved S13 on 2026-10-12.

S13 brings Accurate ECN (RFC 9768) and the ECN repairs that it needs: six repairs of master,
nine additions, a change of DCTCP and a comment. The additions change nothing until a run selects
AccECN or a new parameter, except the ECT on the SYN, SYN-ACK and RST of DCTCP. Where the source
is wrong or incomplete, S13 deviates from it (D-51 to D-60).

| Commit | B of D-6 | Source | Moved CI rows |
| --- | --- | --- | --- |
| `tcp: fix: originate ECT(0), not ECT(1)` | — | 27 (part) | `dctcp` DcTcpIncast, TcpRenoIncast; `redmarker` TcpSender1 (~tND) |
| `tcp: fix: no retransmission at a timeout with nothing in flight` | — | new (D-52) | — |
| `tcp: fix: no ECT on a SYN-ACK, a segment without data or a window probe` | — | 27 (part) | the same three rows (~tND) |
| `tcp: fix: an ECN-setup SYN-ACK only for an ECN-setup SYN` | — | 27 (part) | — |
| `tcp: add: tcpEcnMode, which replaces ecnWillingness` | — | 27 (part) | — |
| `tcp: add: AccECN negotiation and the ACE field (RFC 9768)` | B16 (part) | 27 (part), 65 (part) | — |
| `tcp: add: accEcnBareTimeWaitAck, the Linux ACK in TIME_WAIT` | — | 27 (part) | — |
| `tcp_common: fix: read an AccECN option of any length` | — | new | — |
| `tcp: add: the AccECN option of the data receiver (RFC 9768)` | — | 27 (part) | — |
| `tcp: add: the AccECN option of the data sender (RFC 9768)` | — | 27 (part) | — |
| `tcp: add: no AccECN option after its ACKs got lost twice` | — | 27 (part) | — |
| `tcp: add: the Linux counts of a duplicate for accEcnOptionPolicy` | — | 27 (part) | — |
| `tcp: add: the AccECN fallback of a retransmitted SYN and SYN-ACK` | — | 27 (part) | — |
| `tcp: fix: ECN in force before the first segment after the SYN-ACK` | — | 27 (part) | — |
| `tcp: add: ecnMarkAll, and ECT on the SYN, SYN-ACK and RST of DCTCP` | B16 (part) | 13 (part), 27 (part) | `dctcp` DcTcpIncast (tplx, ~tNl, ~tND) |
| `tcp: change: DCTCP counts its bytes also in a fast recovery (D-23)` | — | new | — |
| `tcp: comment: Accurate ECN in the module and header comments` | — | 23, 48, 64 (parts) | — |

Three commits move CI rows. ECT(0) in place of ECT(1), and no ECT on the control segments,
change only the packet data (~tND) of the two DCTCP incast rows and the RED marker row: the same
packets go out with another ECN field. ECT on the DCTCP SYN and SYN-ACK changes the DCTCP incast
row in each ingredient: the congested switch marks these segments where it dropped them before
(RFC 8257 section 3.6). A first version of the fallback also cleared the flags of a classic ECN
SYN, and both incast rows moved; S13 keeps classic ECN as master (D-58).

New tests: `tcp_ecn_ect_1` to `_3`, `tcp_ecn_cwnd_1`, `tcp_ecn_synack_1`, `tcp_ecnmode_1` and
`_2`, 14 tests `tcp_accecn_*` (the negotiation, the ACE field, the time-wait ACK), 14 tests
`tcp_accecn_option_*`, 4 tests `tcp_accecn_fallback_*`, `tcp_ecn_estab_1`, 5 tests
`tcp_ecn_markall_*` and `tcp_dctcp_recovery_count_1`; the unit test
`tcp_accecn_option_wire_2`. The scripted tester gets the commands `ace`, `ect1`, `ect0` and
`zeroaccecn`, and it delivers the ECN codepoint of a segment; the packet dump shows the ECN
codepoint, the AE, CWR and ECE flags, and the AccECN options.

Evidence, debug build against `omnetpp-6.x`: each commit builds alone with no undefined `inet::`
symbol, and the TCP module tests pass at each commit (446 at the first code commit, 490 at the
head); the TCP standards tests pass at each code commit (25 pass, 2 expected failures, the
packetdrill tests skip without inet-gpl); unit 122 and serializer 4 pass at the head. The new
tests of each code commit fail with the source of the commit before (`tcp_ecnmode_2`, the control
of a passive client, passes there too). The CI fingerprint job runs at each code commit, and only
the rows above move. The TCP fingerprint rows run in debug at the head with no runtime error (the
`tyf` ingredient differs in 52 rows, as on master; the lwIP rows need a build with lwIP). The six
gates pass.

### S14 — `topic/tcp-receive-buffer`: the steps

| Step | Commit | B of D-6 | Source |
| --- | --- | --- | --- |
| 1 | add: `receiveBufferSize`, a receive buffer apart from the window | — | 28 (part) |
| 2 | add: the Linux acceptance of a segment beyond the buffer | — | 28 (part) |
| 3 | add: receive window auto-tuning, selected by `windowAutoTuning` | — | 28 (part) |
| 4 | add: `windowShrinkAllowed`, an offer that follows the free buffer | — | 28 (part) |
| 5 | add: the growth of the receive buffer of an owned connection | — | 28 (part) |
| 6 | add: the window and the receive buffer of one socket | — | new (D-4, D-63) |
| 7 | add: MSG_ZEROCOPY completions | — | 29 (part) |
| 8 | add: the time that the send buffer limits the sender | — | 29 (part) |
| 9 | add: the delivery time of received data (SO_TIMESTAMPING) | — | 29 (part) |
| 10 | add: TCP_NOTSENT_LOWAT, a low-water mark on unsent bytes | — | 35 |
| 11 | change: `timestampSupport` is `@mutable`, as the host-wide sysctl | — | new (D-4, D-63) |

Commit 28 separates the receive buffer from the advertised window, but it also changes the
default offer, so that out-of-order data no longer makes it smaller. S14 keeps the default of
master (D-61). The acceptance of an in-order segment beyond the buffer at an empty queue (Linux
`tcp_data_queue()`) is a Linux behavior; master drops such a segment, and step 2 adds the Linux
behavior with a parameter. Steps 3 to 5 model the receive memory of Linux (`tcp_grow_window()`,
`tcp_shrink_window=1` with the skb truesize, `tcp_clamp_window()`), each off by default (D-62).
Step 6 is the open command of D-4, and step 11 is the sysctl of D-4 (D-63). Commits 29 and 35 give
the socket options whose contract S3 landed (D-64); STATUS gets `skRcvbuf` (step 5) and
`sndbufLimited` (step 8).

Items for later stages: (S15, S16) The persist timer of master runs on when the send buffer is
empty, so a run with a zero window does not end; the tests of S14 with a zero window set
`sim-time-limit`. (S15) The flag of a segment beyond the offered edge comes with the adaptive
delayed ACK (D-62). (S18) The acceptance up to the highest edge ever offered, the upper limit of
the growth of the buffer, the lock by SO_RCVBUF, and a change of the buffer of an open connection
(D-62, D-63); the source sends that change with `TcpSocket::setReceiveBufferSize()`, and S18 adds
it to the function of S14, which sets the buffer of the next open, as Linux SO_RCVBUF acts before
and after the open. (inet-gpl) Its configuration sets `acceptBeyondBufferAtEmptyQueue = true`.
`PacketDrillApp` changes the module parameters for SO_RCVBUF before the open; it can use
`setAdvertisedWindow()` and `setReceiveBufferSize()` of its socket only when it builds against
master after S18, because the source has no such calls. A run can check both only after S18.

### S14 — `topic/tcp-receive-buffer` — landed 2026-10-12

The owner reviewed and approved S14 on 2026-10-12.

S14 brings the receive buffer apart from the advertised window, the Linux receive memory behind
parameters, the window and the buffer of one socket (D-4), and the socket options of commits 29
and 35, whose contract S3 landed. Each addition is off by default or waits for a call of the
application, so no CI row moves. Where the source is wrong or incomplete, S14 deviates from it
(D-61 to D-64).

| Commit | B of D-6 | Source | Moved CI rows |
| --- | --- | --- | --- |
| `tcp: add: receiveBufferSize, a receive buffer apart from the window` | — | 28 (part) | — |
| `tcp: add: the Linux acceptance of a segment beyond the buffer` | — | 28 (part) | — |
| `tcp: add: receive window auto-tuning, selected by windowAutoTuning` | — | 28 (part) | — |
| `tcp: add: windowShrinkAllowed, an offer that follows the free buffer` | — | 28 (part) | — |
| `tcp: add: the growth of the receive buffer of an owned connection` | — | 28 (part) | — |
| `tcp: add: the window and the receive buffer of one socket` | — | new (D-63) | — |
| `tcp: add: MSG_ZEROCOPY completions` | — | 29 (part) | — |
| `tcp: add: the time that the send buffer limits the sender` | — | 29 (part) | — |
| `tcp: add: the delivery time of received data (SO_TIMESTAMPING)` | — | 29 (part) | — |
| `tcp: add: TCP_NOTSENT_LOWAT, a low-water mark on unsent bytes` | — | 35 | — |
| `tcp: change: timestampSupport is @mutable, as the host-wide sysctl` | — | new (D-63) | — |

New tests: `tcp_rcvbuf_1` and `_2`, `tcp_rcvbuf_accept_1`, `tcp_rcvbuf_autotune_1`,
`tcp_rcvbuf_shrink_1`, `tcp_rcvbuf_grow_1` and `_2`, `tcp_socket_rcvbuf_1` and `_2`,
`tcp_zerocopy_1`, `tcp_sndbuf_limited_1`, `tcp_rx_timestamp_1` and `_2`, `tcp_notsent_lowat_1`
and `_2`, and `tcp_timestamp_sysctl_1`. The test client gets the parameters `owned`,
`socketAdvertisedWindow`, `socketReceiveBufferSize`, `zerocopy`, `writerBlockedScript`,
`timestamping`, `notsentLowat` and `tcpParScript`, and a second status line with `skRcvbuf` and
`sndbufLimited`.

Evidence, debug build against `omnetpp-6.x`: each commit builds alone with no undefined `inet::`
symbol, and the TCP module tests pass at each commit (492 at the first code commit, 506 at the
head); the TCP standards tests pass at each code commit (25 pass, 2 expected failures, the
packetdrill tests skip without inet-gpl); unit 122 and serializer 4 pass at the head. The new
tests of each code commit fail with the source of the commit before; there, the test library of
`tcp_socket_rcvbuf_1` and `_2` does not compile, because it calls the new socket functions. The
CI fingerprint job runs at each code commit, and no row moves. The TCP fingerprint rows run in
debug at the head with no runtime error (the `tyf` ingredient differs in 52 rows, as on master;
the lwIP rows need a build with lwIP). The six gates pass.

### S15 — `topic/tcp-timers`: the steps

| Step | Commit | B of D-6 | Source |
| --- | --- | --- | --- |
| 1 | add: the timer constants as parameters, with the values of master | — | 31 (part), 32 (part) |
| 2 | add: `rtoVarianceFloor`, the RTO of Linux | — | 68 (part) |
| 3 | add: `synRetries` and `synLinearTimeouts`, and the SYN timer limit | — | 32 (part) |
| 4 | add: the SYN retries of one socket (TCP_SYNCNT) | — | new (D-66) |
| 5 | add: an RTO of 3 s after a SYN timeout (RFC 6298 section 5.7) | — | 77 |
| 6 | add: `sendDataWithFirstAck` | — | 32 (part) |
| 7 | change: a FIN starts the retransmission timer only when it does not run | — | 26 (part) |
| 8 | add: adaptive delayed ACKs, selected by `adaptiveDelayedAcks` | — | 28 (part), 31 (part) |
| 9 | add: the time of the last received data in STATUS | — | 18 (part) |
| 10 | add: keepalive (RFC 9293 section 3.8.4) | — | 33 |
| 11 | add: keepalive for each connection (MUST-24) | — | new (D-66) |
| 12 | fix: no new fast recovery before the recovery point of a timeout | — | new (D-34) |

S15 had commits 31 to 34 and 77. The loss marking of commit 34 landed with S5e, and its zero-window
probe goes to S15b with the persist timing of commit 32 and the window update of commit 84 (B14,
B15). The timing parameters keep the values of master, and S19 changes the defaults (D-65). Commit
32 changed the RTO to the formula of Linux, and commit 68 put the formula of RFC 6298 back as the
default with `rtoVarianceFloor`; S15 brings only the parameter. The source changes the module
parameter `synRetries` at run time for the socket option TCP_SYNCNT, and it has no switch of
keepalive for one connection, which RFC 9293 MUST-24 requires; steps 4 and 11 add them as socket
calls (D-66). Step 7 follows RFC 6298 section 5.1: master restarted the timer at each FIN, also with
data in flight. Step 12 repairs D-34.

Items for later stages: (S15b) the zero window: the persist timing, the probe, and the window update
that reopens the window. (S19) the defaults of D-65. (inet-gpl) Its configuration sets the values of
Linux that the source has as defaults: `initialRto = 1s`, `maxRexmitTimeout = 120s` and
`synLinearTimeouts = 4`. `PacketDrillApp` changes the parameter `synRetries` for TCP_SYNCNT; when it
builds against master after S18, it uses `setSynRetries()`, because the parameter is not `@mutable`
on master. A run can check both only after S18.

### S15 — `topic/tcp-timers` — landed 2026-10-12

The owner reviewed and approved S15 on 2026-10-12.

S15 brings the timer parameters, the RTO of Linux, the SYN retries, an RTO of 3 s after a SYN
timeout, a FIN that keeps a running retransmission timer, the delayed ACK of Linux, keepalive, the
time of the last received data in STATUS, and the repair of D-34. Each addition is off by default or
keeps the value of master. Where the source is wrong or incomplete, S15 deviates from it (D-65 to
D-69).

| Commit | B of D-6 | Source | Moved CI rows |
| --- | --- | --- | --- |
| `tcp: add: the timer constants as parameters, with the values of master` | — | 31 (part), 32 (part) | — |
| `tcp: add: rtoVarianceFloor, the RTO of Linux` | — | 68 (part) | — |
| `tcp: add: synRetries and synLinearTimeouts, and the SYN timer limit` | — | 32 (part) | — |
| `tcp: add: the SYN retries of one socket (TCP_SYNCNT)` | — | new (D-66) | — |
| `tcp: add: an RTO of 3 s after a SYN timeout (RFC 6298 section 5.7)` | — | 77 | — |
| `tcp: add: sendDataWithFirstAck` | — | 32 (part) | — |
| `tcp: change: a FIN starts the retransmission timer only when it does not run` | — | 26 (part) | — |
| `tcp: add: adaptive delayed ACKs, selected by adaptiveDelayedAcks` | — | 28 (part), 31 (part) | — |
| `tcp: add: the time of the last received data in STATUS` | — | 18 (part) | — |
| `tcp: add: keepalive (RFC 9293 section 3.8.4)` | — | 33 | — |
| `tcp: add: keepalive for each connection (MUST-24)` | — | new (D-66) | — |
| `tcp: fix: no new fast recovery before the recovery point of a timeout` | — | new (D-34) | `bulktransfer` inet_inet_2b (tplx, ~tNl, ~tND) |

One commit moves a CI row. In the bulk transfer example, a server connection gets an RTO during a
SACK recovery at t=2.987, and a duplicate ACK at t=3.122 started a new recovery there; with the
repair of D-34 it does not. Each connection still sends up to the end of the run, with two
retransmissions fewer.

New tests: `tcp_rto_params_1` to `_3`, `tcp_delack_params_1`, `tcp_rto_variance_floor_1`,
`tcp_syn_retries_1`, `tcp_syn_linear_1`, `tcp_socket_synretries_1`, `tcp_rto_syn_fallback_1`,
`tcp_first_ack_1`, `tcp_fin_rexmit_timer_1`, `tcp_adaptive_delack_1` to `_3`,
`tcp_info_last_data_recv_1`, `tcp_keepalive_1` to `_4`, and `tcp_sack_rto_recovery_1`;
`tcp_frto_4` checks the repair of D-34. The test client gets the parameters `socketSynRetries` and
`keepAliveScript`, and a third status line with `lastDataRecvTime`.

Evidence, debug build against `omnetpp-6.x`: each commit builds alone with no undefined `inet::`
symbol, and the TCP module tests pass at each commit (510 at the first code commit, 526 at the
head); the TCP standards tests pass at each code commit (25 pass, 2 expected failures, the
packetdrill tests skip without inet-gpl); unit 122 and serializer 4 pass at the head. The new and
changed tests of each code commit fail with the source of the commit before; there, the test
library of `tcp_socket_synretries_1` and of `tcp_keepalive_3` and `_4` does not compile, because it
calls the new socket functions. The CI fingerprint job runs at each code commit, and only the row
above moves. The TCP fingerprint rows run in debug at the head with no runtime error (the `tyf`
ingredient differs in 52 rows, as on master; the lwIP rows need a build with lwIP). The six gates
pass.

### S15b — `topic/tcp-zero-window`: the steps

| Step | Commit | B of D-6 | Source |
| --- | --- | --- | --- |
| 1 | fix: the persist timer no longer changes the RTO | — | 32 (part) |
| 2 | fix: a zero-window probe sends the first unacknowledged octet again | — | new (D-70) |
| 3 | change: the first zero-window probe after one RTO, then doubled | B15 | 32 (part), 34 (part) |
| 4 | fix: a window update that opens the window ends the persist state | B14 | 13 (part), 84 (D-72) |
| 5 | fix: the persist timer runs only while data waits for a zero window | — | new (D-71) |
| 6 | add: the zero-window probe of Linux, selected by `zeroWindowProbe` | — | 34 (part) |

A test of master shows how its zero-window probing fails. Each probe is one new octet beyond the
closed window, so `snd_max` grows with each probe. The probe starts the retransmission timer, which
then sends the probe octets again with a doubled RTO, the ACKs of the probes count as duplicate
ACKs, and the persist timer writes its own interval into the RTO (5 s). The first probe comes after
5 s, not after one RTO. A window update that opens the window is ignored, so the persist timer keeps
probing after the window is open, and the data that waits is never sent. RFC 9293 section 3.8.6.1
asks for one octet of new data as the probe, or a retransmission, the first probe after one RTO and
exponential intervals (SHLD-29, SHLD-30), and that the connection stays open while the peer answers
the probes (MUST-37). Steps 1 to 5 repair master to that (D-70 to D-72); the probe without data of
the source (Linux `tcp_xmit_probe_skb()`) comes with a parameter (step 6). The plan had the start
and the stop of the persist timer as two steps; they are one rule, so step 5 has both.

Items for later stages: (S16) a sender that has a window smaller than one segment sends one octet,
as the tests of S15b show after the window closes again; commit 69 brings the rules of a small
window. (any stage) The receiver sends no window update when the application reads; Linux does
(`tcp_cleanup_rbuf()`), which RFC 9293 does not require, and neither master nor the source has it.
(any stage) The ACK processing of a segment sends data before the text of the same segment is
processed, so that data acknowledges the text later; Linux sends after the text. (S17) A pure ACK
carries `snd_nxt`, which goes back during a retransmission; BSD and Linux use the highest sequence
number sent. With that repair, a data segment of the peer can also update the send window (RFC 9293
section 3.10.7.4, D-72). (inet-gpl) Its
configuration sets `zeroWindowProbe = "linux"`; a run can check it only after S18.

### S15b — `topic/tcp-zero-window` — landed 2026-10-13

The owner reviewed and approved S15b on 2026-10-13.

S15b repairs the zero-window probing of master: the persist timer no longer changes the RTO, the
probe repeats one octet without the retransmission timer, the first probe comes after one RTO with
doubled intervals, a window update ends the persist state, and the persist timer runs only while
data waits. It adds the probe of Linux behind a parameter. No CI row moves. Where the source is
wrong or incomplete, S15b deviates from it (D-70 to D-72).

| Commit | B of D-6 | Source | Moved CI rows |
| --- | --- | --- | --- |
| `tcp: fix: the persist timer no longer changes the RTO` | — | 32 (part) | — |
| `tcp: fix: a zero-window probe sends the first unacknowledged octet again` | — | new (D-70) | — |
| `tcp: change: the first zero-window probe after one RTO, then doubled` | B15 | 32 (part), 34 (part) | — |
| `tcp: fix: a window update that opens the window ends the persist state` | B14 | 13 (part), 84 (D-72) | — |
| `tcp: fix: the persist timer runs only while data waits for a zero window` | — | new (D-71) | — |
| `tcp: add: the zero-window probe of Linux, selected by zeroWindowProbe` | — | 34 (part) | — |

New tests: `tcp_persist_1` to `_4` and `tcp_persist_linux_1`, which also checks a window update
with nothing in flight. `tcp_info_fields_2`, `tcp_ecn_ect_3`
and `tcp_ecn_cwnd_1` follow the new probe times and the probe octet that no longer goes again.

Evidence, debug build against `omnetpp-6.x`: each commit builds alone with no undefined `inet::`
symbol, and the TCP module tests pass at each commit (527 at the first code commit, 531 at the
head); the TCP standards tests pass at each code commit (25 pass, 2 expected failures, the
packetdrill tests skip without inet-gpl); unit 122 and serializer 4 pass at the head. The new and
changed tests of each code commit fail with the source of the commit before. The CI fingerprint job
runs at each code commit, and no row moves. The TCP fingerprint rows run in debug at the head with
no runtime error (the `tyf` ingredient differs in 52 rows, as on master; the lwIP rows need a build
with lwIP). The six gates pass.

### S16 — `topic/tcp-send-rules`: the steps

| Step | Commit | B of D-6 | Source |
| --- | --- | --- | --- |
| 1 | fix: the sender avoids the silly window syndrome (RFC 9293 MUST-38) | — | 15 (part), 16 (part), 69, new (D-73) |
| 2 | add: `TCP_NODELAY` for one socket | — | 36 (part) |
| 3 | add: the Nagle of Minshall, selected by `minshallNagle` | — | 36 (part) |
| 4 | add: `TCP_MAXSEG` for one socket | — | 15 (part) |

S16 had commits 15 and 16 (parts), 36 and 69. Commit 36 brings two kinds of work: the send rules
(Nagle, `TCP_NODELAY`) and the write boundaries (`TCP_CORK`, `MSG_MORE`, `MSG_EOR`, the PSH of
Linux). S16 takes the send rules, and a new stage S16b takes the write boundaries, so that each
stage stays small enough to review. Master has no sender SWS avoidance, which RFC 9293 section
3.8.6.2.1 requires (MUST-38); the source has no override timeout for it (D-73). The plan had a
step for PSH in the last buffered segment (MUST-61); it goes to S19, because it changes the
packet dump of most module tests (D-74). The Nagle of master is that of RFC 9293 section 3.7.4;
step 3 adds the variant of Minshall behind a parameter (D-75). The counts of `sendData()` of
commit 27 (`allowedToSend`) need no change: master computes the effective window with signed
arithmetic, so it does not underflow.

Items for later stages: (S16b) `TCP_CORK`, `MSG_MORE`, `MSG_EOR` and the PSH of Linux at write
boundaries. (S19) PSH in the last buffered segment, with a parameter (D-74). (inet-gpl) Its
configuration sets `minshallNagle = true`; a run can check it only after S18.

### S16 — `topic/tcp-send-rules` — landed 2026-10-13

The owner reviewed and approved S16 on 2026-10-13.

S16 brings the sender SWS avoidance of RFC 9293 with its override timeout, the socket options
`TCP_NODELAY` and `TCP_MAXSEG`, and the Nagle of Minshall behind a parameter. Where the source is
wrong or incomplete, S16 deviates from it (D-73 to D-75).

| Commit | B of D-6 | Source | Moved CI rows |
| --- | --- | --- | --- |
| `tcp: fix: the sender avoids the silly window syndrome (RFC 9293 MUST-38)` | — | 15 (part), 16 (part), 69, new | `dctcp` DcTcpIncast, TcpRenoIncast; `redmarker` TcpSender1; `bulktransfer` inet_inet_2b |
| `tcp: add: TCP_NODELAY for one socket` | — | 36 (part) | — |
| `tcp: add: the Nagle of Minshall, selected by minshallNagle` | — | 36 (part) | — |
| `tcp: add: TCP_MAXSEG for one socket` | — | 15 (part) | — |

One commit moves CI rows. In the two DCTCP incast rows and the RED marker row, a segment smaller
than the MSS that the congestion window limits now waits for the next ACK, as BSD holds it; in the
bulk transfer row, a peer window of 195 bytes no longer gets a segment of 175 bytes (rule (3)).

New tests: `tcp_sws_1` and `_2`, `tcp_nodelay_1`, `tcp_nagle_minshall_1` and `tcp_maxseg_1`.
`tcp_timestamp_2` and `tcp_zerocopy_1` send full segments in place of small ones, and
`tcp_persist_4` follows a new log text. The test client gets the parameters `noDelayScript` and
`maxSegScript`.

Evidence, debug build against `omnetpp-6.x`: each commit builds alone with no undefined `inet::`
symbol, and the TCP module tests pass at each commit (533 at the first code commit, 536 at the
head); the TCP standards tests pass at each code commit (25 pass, 2 expected failures, the
packetdrill tests skip without inet-gpl); unit 122 and serializer 4 pass at the head. The new and
changed tests of each code commit fail with the source of the commit before, except `tcp_sws_1`,
a control of rule (3): master sends each segment that the window allows, and the test guards
against the defect that source commit 69 repairs. The CI fingerprint job runs at each code commit,
and only the rows above move. The TCP fingerprint rows run in debug at the head with no runtime
error (the `tyf` ingredient differs in 52 rows, as on master; the lwIP rows need a build with
lwIP). The six gates pass.

### S16b — `topic/tcp-write-boundaries`: the steps

| Step | Commit | B of D-6 | Source |
| --- | --- | --- | --- |
| 1 | add: `TCP_CORK` and `MSG_MORE`, with the cork timer | — | 36 (part), new (D-76) |
| 2 | add: `MSG_EOR`, record boundaries that no segment spans | — | 36 (part), new (D-77) |
| 3 | add: the PSH of Linux at write boundaries, by a parameter | — | 36 (part), 46 (part), new (D-78) |

S16b takes the write boundaries of commit 36: the socket options `TCP_CORK` and `MSG_MORE`, which
hold the partial segment at the end of a write, the record boundary of `MSG_EOR`, and the PSH that
Linux sets at the end of a write, in a large write (`forced_push`) and on each burst of segments
(GSO). No RFC has these; each acts only when an application asks for it or the Linux parameter is
on. The PSH in the last buffered segment of RFC 9293 (MUST-61) comes with S19 (D-74). S16b takes
the size of a GSO burst and `TcpAlgorithm::getSrtt()` from commit 46; S18 takes the rest of that
commit. Where Linux and the source differ, S16b follows Linux: the cork timer, Nagle after an
uncork (D-76), and the PSH and Nagle of a record (D-77).

Items for later stages: (inet-gpl) its configuration sets `pushSegmentsOnWriteBoundary = true`
already; a run can check the cork, `MSG_MORE` and `MSG_EOR` scripts only after S18.

### S16b — `topic/tcp-write-boundaries` — landed 2026-10-13

The owner reviewed and approved S16b on 2026-10-13.

S16b brings `TCP_CORK`, `MSG_MORE` and `MSG_EOR`, and the PSH of Linux at write boundaries behind
a parameter. Where the source is wrong or differs from Linux, S16b deviates from it (D-76 to D-78).

| Commit | B of D-6 | Source | Moved CI rows |
| --- | --- | --- | --- |
| `tcp: add: TCP_CORK and MSG_MORE, with the cork timer` | — | 36 (part), new | — |
| `tcp: add: MSG_EOR, record boundaries that no segment spans` | — | 36 (part), new | — |
| `tcp: add: the PSH of Linux at write boundaries, by a parameter` | — | 36 (part), 46 (part), new | — |

New tests: `tcp_cork_1` to `_4`, `tcp_msg_more_1`, `tcp_eor_1` to `_3` and `tcp_push_write_1` to
`_3`. The test client gets the parameter `corkScript` and the `sendScript` flags `more` and `eor`.

Evidence, debug build against `omnetpp-6.x`: each commit builds alone with no undefined `inet::`
symbol, and the TCP module tests pass at each commit (541 at the first code commit, 547 at the
head); the TCP standards tests pass at each code commit (25 pass, 2 expected failures, the
packetdrill tests skip without inet-gpl); unit 122 and serializer 4 pass at the head. The new tests
of each code commit fail with the source of the commit before. The CI fingerprint job runs at each
code commit, and no row moves. The TCP fingerprint rows run in debug at the head with no runtime
error (the `tyf` ingredient differs in 52 rows, as on master; the lwIP rows need a build with
lwIP). The packetdrill suite of inet-gpl, with the comments of its follow-up and against the source
branch, gives 303 MATCH, 12 KERNEL_DRIFT and 3 UNSUPPORTED_FEATURE, and no script changes its
verdict. The six gates pass.

### S17 — `topic/tcp-connection-leftovers`: the steps

| Step | Commit | B of D-6 | Source |
| --- | --- | --- | --- |
| 1 | refactor: remove the copies of the SACK recovery that nothing calls | — | 39 (part), new (D-79) |
| 2 | refactor: walk the SACK scoreboard once, and cache its totals | — | 61 |

S17 had commits 39 and 61. Commit 39 is the remainder of the source branch's cut into features:
it holds fourteen changes that belong to no one feature. Earlier stages took six of them (the
retransmission queue without SACK and the inferred SACKs in S5c, the delivered bytes in S9, the
receive shutdown at a full close and the forked connection in S11, the data of a Fast Open SYN in
S12). The rest, with the items that earlier stages gave to S17, go to four stages by topic, so that
each stays small enough to review: S17 removes dead code and makes the scoreboard scan cheaper,
S17b brings the timestamp rules, S17c the checks of a received segment and the sequence number of a
pure ACK, and S17d the connection setup and the ICMP errors. The comments that cite RFC 793 and
RFC 1323 go with the RFC list of S20.

Master has two copies of the SACK recovery. The connection's copy (`isLost()`, `setPipe()`,
`nextSeg()`, `sendDataDuringLossRecoveryPhase()`, `sendSegmentDuringLossRecoveryPhase()`) has no
caller since S5d; `Rfc6675Recovery` runs. The recovery's `addSacks()`,
`sendDataDuringLossRecoveryPhase()` and `sendSegmentDuringLossRecoveryPhase()` have no caller
either. The SACK option comes from the connection's `addSacks()`, which has the repairs of S8 (the
first block of RFC 2018 section 4) and S13 (the room for the AccECN option); the recovery's copy
has neither. The source moves the option to the recovery's copy and reaches it with a
`dynamic_cast` to `Rfc6675Recovery` (commit 51 guards that cast), so a connection whose recovery is
another one, or one in SYN_RCVD before its recovery exists, sends no SACK option. The SACK option
reports what the receiver holds (RFC 2018); it does not belong to the sender's loss recovery. Step
1 keeps the connection's `addSacks()` in `TcpConnectionSackUtil.cc` and removes the other copies
(D-79).

Commit 61 walks the scoreboard once to get the lost, SACKed and retransmitted bytes, caches the
totals until the queue changes, and walks `setPipe()` with one iterator. Master's queue has more
methods that change it than the source had when commit 61 was written (`discardFrom()` of S15b);
each one clears the cache. A debug build walks the queue at each read and checks the cache, so a
method that forgets to clear it fails a test.

S17 leaves four parts of commit 39 out (D-80). The connection keeps the reset of the duplicate-ACK
counter after the ACK handlers of the algorithm (B1 of D-6): the source resets the counter at the
end of `TcpAlgorithmBase::receivedAckForUnackedData()`, before the code of the flavour, so the
`dupacks = dupthresh` that `TcpVegas` sets after its fine-grained retransmission stays, where
master resets it. `validationInfo()` has no caller in the source. The rename of the statistic
`sndNxt` to `sndSeq` would make INET's TCP differ from the lwIP TCP, which records `sndNxt` too.
The range that `retransmitOneSegment()` reports differs from the source's only in SYN_RCVD, where
the list of send times ignores it.

Items for later stages: (S17b to S17d) as the table says. (S20) the comments that cite RFC 793 and
RFC 1323; `sndSeqSignal` is registered and never emitted; the `sndMax` signal misses the changes of
`snd_max` at a FIN, at a window probe and in `sendOneNewSegment()`, but no statistic records it.
(any stage) `TcpVegas` sets `dupacks = dupthresh` after its fine-grained retransmission at a new
ACK, and the connection resets the counter right after it, so the assignment has no effect.

### S17 — `topic/tcp-connection-leftovers` — landed 2026-10-13

The owner reviewed and approved S17 on 2026-10-13.

S17 removes the copies of the SACK recovery that nothing calls, keeps the SACK option with the
connection (D-79), and walks the SACK scoreboard once with cached totals. It leaves four parts of
commit 39 out (D-80). Both commits keep master's behavior.

| Commit | B of D-6 | Source | Moved CI rows |
| --- | --- | --- | --- |
| `tcp: refactor: remove the copies of the SACK recovery that nothing calls` | — | 39 (part), new | — |
| `tcp: refactor: walk the SACK scoreboard once, and cache its totals` | — | 61 | — |

Evidence, debug build against `omnetpp-6.x`: each commit builds alone with no undefined `inet::`
symbol, and the TCP module tests pass at each commit (555); the TCP standards tests pass at each
code commit (25 pass, 2 expected failures, the packetdrill tests skip without inet-gpl); unit 122
and serializer 4 pass at the head. The debug build checks the cache of the scoreboard totals at
each read. The CI fingerprint job runs at each code commit, and no row moves; its release build
reads the cache without the check. The TCP fingerprint rows run in debug at the head with no
runtime error (the `tyf` ingredient differs in 52 rows, as on master; the lwIP rows need a build
with lwIP). S17 changes no behavior, so inet-gpl needs no change. The six gates pass.

### S17b — `topic/tcp-timestamps`: the steps

| Step | Commit | B of D-6 | Source |
| --- | --- | --- | --- |
| 1 | fix: the SYN-ACK and the handshake ACK echo the timestamp of the peer | — | 39 (part), new (D-81) |
| 2 | fix: PAWS drops a segment with an old timestamp (RFC 7323 section 5.3) | — | new (D-82) |
| 3 | fix: `TS.Recent` only from a segment that passes the window test | — | new (D-82) |
| 4 | fix: the TS option on each segment after the handshake | — | 39 (part), new (D-83) |
| 5 | add: the `TS.Recent` rule of Linux, selected by `tsRecentAfterAckCheck` | — | 26 (part) |

S17b takes the timestamp rules of RFC 7323. RFC 7323 appendix D saves the TSval of a SYN in LISTEN,
and of a SYN-ACK in SYN_SENT, in `TS.Recent`; master takes `TS.Recent` only from a later segment, so
its SYN-ACK and its handshake ACK echo TSecr 0 (step 1, from commit 39).

Master's PAWS test is inverted: section 5.3 (R1) drops a segment with SEG.TSval < TS.Recent while
`TS.Recent` is valid, that is, while the connection was idle for no more than 24 days (section
5.5); master drops it only after 24 days of idle time, so PAWS never drops a segment. The source has
the same test. Section 5.2 also exempts a RST from PAWS and forbids an update of `TS.Recent` from
it (step 2). Section 5.3 updates `TS.Recent` (R3) only after the window test (R2); master reads the
options before the window test, so an old duplicate outside the window sets `TS.Recent` (step 3).

Section 3.2: "the TSopt MUST be sent in every non-<RST> segment for the duration of the connection,
and SHOULD be sent in an <RST> segment". Master's FIN has no options, the option writer skips
CLOSING, LAST_ACK and TIME_WAIT, and the RST of an abort has no option; the receiver refuses a TS
option in CLOSE_WAIT, CLOSING, LAST_ACK and TIME_WAIT, so it does not update `TS.Recent` there
(step 4). A RST that answers a segment stays without the option: section 5.2 asks for TSval 0 in
it, "to enable research", which Linux does not do.

Linux updates `TS.Recent` only after the ACK of the segment passes its check
(`tcp_replace_ts_recent()` in `tcp_ack()`), so a segment that acknowledges unsent data does not set
it; section 5.3 updates it before the ACK is processed. Commit 26 takes the Linux rule; step 5
selects it with a parameter, whose default follows the RFC.

Items for later stages: (inet-gpl) its configuration sets `tsRecentAfterAckCheck = true`; a run
can check it only after S18. (any stage) A RST that answers a segment carries no TS option (D-83).
(any stage) PAWS of Linux accepts an older TSval within one tick and on a reordered pure ACK
(`tcp_disordered_ack()`); INET follows R1 and answers such an ACK (D-82).

### S17b — `topic/tcp-timestamps` — landed 2026-10-13

The owner reviewed and approved S17b on 2026-10-13.

S17b brings the timestamp rules of RFC 7323: `TS.Recent` from the handshake, PAWS, the update after
the window test, and the TS option on each segment after the handshake, with the update rule of
Linux behind a parameter. Where master and the source are wrong, S17b deviates from them (D-81 to
D-83).

| Commit | B of D-6 | Source | Moved CI rows |
| --- | --- | --- | --- |
| `tcp: fix: the SYN-ACK and the handshake ACK echo the timestamp of the peer` | — | 39 (part), new | `netperfmeter` TraceFile-Tcp (`~tND` only) |
| `tcp: fix: PAWS drops a segment with an old timestamp (RFC 7323 section 5.3)` | — | new | — |
| `tcp: fix: TS.Recent only from a segment that passes the window test` | — | new | — |
| `tcp: fix: the TS option on each segment after the handshake` | — | 39 (part), new | — |
| `tcp: add: the TS.Recent rule of Linux, selected by tsRecentAfterAckCheck` | — | 26 (part) | — |

One commit moves a CI row: in the netperfmeter row, the ACKs after the handshake echo the TSval of
the SYN-ACK in place of 0, so only the packet data (`~tND`) changes, not the times or the lengths.

New tests: `tcp_timestamp_3`, `tcp_paws_1`, `tcp_ts_recent_1`, `tcp_ts_recent_2` (a control of the
default), `tcp_ts_recent_linux_1`, `tcp_timestamp_close_1` and `tcp_timestamp_abort_1`.
`tcp_timestamp_1`, `tcp_timestamp_2`, `tcp_timestamp_sysctl_1` and `tcp_options_align_1` echo the
TSval of the SYN-ACK after the handshake. The test client gets the parameter `tAbort`, and the
scripted tester the command `ackadd`.

Evidence, debug build against `omnetpp-6.x`: each commit builds alone with no undefined `inet::`
symbol, and the TCP module tests pass at each code commit (556 at the first code commit, 562 at
the head); at the plan commit one module test failed once in the series and passed in a run again
(555), and the series did not record its name. The TCP standards tests pass at each code commit (25
pass, 2 expected failures, the packetdrill tests skip without inet-gpl); unit 122 and serializer 4
pass at the head. The new and changed tests of each code commit fail with the source of the commit
before, except `tcp_ts_recent_2`, a control of the default of `tsRecentAfterAckCheck`, which master
passes too. The CI fingerprint job runs at each code commit, and only the row above moves. The TCP
fingerprint rows run in debug at the head with no runtime error (the `tyf` ingredient differs in 52
rows, as on master; the lwIP rows need a build with lwIP). The six gates pass.

### S17c — `topic/tcp-segment-checks`: the steps

| Step | Commit | B of D-6 | Source |
| --- | --- | --- | --- |
| 1 | change: the log of an empty segment shows its sequence number | — | new |
| 2 | fix: a pure ACK carries the highest sequence number sent | — | new (D-72, D-84) |
| 3 | fix: a data segment of the peer updates the send window | — | new (D-72, D-84) |
| 4 | fix: the FIN of a segment of old data passes the window test | — | 39 (part), D-85 |
| 5 | add: the ACK check of RFC 5961 section 5.2, by a parameter | — | 39 (part), D-86 |
| 6 | add: the window-edge check of Linux, by a parameter | — | 39 (part), D-86 |
| 7 | fix: the RST of an abort carries the highest sequence number sent | — | new (D-84) |
| 8 | add: the RST with ACK of Linux at an abort, by a parameter | — | 39 (part), D-86 |

S17c takes the checks of a received segment from commit 39 and the two repairs that S15b deferred
(D-72). A pure ACK of master carries `snd_nxt`, which goes back to `snd_una` during a
retransmission; the SND.NXT of RFC 9293 never goes back, and BSD and Linux put the highest sequence
number sent (`snd_max`) on a pure ACK (step 1). With that, the window update of RFC 9293 section
3.10.7.4 can also take a data segment of the peer, as S15b found (step 2).

RFC 9293 counts the SYN and the FIN in SEG.LEN. Master's window test measures only the data, so a
segment that carries old data and a FIN at RCV.NXT, as a peer sends when it retransmits its last
data together with its FIN, fails the test, and the FIN is lost (step 3). A segment without data
keeps master's test.

Three rules of commit 39 are Linux behavior that the RFCs do not require, so each comes with a
parameter whose default keeps master's behavior: the ACK check of RFC 5961 section 5.2, which
RFC 9293 allows (MAY-12) and Linux uses (step 4); the drop of a segment whose end lies beyond the
highest window edge advertised (`rcv_adv`), unless the in-order receive queue is empty (Linux
`tcp_sequence()`, step 5); and the RST with ACK and `rcv_nxt` at an abort (Linux
`tcp_send_active_reset()`), where RFC 9293 sends `<SEQ=SND.NXT><CTL=RST>` (step 6).

During the work, step 1 came first, so that the tests of steps 2 and 7 can read the sequence number
of a pure ACK and of a RST, and step 7 came up as a new defect: the RST of an abort carried
`snd_nxt` too.

Items for later stages: (inet-gpl) its configuration sets `rfc5961AckCheck`,
`dropBeyondWindowEdge` and `abortRstWithAck` to true; a run can check them only after S18. (S20)
the RFC list says that RFC 5961 is modeled only in section 5.2 (D-86). (any stage) The ACK
processing of a segment sends data before the text of the same segment is processed, so that data
acknowledges the text later, as `tcp_wnd_update_data_1` shows.

### S17c — `topic/tcp-segment-checks` — landed 2026-10-13

The owner reviewed and approved S17c on 2026-10-13.

S17c repairs the sequence number of a pure ACK and of the RST of an abort, takes the window from a
data segment of the peer, counts the FIN in the window test, and brings three rules of Linux behind
parameters. Where master and the source are wrong, S17c deviates from them (D-84 to D-86).

| Commit | B of D-6 | Source | Moved CI rows |
| --- | --- | --- | --- |
| `tcp: change: the log of an empty segment shows its sequence number` | — | new | — |
| `tcp: fix: a pure ACK carries the highest sequence number sent` | — | new | `arptest` ARPTest, `arptest2` ARPTest, `bulktransfer` inet__inet, inet_inet_2a, inet_inet_2b, `pcaprecorder` General, `tcp_ppp_reconnect` General, `tcpclientserver` default_tcp, inet__inet, inet__inet_msg, inet__inet_stream; `tcp_eth_reconnect` (six rows, `~tND` only) |
| `tcp: fix: a data segment of the peer updates the send window` | — | new | `bulktransfer` inet_inet_2b; `tcp_eth_reconnect` (four rows, `tplx` only) |
| `tcp: fix: the FIN of a segment of old data passes the window test` | — | 39 (part) | — |
| `tcp: add: the ACK check of RFC 5961 section 5.2, by a parameter` | — | 39 (part) | — |
| `tcp: add: the window-edge check of Linux, by a parameter` | — | 39 (part) | — |
| `tcp: fix: the RST of an abort carries the highest sequence number sent` | — | new | — |
| `tcp: add: the RST with ACK of Linux at an abort, by a parameter` | — | 39 (part) | — |

Two commits move CI rows. With the sequence number of a pure ACK, the peer takes the window of an
ACK that it gets during a retransmission of the other end: the client takes the window 7504 of the
server's ACK, counts the next ACKs as duplicates, and retransmits earlier; in the ARP rows, the
client no longer answers such an ACK as unacceptable. In the tcp_eth_reconnect rows, only the
sequence numbers in the packet data change. With the window update from data, a sender whose peer
sends data too sends one ACK earlier.

New tests: `tcp_ack_seq_1`, `tcp_wnd_update_data_1`, `tcp_fin_old_data_1`, `tcp_rfc5961_ack_1`,
`tcp_window_edge_1`, `tcp_abort_rst_seq_1` and `tcp_abort_rst_ack_1`.

Evidence, debug build against `omnetpp-6.x`: each commit builds alone with no undefined `inet::`
symbol, and the module tests pass at each commit (565 at the plan commit, 572 at the head); in the
run of the stage tests at the head, `ExternalProcess_4`, which runs an outside process, failed once
and passed in a run again. The TCP standards tests pass at each code commit (25 pass, 2 expected
failures, the packetdrill tests skip without inet-gpl); unit 122 and serializer 4 pass at the head.
The new tests of each code commit fail with the source of the commit before. The CI fingerprint job
runs at each code commit, and only the rows above move. The TCP fingerprint rows run in debug at the
head with no runtime error (the `tyf` ingredient differs in 52 rows, as on master; the lwIP rows
need a build with lwIP). The six gates pass. After a rebase onto 17 new commits of master, each
commit builds alone, the module tests pass at the head (579), the CI fingerprint job passes at the
head, and the combined pull-request gate passes.

### S17d — `topic/tcp-connection-setup`: the steps

| Step | Commit | B of D-6 | Source |
| --- | --- | --- | --- |
| 1 | fix: the text of a SYN without Fast Open goes to the application after the handshake | — | new |
| 2 | fix: a hard ICMP error aborts a connection in SYN_RCVD too | — | 26 (part) |
| 3 | add: the soft ICMP errors of Linux abort a connection setup, by a parameter | — | 26 (part), 39 (part) |
| 4 | add: the sequence check of an ICMP error of Linux, by a parameter | — | 26 (part) |
| 5 | fix: the data of a Fast Open SYN completes its zero-copy write | — | 39 (part) |

S17d takes the connection setup and the ICMP errors from commits 26 and 39. RFC 9293 queues the
text of a SYN in LISTEN "for processing later"; master inserts it into the receive queue at
SEG.SEQ, one octet too early, because the SYN takes SEG.SEQ, and never moves RCV.NXT over it; the
source does the same (step 1). RFC 9293 section 3.9.2.2 says that a hard ICMP error SHOULD abort the
connection (SHLD-26); master aborts only in SYN_SENT, also for a connection in SYN_RCVD, which has
no synchronized state yet (step 2).

Two rules of the source are Linux behavior, so parameters select them, with defaults that keep
master's behavior: a soft ICMP error (IPv4 net or host unreachable) aborts a connection setup, as
Linux `tcp_v4_err()` does, where RFC 9293 forbids it (MUST-56) and RFC 5461 section 4 describes the
practice (step 3); and an ICMP error whose quoted sequence number lies outside SND.UNA to SND.MAX is
ignored, which RFC 5927 section 4.1 describes and Linux and the BSDs do, but no RFC requires (step
4). The data of a Fast Open SYN goes out without `sendSegment()`, so its zero-copy completion never
came (step 5).

Two items of earlier stages need no change. The algorithm of a connection that returns from
SYN_RCVD to LISTEN has no timer: since S11, only a connection that is neither forked nor Fast Open
returns there, and its algorithm starts at ESTABLISHED. A passive connection never gets data that a
SEND queued in SYN_RCVD: INET's socket refuses a SEND before the connection is established, and an
application that sends raw commands is the only way there.
