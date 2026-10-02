# Land the TCP modernization on master in small stages

Status: **in progress** — S1 and S2 landed on master on 2026-10-02, S3 on 2026-10-05. S4 is next.

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
| S4 | `topic/tcp-recovery-split` | 7, 8, 9, 10, 50 | +1872 −656 | the recovery interfaces, RFC 5681, RFC 6582, SACK recovery moved into `Rfc6675Recovery` |
| S5 | `topic/tcp-flavour-split` | 11, 12, 13, 54, 55, 56, 60, 84 | +1101 −1312 | the classic flavours on the split architecture, and the repairs of that move |
| S6 | `topic/tcp-cubic` | 14, 45, 57, 62, 65 | +840 −326 | `TcpCubic` with HyStart, and `DcTcp` on the shared ACK path |
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

## 4. The procedure of one stage

1. Make the branch and its worktree from `master`: branch `topic/tcp-<name>`, worktree
   `/home/levy/workspace/inet-tcp-<name>`.
2. Cherry-pick the commits of the stage in the order of the table. Take the test files of the
   stage out of commit 41. Apply rules 4 to 6.
3. Compare: each file of the stage must be equal to its version on the source branch at the
   commit where the stage ends, or the difference must have a reason in this plan.
4. Run the gates: `check-classification.sh`, `check-commits.sh`, `check-links.sh`,
   `check-seals.sh`, `check-source-seals.sh`, and `check-series-builds.sh master..HEAD debug`.
5. Build in debug against `omnetpp-6.x` (the pairing of `inet-master`) and run the tests of the
   stage: its unit and module tests, the module `tcp_` suite, the protocol tests `tcp/`, and
   the fingerprint ingredients `tplx` and `~tND` at the stage head. If a row moves, find the
   commit that moves it and explain the row in that commit (rule 3).

   In this environment, many stored fingerprints do not match on master too (for example DHCP,
   AODV and BGP rows). So a stage compares the calculated values of its head with the calculated
   values of master in the same environment, not with the stored values. The run at the head of
   a stage is the baseline of the next stage. The script is
   `/var/tmp/claude-staged/fp.sh` (it keeps every calculated value in its log).
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

