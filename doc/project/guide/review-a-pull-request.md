# Review a pull request

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rule/pull-request.md](../rule/pull-request.md), [audit/README.md](../audit/README.md)

How to audit a branch against the `PR-*` rules and write the report. The rules are
[rule/pull-request.md](../rule/pull-request.md); three worked examples are in
[audit/report/pull-request/](../audit/report/pull-request/pr-1144.md).

**The commit is the unit of review, not the pull request.** Read the series one commit at a time and
ask one question per commit: *is this change right?* That question has an answer only when the commit
holds one change.

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

# PR-SPLIT-BASELINE — a baseline and a source file in one commit
git show --name-only --pretty="" <commit> | grep -E "^tests/.*\.csv$|^src/"

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
- **[PR-MSG-WHY](../rule/pull-request.md#pr-msg-why)** — does the body give the reason, or repeat the
  diff?

## 4. Check what the change touches

Beyond the `PR-*` rules, name the architectural surface: the contracts, the packet content, the
configuration, the feature descriptors, and the tests
([PR-REQ-ARCH](../rule/pull-request.md#pr-req-arch)). Run the
[agent-review checklist](../enforcement/checklist/general.md) over the diff, and the
[802.11 checklist](../enforcement/checklist/ieee80211.md) when the diff touches those subtrees.

If the branch touches a sealed path, the permission for it must be stated
([rule/sealing.md](../rule/sealing.md)).

## 5. Write the report

`audit/report/pull-request/pr-<n>.md`. One row per rule with a verdict — `PASS`, `FLAG`, `PARTIAL` or
`not verified` — and evidence for each. Then one numbered finding per problem, with the commit, what
it breaks, and what would repair it.

**Say what you did not check.** `PR-SERIES-BUILDS` needs a per-commit CI build, and a review that
does not run one records `not verified` rather than `PASS`. A report that hides its gaps is worse
than a shorter one.
