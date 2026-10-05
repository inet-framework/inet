# Implementation plan: the reviewer's questions

Checkout and revision: `inet-reviewer-questions`, branch `topic/reviewer-questions`, base
`d9e14c5af2` (master and origin/master). Rebases use this commit, not the moving `master` ref.
Plan revision: 2026-10-05.
Approval: on 2026-10-05 the maintainer asked for these changes in a worktree, with flexible checks:
"We need some tests but we can't be too strict." No sealed document and no recorded expectation is
in scope.

## 1. Problem and intended behavior

The maintainer often cannot tell from a pull request or a commit why it is necessary. The texts say
what changed and how, but not which problem the change solves, whether that problem is real, and
whether we want to solve it. A long description does not help when its first screen does not state
the problem.

The current rules mention the reason (`PR-MSG-WHY`, `PR-MSG-SUMMARY`, `PR-REQ-STORY`), but:

- no rule names the questions that the opening of a text must answer;
- no rule asks how we know that the problem is real, or for a preparation commit, which later
  commit needs it;
- the `PR-MSG-BODY` table tells authors that the "what" and the "how" do not belong in the body;
- no check looks at a pull request description, and the commit gate cannot see a body that only
  lists actions.

Intended behavior:

1. One section of `pull-request.md` defines the reviewer's questions: which problem, is it real and
   worth solving, what changes, how. The commit and pull-request rules cite it.
2. The answers scale with the change. A trivial change answers in its subject. Headings such as
   "Why", "What" and "How" are optional.
3. Checks give notes, never failures: one for a commit body that starts with actions and names no
   problem, and one gate that reads a pull request description.
4. The review restates the answers from the text alone. Each answer that it cannot restate becomes
   a question to the author.

## 2. Evidence

Calibration over `master~1000..master` at `d9e14c5af2`, 924 commit bodies, first paragraph without
trailers:

| Test on the first paragraph | Bodies | Share |
| --- | --- | --- |
| no word from a broad list of reason words | 182 | 19.7 % |
| starts with an imperative action (`Add`, `Rename`, `Convert`, ...) | 60 | 6.5 % |
| both | 15 | 1.6 % |

The reason-word test alone flags good messages ("Two classification rules disagreed. ..."), so it
is too noisy for a note. The combined test flags 15 bodies, and they are the action lists that the
maintainer describes, for example "Rename the Neighbour Discovery packet object-names ..." and
"Convert PimSm off its IPv4-typed scaffolding ...". The first version of pull request 1273 and its
nine-commit version pass the combined test: each body starts with a problem.

The calibration of pull request descriptions needs the GitHub API. The anonymous limit of this
machine was used up at the time of writing; step 6 records that calibration.

## 3. Scope and acceptance criteria

In scope: `rule/pull-request.md`, `guide/review-a-pull-request.md`, `guide/contribute-a-change.md`,
`enforcement/checklist/general.md`, `enforcement/README.md`, `check-commits.sh`, a new
`message_opening.py`, a new pull-request description gate with its tests, and a new
`.github/pull_request_template.md`.

Out of scope: a CI job that runs the description gate, and mandatory headings.

Acceptance criteria:

1. The rules define the questions once and cite them; a trivial change stays exempt.
2. The commit gate gives the new note on a body that starts with an action and names no problem,
   and stays quiet on the other forms. The exit status does not change.
3. The description gate reads a description from a file, from standard input, or from a pull
   request number, and gives notes only.
4. The enforcement regressions pass, and `check-links.sh`, `check-seals.sh`, `check-commits.sh` and
   `check-classification.sh` pass on this series.

## 4. Design and reasons

- **Questions, not headings.** The maintainer wants the answers, not a form. A heading list would
  make simple changes heavier and would let a long text hide behind four headings.
- **Proportion.** A typo fix answers in its subject. A feature answers in its opening paragraphs.
- **Read-back as the T4 test.** A reviewer restates each answer from the text without the diff.
  This test is flexible, and it turns a gap into a concrete question for the author.
- **One opening reader.** `message_opening.py` holds the word lists for both gates, so that the
  commit note and the description note cannot drift apart.
- **A template with a comment only.** `.github/pull_request_template.md` holds an HTML comment
  that GitHub does not render, so a description that keeps it stays clean.

## 5. Affected documents and compatibility

No source, model or recorded expectation changes. Authors see a new note from `check-commits.sh`
and a template text when they open a pull request on GitHub. Exit statuses stay the same.

## 6. Implementation steps

Each row is one commit with the group `reviewer-questions` and a `Plan:` line that names this file.

| # | Status | Purpose | Files | Change | Verification |
| --- | --- | --- | --- | --- | --- |
| 1 | done | Record this plan | this file | new plan | — |
| 2 | done | The opening of a commit body answers the reviewer's questions | `pull-request.md`, `review-a-pull-request.md`, `checklist/general.md`, `contribute-a-change.md` | new section with the questions; `PR-MSG-SUMMARY`, `PR-MSG-WHY` and the `PR-MSG-BODY` table cite it | `check-links.sh` |
| 3 | done | The commit gate sees a body that starts with actions | `check-commits.sh`, new `message_opening.py`, `test_check_commits.py`, `pull-request.md`, `enforcement/README.md` | note when the first sentence is an action and the first paragraph names no problem | unit tests; gate on master |
| 4 | done | A description opens with the answers | `pull-request.md`, `review-a-pull-request.md` | rewrite part 1 of `PR-REQ-STORY`; proportion for small pull requests | `check-links.sh` |
| 5 | done | GitHub shows the questions to the author | new `.github/pull_request_template.md` | comment-only template | render check |
| 6 | done | A gate reads a description | new `check-pr-description.sh`, new `check_pr_description.py`, new `test_check_pr_description.py`, `pull-request.md`, `review-a-pull-request.md`, `enforcement/README.md` | notes for a missing opening, evidence first, an action-first opening, and a large pull request with a one-line opening | unit tests; recent descriptions |
| 7 | done | The review restates the answers and asks the open questions | `review-a-pull-request.md`, `checklist/general.md` | read-back in the report | `check-links.sh` |
| 8 | pending | Close the plan | this file | record, then move to `plan/done/` | — |

## 7. Verification

```sh
python3 -m unittest discover -s doc/project/enforcement -p 'test_*.py'
doc/project/enforcement/check-links.sh
doc/project/enforcement/check-seals.sh
doc/project/enforcement/check-commits.sh d9e14c5af2..HEAD
doc/project/enforcement/check-classification.sh d9e14c5af2..HEAD
```

The checks show that the notes fire on the calibrated forms. They cannot show that an answer is
true or that a problem is worth solving; that stays with the reviewer and the maintainer.

## 8. Risks and decisions

- A note on word lists can miss a bad message and can fire on a good one. The combined test fired on
  1.6 % of master's bodies, and the note never changes an exit status.
- The template appears for every contributor on GitHub after a push. It is a comment, so it adds no
  visible text.

## 9. Implementation record

Filled in as the steps land.
