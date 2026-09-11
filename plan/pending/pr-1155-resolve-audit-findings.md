# Resolve the audit findings of PR #1155

Status: **pending** — no step started.
Audit: [doc/project/audit/report/pull-request/pr-1155.md](../../doc/project/audit/report/pull-request/pr-1155.md), third pass, 2026-09-11.
Branch: `topic/tcp-new`, head `33e8b0d073`, merge base `434658d729`, 61 commits, 183 files, +15840 / −3685.

## 1. What this plan resolves

Eight findings: **F-1, F-2, F-6** blocking, **F-3, F-4, F-5, F-7** findings, **F-8** blocking for
the audit tooling rather than for the branch.

| | What it is | Where the work is |
| --- | --- | --- |
| F-1 | the branch edits sealed `common/packet/` and states no permission | a decision, then one line in the description |
| F-2 | no release note for four kinds of break | `WHATSNEW`, the migration guide, one alias |
| F-3 | two baseline-only commits, and 9 commits owe a fingerprint | the expensive step |
| F-4 | a rename of three files inside a 25-file edit | split one commit |
| F-5 | `ITcpRecovery` holds five no-op defaults | 5 declarations, 3 implementors |
| F-6 | the branch no longer merges | rebase over 99 commits, 3 conflicting paths |
| F-7 | 17 commits have no body, 12 of them above 50 lines | the author writes 12 bodies |
| F-8 | the summary tool fails silently without its parsers | `opp_repl`, not this repository |

## 2. Two decisions come before any work

**Neither is the branch author's to make, and both block a step.**

### D-1 — Decide the `common/packet/` seal (blocks F-1)

`common/packet/` is sealed over `AV-ORG-01` and `AV-ORG-02`, both still `Open (decide)`, which
[SR-AUDIT-FIRST](../../doc/project/rule/sealing.md#sr-audit-first) forbids. The branch needs three
lines in that tree. Three outcomes, and any of them unblocks:

1. **Sanction the two clusters** as `AS-ORG-*`, and the seal stands. The branch then states the
   permission under [SR-PR-APPROVAL](../../doc/project/rule/sealing.md#sr-pr-approval).
2. **Repair the two clusters**, then the seal stands on its own.
3. **Lift the seal** until the clusters are decided.

Blocking a three-line chunk repair behind a seal the project's own ledger calls invalid is the
worst of the three, and it is where things stand today.

### D-2 — Decide how far the baseline attribution goes (sizes F-3)

**This decision dominates the cost of the whole plan.** The reconstruction in
[classification-on-tcp-new.md](../../doc/project/audit/report/sweep/classification-on-tcp-new.md)
says **9 commits owe a `fingerprint`** and 2 more owe a `?`, while the branch discharges all of
them in **2 bulk commits**.

| Option | What it costs | What it buys |
| --- | --- | --- |
| **A. Squash each bulk commit into the one before it** — the audit's own wording | two rebase edits | the letter of [PR-SPLIT-BASELINE](../../doc/project/rule/pull-request.md#pr-split-baseline) for `9f40ee337e`, whose cause really is the commit before it |
| **B. Attribute every row to the commit that moves it** | a fingerprint run at each of 9 to 11 commits | [PR-SERIES-BUILDS](../../doc/project/rule/pull-request.md#pr-series-builds): every commit passes its own tests, and `git bisect` over the suite works |

**Option A is not sufficient for the second commit.** `33e8b0d073` says it re-records "the
fingerprints this TCP workstream moves" — a span, not the commit before it — so squashing it into
`f121cec387` records a cause that is false. Under option A that commit must keep its own message
and stay where it is, and the report keeps F-3 open as a stated exception.

**Recommendation: B for the nine, A for nothing.** The branch already proves the author can run the
suite in a separate workspace and describe what moved; option B is that work done nine times
instead of twice. If the effort is refused, take A and say so in the pull request description, so
the exception is a decision and not an omission.

## 3. The order, and why

```
  step 1  rewrite history   (F-4, F-5, F-7, the F-2 alias, the trailers)
  step 2  rebase onto master (F-6)
  step 3  the release note   (F-2)
  step 4  state the seal     (F-1, needs D-1)
  step 5  the baselines      (F-3, needs D-2)   <- the expensive step
  step 6  verify
```

Three constraints fix this order.

**Split the rename before the rebase.** Git detects the three renames in `ed2c8df034` at **53 %,
56 % and 64 %** similarity. A rebase replays that commit over 99 commits of master, and a few more
changed lines take it under the detection threshold, which ends the `git blame` history of three
central TCP files. Splitting first gives a pure-rename commit that rebases with no detection at
all.

**The baselines come after the rebase.** `tests/fingerprint/showcases.csv` is one of the three
conflicting paths, so master has re-recorded rows the branch also re-records. Every value measured
before the rebase is stale.

**One history pass, not three.** F-4, F-5 and F-7 all edit the same series. Doing them together
costs one rebase; doing them apart costs three, and each one risks the rename detection again.

## 4. The steps

### Step 1 — One history pass

Work in a dedicated worktree, on a copy of the branch, so the original stays reachable.

**1a. Split `ed2c8df034` (F-4).** Two commits: a rename-only commit, then the content rewrite.

```
tcp: name: rename TcpBaseAlg and TcpTahoeRenoFamily for the split architecture
    Change: src.tcp.flavours | name | whatsnew migration | tcp-algorithm-split
tcp: refactor: move the classic flavours onto the split architecture
    Change: src.tcp.flavours | refactor | - | tcp-algorithm-split
```

The rename commit uses `git mv` and nothing else, so the similarity is 100 %.
**Done when** `git show --name-status -M` on the first commit shows only `R100` lines, and
`check-commits.sh` no longer flags `PR-SPLIT-MOVE`.

**1b. Add the deprecation alias (F-2, part).** In the rename commit, one line per renamed type:

```cpp
using TcpBaseAlg = TcpAlgorithmBase;            // deprecated, remove after the next release
using TcpTahoeRenoFamily = TcpClassicAlgorithmBase;
```

That satisfies [RR-DEPRECATE-FIRST](../../doc/project/rule/release.md#rr-deprecate-first) and keeps
every out-of-tree TCP algorithm compiling for one release.
**Done when** a translation unit that names `TcpBaseAlg` still compiles against the branch.

**1c. Make `ITcpRecovery` pure (F-5).** Amend `96b310f5f3`, the commit that introduces the
interface — not a fix-up at the end, which
[PR-SERIES-ORDER](../../doc/project/rule/pull-request.md#pr-series-order) forbids.

Remove the five `{}` bodies from `segmentsAcked`, `dataSent`, `segmentRetransmitted`,
`onRexmitTimeout` and `reoTimeout`, and add an empty override in each of `Rfc5681Recovery`,
`Rfc6582Recovery` and `Rfc6675Recovery` where the behavior really is nothing. Each override then
states its own answer, and a later rename of any of the five breaks an out-of-tree implementor
loudly instead of silently dropping its override.

This is a **new interface with no out-of-tree implementor yet**, so this is the cheapest moment it
will ever have.
**Done when** `check-interfaces.sh --verbose` reports 16 at the head, down from 17 — the remaining
one is `IIeee80211Band`, which master repaired after this branch's base and which this branch does
not touch.

**1d. Write twelve bodies (F-7).** Only the author can do this: the reasons exist and are not in
the tree. The subjects are good, and that is the trap — *"undo a reduction that turned out to be
unnecessary"* reads like a reason and is a restatement of what the code does.

Each body answers the question the audit already asked of it:

| Commit | The question its body must answer |
| --- | --- |
| `81713e23d9` | which option combinations overflowed the segment, and what the old size did wrong |
| `6d3b0fdcdb` | what RFC 8985 detects that the previous loss test missed |
| `710b18208c` | what of RFC 7413 is implemented, what is left out, which options INET negotiates |
| `2fec57756c` | which features share this plumbing, and why it is shared rather than repeated |
| `7327e5c5d2` | why AccECN and not classic ECN, and what a non-AccECN peer sees |
| `eb549a12a5` | under which condition the reduction was unnecessary, and what happens if the undo fires wrongly |
| `4b46417a58` | which timeout, and why a probe beats waiting it out |
| `1a9200e2a3` | what the advertised window used to be tied to, and what breaks when it is not the buffer |
| `3616a5a5ca` | who asked for keepalive, and what the defaults are for |
| `a4317c0d3e` | which socket options, and which behavior each one reaches |
| `838e6d254f` | how the degree is learned, and what bounds it |
| `8bbf0d575b` | which three timings, and what each one was before |

Five more commits have no body and are under the gate's 50-line threshold — `ffefa0d484`,
`488b84a1e3`, `125b855d72`, `b479ccc2f8`, `d8a21d0616`. They are the reviewer's to judge.
**Done when** `check-commits.sh` reports no `PR-MSG-BODY` violation.

**1e. Add the `Change:` trailers.** All 61, from the reconstruction in
[classification-on-tcp-new.md](../../doc/project/audit/report/sweep/classification-on-tcp-new.md),
which already writes them.

This is not tidiness. With the trailers in place,
[CR-OBL-INERT](../../doc/project/rule/classification.md#cr-obl-inert) guards step 5: a commit that
claims no behavior change and carries a baseline row fails the gate. Without them, step 5 has no
mechanical check at all.
**Done when** `check-classification.sh` passes over the series.

### Step 2 — Rebase onto master (F-6)

Three conflicting paths, and each has a different shape:

| Path | The conflict | How to resolve |
| --- | --- | --- |
| `src/inet/linklayer/ppp/PppHeaderSerializer.cc` | the branch rewrites PPP for RFC 1661; master changed the same serializer | read both changes; the branch's RFC 1661 move is the deliberate one, master's change must be re-applied on top of it |
| `src/inet/linklayer/ppp/PppHeaderSerializer.h` | the same | the same |
| `tests/fingerprint/showcases.csv` | both sides re-recorded rows | **do not merge by hand.** Take master's file and let step 5 re-record |

**Done when** `git merge-tree --write-tree` against `origin/master` is clean and the branch builds
in both modes.

### Step 3 — The release note (F-2)

`WHATSNEW` is untouched today and `doc/src/` gains 15 lines, none of which mention a break. Four
kinds of break need naming:

1. **Removed signal and statistic** — `sndNxt`.
2. **Renamed types** — `TcpBaseAlg`, `TcpTahoeRenoFamily`; step 1b's alias makes this a deprecation
   rather than a break, and the note says when the alias goes.
3. **Removed public surface** — 25 functions removed outright, 22 more moved with their renamed
   class, and `PppTrailer` removed. The exact list is the *Breaking* section of
   [pr-1155-summary.md](../../doc/project/audit/report/pull-request/pr-1155-summary.md); do not
   retype it, generate it.
4. **Changed defaults** — `tcpAlgorithmClass` `"TcpReno"` → `"TcpCubic"`; `sackSupport`,
   `timestampSupport`, `delayedAcksEnabled`, `limitedTransmitEnabled` `false` → `true`; `mss` `536`
   → `-1`; `advertisedWindow` `14 * this.mss` → `65535`. **This is what moves the 53 fingerprints**,
   so this entry and step 5 must agree.

A migration guide entry under `doc/src/migration-guide/` carries 2 and 3;
[RR-BREAK-MIGRATE](../../doc/project/rule/release.md#rr-break-migrate) asks for the repair
instructions, not only the announcement.

**Done when** a user who upgrades can find, for each of the four, what to change in their own code
or ini file.

**Note for the project, not for this branch:** this same finding has now appeared in five pull
requests from two authors. Five occurrences say the obligation is not visible where it is incurred,
and the change summary already computes the exact list a `T3` check would need. That check is out
of scope here and belongs in its own plan.

### Step 4 — State the seal permission (F-1)

Needs **D-1**. Once the seal is decided, the pull request description names the three files —
`chunk/BitCountChunk.cc`, `chunk/ByteCountChunk.cc`, `recorder/PcapRecorder.cc` — and the
permission that covers them.
**Done when** `check-source-seals.sh --base origin/master` passes, or the stated permission covers
what it reports.

### Step 5 — The baselines (F-3)

Needs **D-2**, and it is the step that decides how long this plan takes.

Under **option B**: run the fingerprint suite at each of the nine commits that own a `fingerprint`
obligation, and put the rows each one moves into that commit.

| # | Commit | Scope |
| --- | --- | --- |
| 2 | `a705db8a75` | `ppp` — RFC 1661 |
| 33 | `e24da2c9d4` | the modernized defaults — the large one |
| 40 | `e04d92d9c3` | CUBIC HyStart |
| 44 | `4190b1977e` | the SACK queue lost mark |
| 45 | `f2b5fd13b7` | RFC 6675 nextSeg rule (2) |
| 49 | `37d121af2e` | classic RFC 3168 ECN |
| 50 | `5d94c07e75` | duplicate ACK counting |
| 51 | `7d52875f17` | non-SACK Reno cwnd deflate |
| 52 | `fea41a027a` | `TcpCubic` on `TcpClassicAlgorithmBase` |

Commits 1 and 7 carry a `?` and must be settled too: run them and replace the `?` with `-` or with
the rows.

**Keep the message of `33e8b0d073`.** It is the best baseline provenance this audit has seen: the
tip built in a separate workspace, the full suite run there, 53 rows named by family, and a record
of what it deliberately did not touch. Whatever option D-2 picks, that text is reused rather than
rewritten.

**Done when** every commit that moves a recorded expectation carries it, and
`check-commits.sh` reports no `PR-SPLIT-BASELINE` violation.

### Step 6 — Verify

```bash
export PATH=/home/levy/workspace/omnetpp/bin:$PATH        # REQUIRED — F-8
MB=$(git merge-base HEAD origin/master)
doc/project/enforcement/check-commits.sh         $MB..HEAD
doc/project/enforcement/check-classification.sh  $MB..HEAD
doc/project/enforcement/check-interfaces.sh --verbose
doc/project/enforcement/check-source-seals.sh --base origin/master
git merge-tree --write-tree HEAD origin/master
opp_summarize_changes --repo . --pr 1155 --usage -o pr-1155-summary.md
```

Then a fourth audit pass, which should turn F-1 to F-7 into `PASS`.

## 5. F-8 is separate, and it is not in this repository

`opp_summarize_changes` produced **210 added** where the truth is **547**, with an empty stderr and
exit status 0, because `opp_nedtool` and `opp_msgtool` were off the `PATH`. It removed every NED
and message fact — 337 of 547 — while its own preamble claimed those parsers had supplied them.

Two repairs in `opp_repl`:

1. **Fail loudly.** The tool checks that every parser it names is runnable, and stops if one is
   not.
2. **Record the provenance.** The generated preamble names the parser versions that actually ran,
   so a reader sees their absence.

Until both land, every command line that generates a summary carries the `PATH` export, as step 6
does and as
[review-a-pull-request.md](../../doc/project/guide/review-a-pull-request.md) now does.

## 6. Effort, and where it goes

| Step | Size | What dominates |
| --- | --- | --- |
| 1a, 1b, 1c | small | mechanical, an hour each at most |
| 1d | medium | **the author's knowledge**, not typing; nobody else can write these twelve |
| 1e | small | the trailers already exist in the trial report |
| 2 | medium | two real source conflicts in `PppHeaderSerializer`, plus a build in both modes |
| 3 | medium | the generated *Breaking* list makes it writing rather than discovery |
| 4 | **blocked** | needs D-1, and D-1 is not this branch's to decide |
| 5 | **large under option B** | nine fingerprint runs, and the suite is the clock |
| F-8 | small | two changes in `opp_repl` |

**Two things gate everything else: D-1 and D-2.** Steps 1, 2 and 3 can start today.

## 7. Risks

| Risk | What happens | What reduces it |
| --- | --- | --- |
| the rebase loses the rename detection | the `git blame` history of three central TCP files ends at this branch | step 1a, done before step 2 |
| master advances again while this runs | step 2 and step 5 both go stale | do steps 2 to 5 in one stretch; the branch has already waited 99 commits |
| the twelve bodies get written as restatements | F-7 closes on paper and the history stays uninformative | the question table in step 1d; a body that answers none of it has not closed anything |
| step 5 is skipped under option A | `git bisect` over the fingerprint suite still lands on a bulk commit | if A is taken, say so in the description as a decision |
| the summary is regenerated without the parsers | the report states 210 where the truth is 547 | the `PATH` export in step 6, until F-8 lands |

## 8. Out of scope

- **The `T3` release-note check** that F-2's five occurrences argue for. It is a project change, not
  a branch change, and it needs its own plan.
- **`AR-EXT-MINIMAL-SURFACE` and `AR-EXT-VIRTUAL-IS-A-PROMISE`** — 28 uncalled and 95 unoverridden
  at the head. The audit records these as *questions to the author*, not findings, and answering
  them is review conversation rather than plan work.
- **The two unsanctioned `AV-ORG` clusters themselves.** D-1 decides what this branch does about
  them; repairing them is the seal owner's work.
