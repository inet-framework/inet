# Review a pull request

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rule/pull-request.md](../rule/pull-request.md), [rule/classification.md](../rule/classification.md), [review-a-code-change.md](review-a-code-change.md), [audit/README.md](../audit/README.md)

How to audit a branch against the `PR-*` and `CR-*` rules and write the two reports. The rules are
[rule/pull-request.md](../rule/pull-request.md), which says what a *change* must look like, and
[rule/classification.md](../rule/classification.md), which says what a commit *is*. Worked examples
are in [audit/report/pull-request/](../audit/report/pull-request/pr-1144.md), and
[classification-on-tcp-new.md](../audit/report/sweep/classification-on-tcp-new.md) applies the
classification to a 61-commit branch.

**The commit is the unit of the `PR-*` audit, not the pull request.** Read the series one commit at a
time so each commit can be judged as one change. Review each commit's code correctness with
[review-a-code-change.md](review-a-code-change.md), then review the integrated branch contract so a
correct commit series cannot hide a cross-commit regression.

## 1. Get the branch and the merge base

```bash
git fetch origin refs/pull/<n>/head:refs/pr/<n>
MB=$(git merge-base refs/pr/<n> origin/master)
git log --oneline $MB..refs/pr/<n>
git diff --stat $MB..refs/pr/<n>
```

Record the head commit and the merge base in the report. A review of "the branch" with no commit is
not repeatable.

## 2. Run the mechanical checks

```bash
doc/project/enforcement/check-commits.sh $MB..refs/pr/<n>
```

What it covers, and what to run by hand when you want the detail:

```bash
# PR-SPLIT-WHITESPACE — a file whose diff is empty when whitespace and blank lines are ignored
git show -w --ignore-blank-lines --numstat --pretty="" <commit> -- <file>

# PR-SPLIT-MOVE — a rename next to a content change
git show --name-status -M -C <commit> | grep -E "^[RC]"

# PR-SPLIT-BASELINE — which commits change source, and which change only baselines
git log --format="%h %s" --name-only $MB..refs/pr/<n>
# the source change of a commit that also moves baselines
git show <commit> -- . ':!tests'

# PR-SERIES-LINEAR / PR-SERIES-ORDER
git log --merges $MB..refs/pr/<n>
git log --format=%s $MB..refs/pr/<n>
```

**The whitespace check needs `--ignore-blank-lines` as well as `-w`.** With `-w` alone a branch that
removes blank lines instead of changing indentation reports nothing. That is a real finding from
[pr-1144](../audit/report/pull-request/pr-1144.md), and it is the reason the flag is written here.

## 3. Audit the commit messages

```bash
doc/project/enforcement/check-classification.sh $MB..refs/pr/<n>
```

It checks the mechanical half of [rule/classification.md](../rule/classification.md) and it prints
the breakdown that step 4 needs. What each check means:

| Check | What a violation tells you |
| --- | --- |
| `CR-TAG-TRAILER` | the commit has no `Change:` line, so it is unclassified |
| `CR-TAG-FORM` | the trailer has the wrong field count or a word outside the vocabulary |
| `CR-TAG-SUBJECT` | the subject omits the kind, or a prefix disagrees with the trailer |
| `CR-SCOPE-AREA` | the claimed area is not among the paths the commit touches |
| `CR-DEPTH-ONE` | the commit claims two depth levels, so it holds two changes |
| `CR-OBL-INERT` | the commit claims no behavior change and moves a recorded expectation |

**The last two are the ones that find real defects.** `CR-DEPTH-ONE` is
[PR-SPLIT-MECHANICAL](../rule/pull-request.md#pr-split-mechanical) seen from the other side, and
`CR-OBL-INERT` is the only check in the rule set that a test suite can settle.

**A trailer is a claim, and the gate checks only the half that is derivable.** Three things stay
for you to judge:

- **Is the depth honest?** A commit that says `refactor` and rewrites a loop condition is a
  behavior change wearing the wrong word. Read the diff of every `refactor`, `name` and `comment`
  commit — there are few, and the gate has already listed them.
- **Is the obligation discharged, and where?** The two obligations have **different units**, and
  reading them the same way produces a false finding.
  [TR-SHIP-WITH](../rule/testing.md#tr-ship-with) asks for the test in the same *pull request*, so
  a batch of tests at the end of a series satisfies the letter of it.
  [PR-SPLIT-BASELINE](../rule/pull-request.md#pr-split-baseline) asks for the baseline in the same
  *commit*, so a batch at the end is a violation. A long gap between behavior and test is still
  worth a **note**: the rule's own reason is that a test written later is written against the code
  as it turned out.
- **Is a `?` still a `?`** A commit that says `?` for its baselines is honest and unfinished. It
  is a `PARTIAL`, not a `PASS`, and the report says what run would settle it.

**Do not report a missing trailer on a commit that predates the rule.** Check the date of the
branch against the date `classification.md` entered the tree.

## 4. Read the breakdown

The gate prints it, and `commit_breakdown.py` produces it alone from any commit range:

```bash
git log --reverse --format='%s%x09%(trailers:key=Change,valueonly)' $MB..refs/pr/<n> \
  | python3 doc/project/enforcement/commit_breakdown.py
```

It emits **by depth**, **by direction and intent**, **area against depth** for a series above ten
commits, **by group**, and **the obligations the series owes**. Read it for the four patterns that
a diff does not show:

| Pattern | What it means |
| --- | --- |
| a large `behavior` count and a small **obligations** count | the series adds behavior and owes tests it has not written |
| a row in **by group** reading `(none)` | commits with no group in a series where the rest have one — a [PR-SPLIT-DRIVEBY](../rule/pull-request.md#pr-split-driveby) question |
| a mixed depth such as `name+refactor` | one commit holding two changes |
| an area in **area against depth** that the topic does not explain | the branch reaches further than its title says |

**The count of commits below `behavior` is the reviewer's budget.** Those commits need no
behavioral review, so the line tells you how much of the series you can read quickly.

This breakdown belongs in the **summary**, not in the audit report: the summary says what the
change is, and the audit judges it. Paste the gate's breakdown into
`audit/report/pull-request/pr-<n>-summary.md` under a heading `## The commits`, above the interface
sections. `opp_summarize_changes` does not generate it yet.

## 5. Judge what a script cannot

Read each commit against the rules that need judgment:

- **[PR-SPLIT-ONE-CHANGE](../rule/pull-request.md#pr-split-one-change)** — does the subject need an
  "and"? Then the commit holds two decisions.
- **[PR-SPLIT-UPSTREAM](../rule/pull-request.md#pr-split-upstream)** — does a commit change a shared
  component to serve one protocol? Try to describe the shared change without naming that protocol; if
  you cannot, the feature is in the wrong place or it is too narrow.
- **[PR-SPLIT-PREPARE](../rule/pull-request.md#pr-split-prepare)** — does a commit called a refactor
  change behavior?
- **[PR-SPLIT-DRIVEBY](../rule/pull-request.md#pr-split-driveby)** — is a hunk unrelated to the
  subject line?
- **[PR-SPLIT-BASELINE](../rule/pull-request.md#pr-split-baseline)** — the regenerated values belong
  in the commit that moves them, and that commit's message must say which behavior moved and why the
  new values are right. A baseline commit that stands alone after a source commit is one change
  divided in two: ask for a squash. A baseline commit that no source commit causes — a compiler or
  solver version change — is correct as it stands, and its own message carries the reason.
- **[PR-MSG-BODY](../rule/pull-request.md#pr-msg-body)** — run the gate, then read the bodies the
  gate cannot judge. Count them: a series where most commits explain themselves and a few do not is
  a different thing from one where the habit is absent. `git log --format='%h %s' --no-walk
  $(git log --format=%H $MB..HEAD | while read c; do [ -z "$(git log -1 --format=%b $c | grep -v '^$')" ] && echo $c; done)`
  lists every commit with no body at all.
- **[PR-MSG-WHY](../rule/pull-request.md#pr-msg-why)** — does the body give the reason, or repeat the
  diff?
- **[PR-MSG-REPRODUCE](../rule/pull-request.md#pr-msg-reproduce)** — every commit the breakdown
  counts under `fix` must say how to see the defect happen. Steps are enough for most; ask for a
  regression test only where the defect sits on a crossed path, could return under a refactor, or
  came from a misread standard. `git log --format='%h %s' --grep='^Change:.*\.fix' $MB..HEAD` lists
  the candidates once the branch carries trailers.
- **[PR-MSG-PLAN](../rule/pull-request.md#pr-msg-plan)** — if the series follows a plan, does each
  commit name it, and does the path still exist?
- **[TR-BASELINE-PROVENANCE](../rule/testing.md#tr-baseline-provenance)** — **every moved row is
  accounted for**, not the set as a whole. Rows that share one explanation are named together; a
  count is not an explanation. An unexplained row is an unintended change until somebody shows
  otherwise.
- **[AR-EXT-MINIMAL-SURFACE](../rule/architecture.md#ar-ext-minimal-surface)** and
  **[AR-EXT-VIRTUAL-IS-A-PROMISE](../rule/architecture.md#ar-ext-virtual-is-a-promise)** — generate
  the summary with `--usage`; its *Questions for the review* section lists every new public function
  that nothing calls or only a test calls, and every new virtual that nothing overrides. Each is a
  question to the author, not a finding: *who is this for?* and *what would an override do?*

## 6. Review each commit's correctness

For every commit, apply [review-a-code-change.md](review-a-code-change.md) to that exact diff. Name
the changed contracts, trace their callers and terminal paths, and run the canonical tier-4
checklists it selects. This is also the code-correctness evidence required by
[PR-REQ-ARCH](../rule/pull-request.md#pr-req-arch); the `PR-*` structure audit does not substitute for
it.

If the branch touches a sealed path, the permission for it must be stated. That statement is review
evidence; merge authorization comes from the trusted, head-bound decision in
[SR-PR-APPROVAL](../rule/sealing.md#sr-pr-approval).

## 7. Review the integrated branch contract

After the commit-by-commit pass, review the merge-base-to-head change as one integrated code change
under [review-a-code-change.md](review-a-code-change.md). Recheck interfaces and their final
implementations, generated inputs and consumers, feature-off and effective configurations, semantic
siblings, terminal paths, tests, and baselines in the final tree. This pass catches contracts that
are locally valid in separate commits but inconsistent when composed.

## 8. Write the report

**Two files.** `audit/report/pull-request/pr-<n>.md` judges the commits; `pr-<n>-summary.md` states
what the change does and carries the breakdown from step 4.

In the audit, one row per rule with a verdict — `PASS`, `FLAG`, `PARTIAL` or `not verified` — and
evidence for each. The `CR-*` rows go beside the `PR-*` rows, because the two rule sets check the
same commits: `CR-DEPTH-ONE` beside `PR-SPLIT-MECHANICAL`, `CR-OBL-INERT` beside
`PR-SPLIT-BASELINE`, `CR-TAG-SUBJECT` beside `PR-MSG-SUBJECT`. Then one numbered finding per problem, with the commit, what
it breaks, and what would repair it.

`PARTIAL` is limited to this composite `PR-*` evaluation: use it only when independently checkable
parts of one pull-request rule have different outcomes. It is never a tier-4 checklist verdict and
never substitutes for a correctness finding, checklist `FLAG`, checklist `QUESTION`, or `not
verified` result. Present correctness findings and checklist output in the order defined by
[review-a-code-change.md](review-a-code-change.md), with the commit or integrated branch scope made
explicit.

**Say what you did not check.** `PR-SERIES-BUILDS` needs a per-commit CI build, and a review that
does not run one records `not verified` rather than `PASS`. The same holds for a branch that moves
fingerprints: only a run of the suite at each commit shows that the recorded values match the source
beside them. A report that hides its gaps is worse than a shorter one.
