# Implementation plan: reviewable commits and pull requests

Checkout and revision: `inet-reviewable-changes`, branch `topic/reviewable-changes`, from master
`ee4460cb1d`. No local changes at the start.
Plan revision: 2026-10-05.
Approval: on 2026-10-05 the maintainer accepted the proposal to record this plan and to make the
changes in a worktree, one commit for each rule change. The approval covers the steps in section 6.
No sealed document and no recorded expectation is in scope.

## 1. Problem and intended behavior

The project rules ask for one change in each commit and for a pull request of reviewable size. A
pull request that holds a large feature in one commit can still pass the audit. The audit of the
first version of pull request 1273 is the example.

That version had one commit with 67 files, 2098 changed source lines and a 541-word message. It
held at least three changes that could stand alone. A fix of radio commands moved 26 fingerprint
rows in DCF and Block Ack configurations. A lifecycle reset added stop and crash handling to the
MAC. The TXOP admission moved 11 HCF rows. The message also held two sections of pull-request
revision history and two test logs.

After review, the author divided the pull request into nine commits. The isolated radio fix
reproduced all 26 rows. The single commit had only inferred that cause.

The intended behavior of the rules after this change:

1. A rule tells the author to divide a commit wherever a part can stand alone, and gives the
   reviewer signs that are stronger than "the subject has no 'and'".
2. A commit body starts with a short summary that a reviewer understands in about two minutes.
3. A pull request description starts with a summary of a few paragraphs that a reviewer reads in
   about five minutes.
4. The commit gate gives notes for the measurable signs. The review guide asks for an explicit
   verdict with evidence, so that "no gate fired" does not become a `PASS`.

## 2. Current rules and evidence

| Rule or tool | Current text or behavior | Gap |
| --- | --- | --- |
| `PR-SPLIT-ONE-CHANGE` | One change is one decision. The test is a subject without "and". | An abstract subject passes. No test for "can a part stand alone". |
| `PR-SPLIT-PREPARE` | A refactor comes before the fix that needs it. | No word on a prerequisite fix or on a new mechanism before its first user. |
| `PR-MSG-WHY` | The body gives the reason. The "what" must not be restated. | Nothing asks for a summary first; authors write lists of actions. |
| `PR-MSG-FACTS` | No progress notes. The gate checks only attribution trailers. | Revision history and test logs pass. |
| `PR-REQ-STORY` | Topic, reason, reading order, tests, baselines. | No summary first; no reading time. |
| `PR-REQ-TOPIC` | One topic, of a size a reviewer can hold. | No word on a step that is useful alone and moves behavior outside the topic. |
| `CR-DEPTH-DIRECTION` and `CR-TAG-SUBJECT` | Directions are not exclusive; a mixed marker "is meant to look wrong". | The two texts disagree. No gate lists mixed markers. |
| `review-a-pull-request.md` step 5 | Asks "does the subject need an 'and'?" | Weak test; a judgment rule defaults to `PASS`. |
| `PLR-PROPORTION` | Order the steps by their dependencies. | Nothing ties the commits to the plan's steps. |

Measurements over `master~1000..master` at `ee4460cb1d` (1000 commits, merges excluded). Source
lines count `.cc`, `.h`, `.ned` and `.msg` files under `src/`, without `_m.h` and `_m.cc`:

| Measure | p50 | p95 | p99 | Pull request 1273, first version |
| --- | --- | --- | --- | --- |
| Changed source lines, commits that change source (410) | 34 | 425 | 1046 | 2098 |
| Body words | 92 | 291 | 440 | 541 |
| Words in the first body paragraph | 48 | 94 | 121 | 41 |

24 of the 1000 commits change more than 400 source lines. Most of them add a new protocol, import
reference code, or do a sweep. 23 of the 1000 commits have a `Validation:` paragraph.

## 3. Scope and acceptance criteria

In scope: `rule/pull-request.md`, `rule/classification.md`, `rule/planning.md`, the guides
`review-a-pull-request.md`, `contribute-a-change.md` and `write-an-implementation-plan.md`,
`enforcement/checklist/general.md`, `enforcement/README.md`, the gates `check-commits.sh` and
`check-classification.sh`, a new regression test for `check-commits.sh`, and the generated index
in `audit/seal-list.md`.

Out of scope: a CI job for `check-commits.sh` (the gate stays "by hand"), a change to the
`Change:` vocabulary, and a re-audit of open pull requests.

Acceptance criteria:

1. Each new or changed rule has its identifier heading, bold lead sentence, index row and
   enforcement line (`DR-ID-HEADING`, `DR-INDEX`, `DR-TIER`).
2. `check-commits.sh` gives the new notes on the first version of pull request 1273
   (`0c69196d37`): source size, two fingerprint patterns, body length, revision history and test
   log. The new notes never change the exit status.
3. `check-classification.sh` lists each mixed direction as a question and its exit status does not
   change.
4. The enforcement regressions pass: `python3 -m unittest discover -s doc/project/enforcement -p 'test_*.py'`.
5. `check-links.sh`, `check-seals.sh`, `check-commits.sh` and `check-classification.sh` pass on
   this series.

## 4. Design and reasons

- **Notes, not violations.** Each size and length limit has exceptions that a script cannot see:
  a new protocol, imported code, a sweep. A violation that fires on correct commits teaches the
  reader to skip the gate (`checklist/general.md`, "precision over recall"). A note points the
  reviewer at a question, and the review guide asks for the answer.
- **Measured limits.** The numbers come from master, as the 50-line limit of `PR-MSG-BODY` does:
  400 source lines (p95 is 425), 300 body words (p95 is 291), 120 words in the first paragraph
  (p99 is 121).
- **Fingerprint patterns.** A moved row changes some of its ingredient parts (`tplx`, `~tNl`,
  `~tND`, `tyf`). Rows that change different sets of parts usually have different causes. The gate
  counts the sets in each commit. A small Python reader in `enforcement/` parses the diff, as
  `commit_breakdown.py` does for the breakdown.
- **Rule text owns the policy.** The guides and the checklist cite the rules and do not copy them
  (`DR-CITE-DONT-REPEAT`).
- **One commit for each rule change.** The rule text, its gate note, its review step and its
  checklist item are one decision and travel together.

## 5. Affected documents and compatibility

No source, test model or recorded expectation changes. Authors get new notes from
`check-commits.sh` and `check-classification.sh`; the exit status of both gates stays the same.
Two new rule identifiers appear: `PR-SPLIT-SIZE` and `PR-MSG-SUMMARY`. The seal index changes its
unit counts for `rule/pull-request.md`.

## 6. Implementation steps

Each row is one commit with the group `reviewable-changes` and a `Plan:` line that names this file.

| # | Status | Purpose | Files | Change | Verification |
| --- | --- | --- | --- | --- | --- |
| 1 | done | Record this plan | this file | new plan | `check-links.sh` |
| 2 | done | A commit divides where a part stands alone | `pull-request.md`, `review-a-pull-request.md`, `checklist/general.md`, `contribute-a-change.md` | rewrite `PR-SPLIT-ONE-CHANGE` with the test for division and its signs; explicit verdict in step 5 | `check-links.sh` |
| 3 | done | The gate shows rows that move in different ways | `check-commits.sh`, new `fingerprint_moves.py`, new `test_check_commits.py`, `pull-request.md`, `enforcement/README.md` | note when one commit moves rows with more than one pattern | unit tests; `check-commits.sh 0c69196d37~1..0c69196d37` |
| 4 | done | A large commit says why it does not divide | `pull-request.md`, `check-commits.sh`, `test_check_commits.py`, `review-a-pull-request.md`, `audit/seal-list.md` | new `PR-SPLIT-SIZE`; note above 400 source lines | unit tests; gate on `0c69196d37` |
| 5 | done | Preparation covers fixes and new mechanisms | `pull-request.md`, `checklist/general.md` | widen `PR-SPLIT-PREPARE`; quick reference rows | `check-links.sh` |
| 6 | done | A body starts with a summary | `pull-request.md`, `check-commits.sh`, `test_check_commits.py`, `review-a-pull-request.md`, `checklist/general.md`, `audit/seal-list.md` | new `PR-MSG-SUMMARY`; amend the `PR-MSG-WHY` table; notes for paragraph and body length | unit tests; gate on `0c69196d37` |
| 7 | done | A message holds no revision history and no test log | `pull-request.md`, `check-commits.sh`, `test_check_commits.py`, `checklist/general.md` | amend `PR-MSG-FACTS`; note for history words and `Validation:` | unit tests; gate on `0c69196d37` |
| 8 | done | A description starts with a summary | `pull-request.md`, `review-a-pull-request.md` | rewrite `PR-REQ-STORY` | `check-links.sh` |
| 9 | done | A step useful alone goes first | `pull-request.md` | amend `PR-REQ-TOPIC` | `check-links.sh` |
| 10 | done | A mixed direction marks a commit that cannot divide | `classification.md`, `check-classification.sh` | align `CR-DEPTH-DIRECTION` and `CR-TAG-SUBJECT`; note for mixed directions | gate on the nine-commit series of pull request 1273 |
| 11 | done | The commits follow the plan's steps | `planning.md`, `write-an-implementation-plan.md`, `review-a-pull-request.md` | amend `PLR-PROPORTION`; review question | `check-links.sh` |
| 12 | done | Close the plan | this file | move to `plan/done/` | `check-links.sh` |

## 7. Verification

From the worktree root, after each step that changes a gate or a link:

```sh
python3 -m unittest discover -s doc/project/enforcement -p 'test_*.py'
doc/project/enforcement/check-links.sh
doc/project/enforcement/check-seals.sh
```

After the last step:

```sh
doc/project/enforcement/check-commits.sh master..HEAD
doc/project/enforcement/check-classification.sh master..HEAD
doc/project/enforcement/check-commits.sh 0c69196d37~1..0c69196d37
doc/project/enforcement/check-commits.sh 8c29416d8d..6d935f633f
doc/project/enforcement/check-classification.sh 8c29416d8d..6d935f633f
```

The two pull-request ranges need `git fetch origin +pull/1273/head:refs/remotes/pr/1273` and the
old commit `0c69196d37`, which this machine still has. The checks show the new notes on real
commits; they cannot show that reviewers act on the notes.

## 8. Risks and decisions

- A note can become noise. The limits sit at p95 and p99 of master, so about one commit in twenty
  that changes source gets a size note.
- `PR-MSG-FACTS` now moves test logs out of commit messages. 23 of the last 1000 commits on master
  have such a paragraph; the rule applies to new commits only.
- The rule changes stale no seal, because no unit of the three rule documents is sealed.

## 9. Implementation record

All twelve steps are done on `topic/reviewable-changes`, in twelve commits and a move. The branch
is local and not pushed.

The autosquash rebases used the `master` ref while it moved, so the series sits on master
`649d4756ae`, three commits after the start. Those three commits change only TCP files and the TCP
plan. The checks in the evidence table ran on that base.

### Decisions and facts from the implementation

- `fingerprint_moves.py` first printed two patterns with the same row count in an order that
  depended on the Python hash seed, because it iterated over a set. A gate must give the same
  output on every run. The reader now sorts its keys and its output. The correction went into
  step 3, and the tests pass under `PYTHONHASHSEED` 0 to 3.
- Steps 4 and 6 first left the gate inventory in `enforcement/README.md` unchanged. Each step now
  updates its own row.
- The new `PR-MSG-FACTS` note fired on the message of step 7, which quoted a phrase of revision
  history as an example. The message now describes the case without the quote. A quoted phrase can
  give a note; that is acceptable for a note, and the reviewer decides.
- The nine-commit series of pull request 1273 has five mixed directions, not six.
- `check-classification.sh` had no regression test. Step 10 adds `test_check_classification.py`.
- The steps also update `contribute-a-change.md` and add review-guide bullets for `PR-REQ-TOPIC`
  and `PR-REQ-STORY`, which section 6 did not list.

### Evidence

Commands ran from the worktree root on 2026-10-05.

| Check | Result |
| --- | --- |
| `python3 -m unittest discover -s doc/project/enforcement -p 'test_*.py'` | 121 tests, OK; 12 of them are new |
| The same tests, `check-seals.sh` and `check-links.sh` at each commit of the series, in a detached copy under `/var/tmp` | tests OK and seals exit 0 at every commit; the broken-link count stays the same at every commit |
| `check-links.sh` in the worktree | 1 broken link, to the generated `MplsPacket_m.h`, as on master |
| `check-commits.sh master..HEAD` | PASS, no note |
| `check-classification.sh master..HEAD` | PASS |
| New `check-commits.sh` notes, as in the second table below | exit status unchanged |

The `/var/tmp` copy has 164 broken links at every commit, because the sibling `../standards/`
project is absent next to it. The count does not change across the series.

The gates on pull request 1273. Both ranges exit with status 0, as before the change:

| Range | New notes |
| --- | --- |
| `0c69196d37~1..0c69196d37`, the single commit | rows move in 2 ways (26 `tplx`; 11 `tplx,~tND,~tNl`); 2098 source lines; body of 541 words; "Audit correction"; a `Validation:` paragraph |
| `8c29416d8d..6d935f633f`, the nine commits | 409 source lines in `06bc632d16`; 828 in `1cc9905c7d`; a `Validation:` paragraph in each commit; five mixed directions |

### Limits

The checks show that the notes fire on real commits and stay quiet on this series. They cannot
show that reviewers act on the notes. `check-commits.sh` and `check-classification.sh` still run by
hand only.
