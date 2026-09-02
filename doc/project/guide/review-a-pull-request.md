# Review a pull request

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rule/pull-request.md](../rule/pull-request.md), [review-a-code-change.md](review-a-code-change.md), [audit/README.md](../audit/README.md)

How to audit a branch against the `PR-*` rules and write the report. The rules are
[rule/pull-request.md](../rule/pull-request.md); three worked examples are in
[audit/report/pull-request/](../audit/report/pull-request/pr-1144.md).

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

## 3. Judge what a script cannot

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
- **[PR-MSG-WHY](../rule/pull-request.md#pr-msg-why)** — does the body give the reason, or repeat the
  diff?

## 4. Review each commit's correctness

For every commit, apply [review-a-code-change.md](review-a-code-change.md) to that exact diff. Name
the changed contracts, trace their callers and terminal paths, and run the canonical tier-4
checklists it selects. This is also the code-correctness evidence required by
[PR-REQ-ARCH](../rule/pull-request.md#pr-req-arch); the `PR-*` structure audit does not substitute for
it.

If the branch touches a sealed path, the permission for it must be stated. That statement is review
evidence; merge authorization comes from the trusted, head-bound decision in
[SR-PR-APPROVAL](../rule/sealing.md#sr-pr-approval).

## 5. Review the integrated branch contract

After the commit-by-commit pass, review the merge-base-to-head change as one integrated code change
under [review-a-code-change.md](review-a-code-change.md). Recheck interfaces and their final
implementations, generated inputs and consumers, feature-off and effective configurations, semantic
siblings, terminal paths, tests, and baselines in the final tree. This pass catches contracts that
are locally valid in separate commits but inconsistent when composed.

## 6. Write the report

`audit/report/pull-request/pr-<n>.md`. One row per rule with a verdict — `PASS`, `FLAG`, `PARTIAL` or
`not verified` — and evidence for each. Then one numbered finding per problem, with the commit, what
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
