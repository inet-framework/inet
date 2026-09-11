# Resolve the audit findings of PR #1155

Status: **ready** — both decisions made 2026-09-11, no step started.
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

Two rules written on 2026-09-11 also apply to this branch, and neither was in the audit because
neither existed when it was written:

| Rule | What it asks of this branch |
| --- | --- |
| [PR-MSG-REPRODUCE](../../doc/project/rule/pull-request.md#pr-msg-reproduce) | the 14 `fix` commits say how to see the defect — step 1f |
| [PR-MSG-PLAN](../../doc/project/rule/pull-request.md#pr-msg-plan) | every commit this plan touches names this plan — step 1g |

[TR-BASELINE-PROVENANCE](../../doc/project/rule/testing.md#tr-baseline-provenance) was strengthened
the same day, and that is D-2 below.

## 2. The two decisions, both now made

### D-1 — `common/packet/`: unseal, edit, reseal — **decided 2026-09-11**

The seal owner validated the three edits and allows the path to be unsealed for them and resealed
after. That unblocks F-1 and it does not resolve the underlying question: `AV-ORG-01` and
`AV-ORG-02` remain `Open (decide)`, and the reseal puts the seal back over two clusters the ledger
still calls invalid. **The reseal is on the same footing it had before; this decision is about the
branch, not about the seal.**

The three files are `chunk/BitCountChunk.cc`, `chunk/ByteCountChunk.cc` and
`recorder/PcapRecorder.cc`. Step 4 carries out the unseal and the reseal.

### D-2 — Attribute every moved row — **decided 2026-09-11, the hard way**

Every commit that moves a fingerprint carries the new values, and **every moved row is analyzed and
explained** from the change that causes it and from what that fingerprint records.

This is the rule, not a preference for this branch:
[TR-BASELINE-PROVENANCE](../../doc/project/rule/testing.md#tr-baseline-provenance) was strengthened
on 2026-09-11 to say it. **A row that moves and cannot be explained from the diff is the finding**
— an unexplained row is an unintended change until somebody shows otherwise, because a fingerprint
says only that the trajectory differs and never which of the two is right. Rows that share one
explanation are named together; a count is not an explanation.

Step 5 is therefore the long step, and the plan says so rather than hiding it.

## 3. The order, and why

```
  step 1  rewrite history   (F-4, F-5, F-7, the F-2 alias, the trailers,
                             9 of the 14 reproductions, the Plan: lines)
  step 2  rebase onto master (F-6)
  step 3  the release note   (F-2)
  step 4  unseal, edit, reseal (F-1)
  step 5  the baselines, every row explained (F-3)   <- the long step
  step 5b the five reproductions that step 5 supplies (F-3 -> PR-MSG-REPRODUCE)
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

**Five reproductions wait for step 5.** Commits 44, 45, 49, 50 and 51 each move fingerprint rows,
and the row analysis step 5 must do anyway *is* the reproduction those commits owe. Writing them
before step 5 means writing them twice.

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

**1f. Add the reproduction to fourteen fix commits
([PR-MSG-REPRODUCE](../../doc/project/rule/pull-request.md#pr-msg-reproduce)).**

The rule is new, written 2026-09-11. Fourteen commits in this series carry `.fix`, **all fourteen
already have a body, and the bodies are among the best in the project** — `f2b5fd13b7` traces an
unsigned subtraction wrapping to ~4G through to a stalled recovery, and names what it deliberately
leaves for a follow-up. **None of them says how to see the defect happen.** They explain why the
code was wrong from reading the code, not which configuration shows it.

| # | Commit | What must be added |
| --- | --- | --- |
| 1 | `ed742203a8` | which chunk split loses the fill byte, and what the receiver then sees |
| 3 | `21d34de9e8` | which tool reads the trace wrongly under `LINKTYPE_PPP_WITH_DIR` |
| 7 | `747ec0f435` | the scenario that disconnects mid-transmission, and the assertion that fires |
| 38 | `06853ca06f` | what the oracle ran, and what each of the three defects looked like |
| 39 | `c0ec1b6a94` | the option form and the pre-ACK window case that fail |
| 40 | `e04d92d9c3` | the RTT profile where the old detector exits slow start at the wrong point |
| 44 | `4190b1977e` | the loss pattern that splits a SACK region **— see step 5** |
| 45 | `f2b5fd13b7` | the window and pipe values that wrap **— see step 5** |
| 46 | `429e8cc9be` | the option layout that aborted the simulation, and the error text |
| 48 | `8af4179c43` | the parameter combination that compared against the wrong default |
| 49 | `37d121af2e` | the ECN scenario that got no reaction **— see step 5** |
| 50 | `5d94c07e75` | the flavour and loss pattern whose duplicate ACKs went uncounted **— see step 5** |
| 51 | `7d52875f17` | the loss episode after which cwnd stayed high **— see step 5** |
| 53 | `0776e1591c` | the half-close sequence that dropped the connection |

**Five of the fourteen get their reproduction from step 5 for free.** Commits 44, 45, 49, 50 and 51
each move fingerprint rows, and a row that moves *is* a scenario where the defect showed. Step 5
has to identify and explain those rows anyway, so run step 1f for those five **after** step 5 and
lift the configuration from the row analysis.

**None of the fourteen needs a standalone regression test on its face**, and the rule says the test
is justified only where the defect sits on a crossed path, could return under a refactor, or came
from a misread standard. Two are worth a second look against that test: `f2b5fd13b7` is an unsigned
underflow in RFC 6675 `nextSeg`, which a future edit to `setPipe` could reintroduce; and
`7d52875f17` is a missing `state->lossRecovery` that any rework of the recovery split could drop
again. **The author decides; the plan only marks them.**

**Done when** each of the fourteen bodies names a configuration and an observable.

**1g. Name this plan in every commit
([PR-MSG-PLAN](../../doc/project/rule/pull-request.md#pr-msg-plan)).**

Every commit this plan produces or rewrites carries, above its `Change:` trailer:

```
Plan: plan/pending/pr-1155-resolve-audit-findings.md
```

That is what lets a later reader find out why commit 15 became two commits, why the baselines are
spread across eleven commits instead of two, and who decided the seal. None of that fits in a
commit body and all of it is here.

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

### Step 4 — Unseal, then reseal (F-1)

**D-1 is decided**, so this step is mechanical.

1. Unseal `common/packet/` in
   [audit/seal-list.md](../../doc/project/audit/seal-list.md), with the reason: the three edits
   were validated by the seal owner on 2026-09-11.
2. The three edits ride in their own commits, where they already are — `ed742203a8` and
   `21d34de9e8`.
3. Reseal the path, against the same audit it rested on before.
4. The pull request description names the three files and the validation, under
   [SR-PR-APPROVAL](../../doc/project/rule/sealing.md#sr-pr-approval).

**Say what the reseal does not fix.** `AV-ORG-01` and `AV-ORG-02` are still `Open (decide)`, so the
reseal restores a seal the ledger calls invalid. That is the state this branch found and the state
it leaves; the branch is not the place to repair it.

**Done when** `check-source-seals.sh --base origin/master` passes and the seal list records both
the unseal and the reseal.

### Step 5 — The baselines, every row explained (F-3)

**D-2 is decided: the hard way.** Every commit that moves a fingerprint carries the new values, and
every moved row is explained.

Eleven commits are in scope — nine that own a `fingerprint` and two that own a `?` and must be
settled either way:

| # | Commit | What it changes | Expect |
| --- | --- | --- | --- |
| 1 | `ed742203a8` | the fill byte of a split count chunk | `?` — settle it |
| 2 | `a705db8a75` | PPP for RFC 1661, `PppTrailer` removed | every row with a PPP link |
| 7 | `747ec0f435` | byte-align a truncated packet | `?` — settle it |
| 33 | `e24da2c9d4` | the modernized defaults | **the large one** — most of the 53 |
| 40 | `e04d92d9c3` | the CUBIC HyStart delay detector | rows that run CUBIC |
| 44 | `4190b1977e` | the lost mark across a SACK queue split | rows with SACK loss |
| 45 | `f2b5fd13b7` | the RFC 6675 `nextSeg` rule (2) underflow | rows with SACK loss |
| 49 | `37d121af2e` | the classic RFC 3168 ECN reaction | rows with ECN |
| 50 | `5d94c07e75` | duplicate ACK counting for every flavour | rows with loss |
| 51 | `7d52875f17` | non-SACK Reno cwnd deflate | rows with non-SACK Reno |
| 52 | `fea41a027a` | `TcpCubic` on `TcpClassicAlgorithmBase` | rows that run CUBIC |

**The method, per commit**, once the branch sits on the new master:

1. Run the fingerprint suite at that commit.
2. Take the rows that move, and only those. A row that moves here and was already moved by an
   earlier commit in the series belongs to the earlier one.
3. **Explain each row from the diff.** Which behavior in this commit changes the trajectory this
   row records. Group the rows that share an explanation — *"the 31 rows under `examples/inet/`
   all carry TCP traffic with the default algorithm, which this commit changes from `TcpReno` to
   `TcpCubic`"* is one explanation for 31 rows and it is complete.
4. **A row you cannot explain stops the step.** It is an unintended change until shown otherwise,
   and finding one is this step earning its cost.
5. Put the rows and the explanation in that commit.

**The expectation column above is a prediction, not a result.** A row that moves where the column
says it should not is the interesting case, and it is the reason to run all eleven rather than
assume the defaults commit owns everything.

**Reuse the text of `33e8b0d073`.** It is the best baseline provenance this audit has seen — the
tip built in a separate workspace, the suite run there, 53 rows named by family, and a record of
what it deliberately did not touch. That text becomes the explanation of commit 33 and the source
of the per-commit wording for the rest.

**Done when** every commit that moves a recorded expectation carries it with a row-level
explanation, `check-commits.sh` reports no `PR-SPLIT-BASELINE` violation, and
`check-classification.sh` reports no `CR-OBL-INERT` violation.

### Step 5b — The five reproductions that step 5 supplies

Commits 44, 45, 49, 50 and 51 owe a reproduction under
[PR-MSG-REPRODUCE](../../doc/project/rule/pull-request.md#pr-msg-reproduce) and each moves
fingerprint rows. Step 5 identifies and explains those rows, and a row that moves is a scenario in
which the defect showed. Lift the configuration from the row analysis into the body.

**This is the one place where two obligations pay for each other**, and it is why the plan
separates these five from the nine in step 1f.

**Done when** each of the five names the configuration its own moved rows identify.

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

By hand, because no gate reaches them:

- every `.fix` commit names a configuration and an observable
  ([PR-MSG-REPRODUCE](../../doc/project/rule/pull-request.md#pr-msg-reproduce));
- every moved fingerprint row has an explanation, grouped where the explanation is shared
  ([TR-BASELINE-PROVENANCE](../../doc/project/rule/testing.md#tr-baseline-provenance));
- every commit carries `Plan: plan/pending/pr-1155-resolve-audit-findings.md`
  ([PR-MSG-PLAN](../../doc/project/rule/pull-request.md#pr-msg-plan)).

Then a fourth audit pass, which should turn F-1 to F-7 into `PASS`. **Move this file to
`plan/done/` when it does**, and the `Plan:` lines still resolve, because the path in a commit
message is read against the tree at that commit.

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
| 1f | medium | nine reproductions now, five after step 5; the bodies exist and none names a configuration |
| 1g | small | one line per commit, added in the same pass |
| 2 | medium | two real source conflicts in `PppHeaderSerializer`, plus a build in both modes |
| 3 | medium | the generated *Breaking* list makes it writing rather than discovery |
| 4 | small | **unblocked** — the seal owner validated the three edits |
| 5 | **large** | eleven fingerprint runs, and then a row-level explanation for each; the suite is the clock and the analysis is the work |
| F-8 | small | two changes in `opp_repl` |

**Nothing gates the start any more.** Both decisions are made, and steps 1, 2 and 3 can begin
immediately. Step 5 is the clock: eleven fingerprint runs and eleven explanations.

## 7. Risks

| Risk | What happens | What reduces it |
| --- | --- | --- |
| the rebase loses the rename detection | the `git blame` history of three central TCP files ends at this branch | step 1a, done before step 2 |
| master advances again while this runs | step 2 and step 5 both go stale | do steps 2 to 5 in one stretch; the branch has already waited 99 commits |
| the twelve bodies get written as restatements | F-7 closes on paper and the history stays uninformative | the question table in step 1d; a body that answers none of it has not closed anything |
| a moved row gets explained by assertion | *"the defaults changed"* covers 53 rows and proves nothing; an unintended change rides through | step 5 point 4 — a row you cannot trace from the diff **stops the step** |
| the eleven runs find rows nobody predicted | step 5 grows, and the branch may hold a defect | that is the step working; the expectation column is a prediction and a miss is the finding |
| the reproductions get written from the code | a body that re-derives the bug from the diff is the body that is already there; the rule asks for a configuration and an observable | step 1f names what is missing per commit; five come from step 5's rows, which are evidence and not argument |
| the summary is regenerated without the parsers | the report states 210 where the truth is 547 | the `PATH` export in step 6, until F-8 lands |

## 8. Out of scope

- **The `T3` release-note check** that F-2's five occurrences argue for. It is a project change, not
  a branch change, and it needs its own plan.
- **`AR-EXT-MINIMAL-SURFACE` and `AR-EXT-VIRTUAL-IS-A-PROMISE`** — 28 uncalled and 95 unoverridden
  at the head. The audit records these as *questions to the author*, not findings, and answering
  them is review conversation rather than plan work.
- **The two unsanctioned `AV-ORG` clusters themselves.** D-1 settles what this branch does — unseal,
  edit, reseal — and leaves the clusters `Open (decide)`. Repairing them is the seal owner's work
  and it needs its own plan.
