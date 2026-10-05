# Land the TCP modernization on master in small stages

Status: **in progress** — S1 and S2 landed on master on 2026-10-02, S3, S4 and S5 on 2026-10-05, S5b on 2026-10-06. S5c is next.

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
| S5c | `topic/tcp-classic-recovery` | 13 (part), 54, 55 (part), 56, 70 | — | the behavior changes B4 to B8, B10, B11 of D-6, one change per commit |
| S5d | `topic/tcp-sack-recovery` | 13 (part) | — | SACK loss recovery in `Rfc6675Recovery`, and DCTCP on it: B12, B13, B16 of D-6 |
| S6 | `topic/tcp-cubic` | 14, 45, 55 (`TcpCubic` part), 57, 60, 62, 65 | +840 −326 | `TcpCubic` with HyStart, and `DcTcp` on the shared ACK path |
| S7 | `topic/tcp-segment-sizing` | 15, 16, 53 | +420 −54 | segment sizing against the option space, bytes in flight |
| S8 | `topic/tcp-rack` | 17, 18, 19, 49 | +768 −47 | RACK loss detection (RFC 8985), STATUS counters, the reordering window |
| S9 | `topic/tcp-prr` | 20, 59 | +143 −14 | Proportional Rate Reduction (RFC 6937) |
| S10 | `topic/tcp-undo-frto-tlp` | 22, 24, 25, 76, 85 | +505 −7 | spurious-loss undo, F-RTO (RFC 5682), the tail loss probe |
| S11 | `topic/tcp-connection-lifecycle` | 23, 30, 38, 73 | +179 −45 | reset after a full close, SYN-ACK re-send, FIN read clamp, STATUS before open |
| S12 | `topic/tcp-fast-open` | 26, 43, 44, 51 | +1187 −104 | TCP Fast Open (RFC 7413) |
| S13 | `topic/tcp-accecn` | 27 | +864 −61 | Accurate ECN |
| S14 | `topic/tcp-receive-buffer` | 28, 29, 35, new work | +400 −3 | the receive buffer apart from the advertised window, zero-copy, `TCP_NOTSENT_LOWAT`, the receive buffer of a socket before open (D-4) |
| S15 | `topic/tcp-timers` | 31, 32, 33, 34, 77 | +441 −94 | adaptive delayed ACK, timer parameters, keepalive, loss marking at a timeout |
| S16 | `topic/tcp-write-boundaries` | 36, 37, 69 | +355 −9 | PSH at write boundaries, `TCP_CORK`, a window smaller than one MSS |
| S17 | `topic/tcp-connection-leftovers` | 39, 61, 74, 86 | +445 −185 | the remaining connection work, the SACK scoreboard scan, two repairs |
| S18 | `topic/tcp-pmtud-rcvbuf` | 46, 47, 42, 58 | +556 −65 | RFC 4821 path MTU discovery, receive buffer memory, the applications |
| S19 | `topic/tcp-modern-defaults` | 40, 63, 68, 70, 71, 72, 78, 79, 80, 81, 82, 87 | +863 −488 | the modern defaults, the RFC 6298 and RFC 5681 corrections, the tests under the new defaults |
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

