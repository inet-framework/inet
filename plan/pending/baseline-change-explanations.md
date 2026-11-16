# Explanations of baseline changes

**Status:** in progress. Written 2026-11-16 from `master` at `b61fa3c136`. The user settled E1 to E8 and approved the plan on 2026-11-16.

## Goal

Each change of a recorded baseline comes with a reason for each kind of change: the fingerprints in `tests/fingerprint/store.json`, the statistical results in the `statistics` repository, and the chart images in the `media` repository. The reasons are in the message of the commit that changes the baseline. The description of the pull request summarizes the baseline changes of all its steps. The code that moves the baseline values is the smallest code fragment that produces these changes, so the reviewer understands it and can accept the reasons. How many baseline entries move does not matter: a change can move all of them for a good reason.

A gate checks that each reason and the summary are there. The gate cannot check that a reason is true: that stays with the reviewer and with the attribution run (`TR-BASELINE-ATTRIBUTION`).

## Facts

All facts come from the tree at `b61fa3c136`, the local clones, and measurements on 2026-11-16.

| # | Fact | Source |
| --- | --- | --- |
| F1 | Fingerprints: the rules want the reason in the message of the INET commit, for each group of moved rows (`TR-BASELINE-PROVENANCE`). The gates check three things: a commit below the behavior level does not change the store (`CR-OBL-INERT`); a commit that changes only baselines has a non-empty body (`PR-SPLIT-BASELINE`); a note when runs move in more than one way. No gate checks that a source commit that moves fingerprints explains them, or that its `Change:` trailer names `fingerprint`. | `check-commits.sh`, `check-classification.sh` |
| F2 | Statistics and charts: the baseline commit holds the record. The gate checks that the message holds the computed `Moved results:` block, and an analysis of at least 20 words, and gives a note when the tests move in more than one way. It does not check that each group has its own reason. The block has no class for the size of a change and no category for a renamed test or image. | `check_baselines.py`, `baseline_summary.py` |
| F3 | The description of a pull request reaches only `check-pr-description.sh`, which gives notes and never fails. `PR-REQ-STORY` asks for a list of the baseline updates (T4). | `check_pull_request.py`, `rule/pull-request.md` |
| F4 | A module `.test` file holds the test and its expected output together. | `tests/module` |
| F5 | Practice is better than the gates. 15 commits changed the fingerprint store from 2026-09-22 to 2026-11-16. All 13 with a `Change:` trailer name `fingerprint`, and all 15 messages mention fingerprints. | `git log` |
| F6 | The categories of these 15 commits: 3 add runs (12 to 289), 1 removes runs (25), 14 change runs, and no run is renamed. In one commit, the conversion from CSV files, the changed runs fall into 18 patterns of moved ingredient sets. | the store at each commit |
| F7 | 3 of the 15 commits change no source line: the conversion from CSV files and two records of missing runs. Of the other 12, 11 change at most 16 source lines, and one changes 296 (`f874a26bd9`). In the earlier, longer period of the plan `baseline-updates-in-pull-requests`, 87 % of the 70 commits that moved fingerprints changed at most 100 source lines. | `git log --numstat`; F13 of that plan |
| F8 | The 25 baseline commits of `statistics` after the last green point change 825 test files. By the largest relative change of a test file: 186 change by 1 % or more, 7 by less than 1 %, 1 by at most 1e-9, 630 only add or remove rows (two new drop reasons), and 1 has a change with no finite relative value. | `baseline_summary.analyze` over `9ab4c26..f76ce96` |
| F9 | Differences from another compiler or C library were at most about 6e-13 in relative terms; the smallest real trajectory changes were about 1e-3. | memory note of the baseline recipe, 2026-07-13 |
| F10 | A fingerprint store entry is one run and one ingredient set: working directory, ini file, config, run number, ingredients, fingerprint, simulation time limit, expected test result. | `tests/fingerprint/store.json` |

## Definitions

- **Baseline change**: an added, removed or changed entry of a recorded baseline: a run and ingredient set of the fingerprint store, a test file of `statistics`, a chart image of `media`.
- **Step**: a commit of a series that changes a baseline. It changes the fingerprint store, or it moves a pin over a baseline commit with results.
- **Category**: one line of the computed list of the baseline changes of a step (design, section 1).
- **Reason**: the text after " — " on a line of the list, written by the author.
- **Smallest fragment**: the smallest code change that produces the baseline changes of a step: the step itself, or a reduction (`TR-BASELINE-ATTRIBUTION`).
- **Size class**: the class of a changed statistical test file, by the largest relative change of its rows (design, section 1).

## Design

### 1. The categories

A tool computes the list of the baseline changes of a step. Each line is one category, with the count and the folders, so that the same baselines always give the same lines.

| Kind | Categories |
| --- | --- |
| Statistics, per test file | added; removed; renamed (a removed and an added file with equal rows); changed, one line for each pair of size class and group of moved row names. A group marks an added row name with `+` and a removed one with `-`. |
| Charts, per image | added; removed; renamed (equal bytes); changed |
| Fingerprints, per run | added; removed; renamed (equal fingerprints); changed, one line for each pattern of changed ingredient sets; ingredient sets added or removed; limit or expected result changed |

The size classes of a changed statistical test file (E2):

| Class | Largest relative change of a row |
| --- | --- |
| noise | at most 1e-9, the level of compiler and library differences (F9) |
| minor | less than 1 % |
| large | 1 % or more, or a change with no finite relative value, or a change of a text value |
| rows | no value changes; rows are only added or removed |

### 2. A reason for each line

The commit that changes the baseline holds the list, with a reason on each line after " — " (E1):

```
Baseline changes:
  statistical tests changed, large (44; examples/manetrouting 19, examples/wireless 10, ...): queueLength, packetDrop, queueingTime and 1104 more — a reception can draw two random numbers, so the drops and the queues move
  statistical tests changed, large (95; examples/wireless 44, ...): reception decision computation count, cache hit — the medium computes the decision of each part
  fingerprint runs added (3; examples/foo): all ingredients — the commit adds the example foo
```

- **The place**: the INET commit for the fingerprints, and the baseline commit for statistics and charts, as today.
- **The check**: the gate computes the list again. The text before " — " must equal the computed line, and a reason must follow. The gate counts no words (E7): the measure of an explanation is the size of the code fragment of section 4, not the length of the text.
- **Prose**: the message can hold more prose. The list replaces the `Moved results:` block and the check of 20 words.
- **Trailers**: the trailers `INET-Subject:` and `Attribution:` of a baseline commit stay.

### 3. The summary in the pull request

The description of a pull request has a section "Baseline changes" with one entry for each step: the subject of the step, then a summary of its baseline changes in the words of the author (E3, E6). The gate fails when the series has a step and the section does not name its subject with a summary.

The check runs in the trusted pull-request audit, which has the description. A run by hand takes the description with `--description-file` or `--pr`; without a description, the gate gives a note and does not check. The report of the gate prints the computed list of each step, so the reviewer and the author see it in the job summary.

### 4. The smallest fragment

The code that moves the baseline values of a step is the smallest code fragment that produces them (E7). A step that changes 100 lines of code and moves 500 baseline entries is wrong when 10 changed lines move the same 500 entries. Then the 10 lines are the step, and the other 90 lines go into a commit before it that moves no baseline (`PR-SPLIT-PREPARE`). Or a reduction shows the 10 lines. The number of moved entries is no measure: a step can move all fingerprints, charts or statistics for a good reason (E5).

- **The measure** is the size of the fragment: the changed source lines of the step (form `commit`) or of its reduction (forms `forward` and `reverse`). Above 30 lines the gate gives a note; above 100 lines it fails. These are the limits of `TR-BASELINE-ATTRIBUTION`.
- **Statistics and charts** have this rule already. The reduction is a file in `attribution/` of the baseline commit.
- **Fingerprints, new**: the same measure and the same forms. A fingerprint reduction is a patch in the message of the INET commit, after the line `Reduction:`, with the trailer `Attribution: forward` or `Attribution: reverse` (E8). The gate checks that the patch applies and counts its lines; the attribution run proves that it gives the fingerprints of its form.
- **No limit, only a reason**: a step whose baseline changes only add, remove or rename entries (a first record, a removal, a rename), and a step that changes no source line (a tool or version change, F7).
- **Effect on the old commits** (F7): one of the 12 source commits, `f874a26bd9` with 296 lines, is above the limit with no reduction.

### 5. The obligation in the trailer

A commit that changes the fingerprint store names `fingerprint` in the obligations of its `Change:` trailer (`CR-OBL-BASELINE`, now T3). Practice already does this (F5).

### 6. Out of scope

The expected outputs of module tests (E4, F4).

## Rule and document changes

| Document | Change |
| --- | --- |
| `rule/testing.md` | `TR-BASELINE-PROVENANCE`: the list with a reason for each line, for the three kinds, and the size classes. `TR-BASELINE-ATTRIBUTION`: the smallest fragment, for fingerprints too, with the reduction in the commit message, and the steps with no limit. |
| `rule/pull-request.md` | New `PR-REQ-BASELINES`: the description summarizes the baseline changes of each step. `PR-REQ-STORY` and the enforcement table refer to it. |
| `rule/classification.md` | `CR-OBL-BASELINE`: T3 for a commit that changes the fingerprint store. |
| `guide/change-a-baseline.md` | The list and its reasons, for fingerprints too; the section of the description; small steps for fingerprints. |
| `guide/review-a-pull-request.md`, `guide/contribute-a-change.md` | Short references. |
| `enforcement/README.md`, `.github/pull_request_template.md` | The new checks; the section "Baseline changes". |

## Steps

Each entry is one future commit, on the branch `topic/baseline-change-explanations` in the worktree `inet-baseline-change-explanations`. The gate comes before the rules, because the rule documents link to the gate.

| Id | Proposed subject | Depends on | Verification |
| --- | --- | --- | --- |
| X1 | plan: add: explanations of baseline changes | none | — |
| X2 | doc: change: a reason for each category of a statistical or chart baseline change | X1 | `baseline_summary.py`: categories, size classes, renames, the list, the reasons; `check_baselines.py` checks the reasons; `baseline-candidates.py` writes the list with empty reasons. Unit tests, and the list of each of the 25 `statistics` commits of F8. |
| X3 | doc: add: a reason for each category of a fingerprint change | X2 | The categories of the fingerprint store; the check of the steps that change it; the obligation `fingerprint`. Unit tests, and the lists of the 15 commits of F6. |
| X4 | doc: add: the smallest fragment of a fingerprint change | X3 | The size limits, the steps with no limit, and the reduction in the message: it applies, and its lines count. Unit tests; the 15 commits give one failure (`f874a26bd9`). |
| X4b | doc: add: the attribution run of a fingerprint change | X4 | `check-baseline-attribution.sh` also runs the moved fingerprint runs: at the parent against the old store, at the commit against the new store, and with the reduction. Unit tests with fake commands; one real reduction of a fingerprint commit of F6 in a copy of the CI job. |
| X5 | doc: add: the pull request summarizes the baseline changes of each step | X3 | The check of the description; `check_pull_request.py` passes the description. Unit tests, and the description of PR #1452 as data. |
| X6 | doc: change: rules for the explanation of each baseline change | X2 to X5, X4b | The rule documents of the table above. `check-seals.sh` and `check-links.sh` pass. |
| X7 | doc: change: procedure and template for the explanations of baseline changes | X6 | The guides, the README and the template. `check-links.sh` passes. |
| X8 | plan: change: record the results | X7 | — |
| X9 | plan: location: move the plan to done | X8 | Two commits, as `PR-SPLIT-MOVE` asks. |

## The open pull requests

The gate checks every pull request again at its next push. A pull request whose series changes the fingerprint store, or moves a pin over a result, then fails until its commits have the lists and its description has the section. There is no exception for the date, as for the other `PR-*` rules. After the landing, the sweep of `check-pull-request.yml` gives the number. The sweep of the statistical and chart tests still waits for a green `master`.

## Decisions

| # | Decision | Proposal | State |
| --- | --- | --- | --- |
| E1 | The format of a reason | On the same line as the computed text, after " — ". Rejected: a separate section with a key for each line. | settled, user 2026-11-16 |
| E2 | The size classes | noise at most 1e-9, minor below 1 %, large 1 % or more. | settled for now, user 2026-11-16 |
| E3 | The pull request | The description summarizes the baseline changes, because a series can have more than one step. | settled, user 2026-11-16 |
| E4 | The expected outputs of module tests | Out of scope. | settled, user 2026-11-16 |
| E5 | What makes a step too large | The size of its code, never the number of moved baseline entries: a step can move all fingerprints, charts or statistics for a good reason. | settled, user 2026-11-16 |
| E6 | The form of the summary | A section "Baseline changes" with one entry for each step, by its subject; the gate fails when a step has no entry. | settled, user 2026-11-16 |
| E7 | What a short explanation is | The smallest code fragment that produces the baseline changes, not a number of words. A step of 100 changed lines that moves 500 entries is wrong when 10 lines move the same entries. The gate counts no words in a reason. | settled, user 2026-11-16 |
| E8 | The place of a fingerprint reduction | A patch in the message of the INET commit, after the line `Reduction:`, with the trailer `Attribution: forward` or `reverse`. Rejected: a patch file in the INET tree, which would keep code that never runs. | settled, user 2026-11-16 |

## Limits

- The gate checks that a reason is there, not that it is true.
- The size limits count lines. Whether a smaller fragment exists, and whether the change is easy to understand, stay the decisions of the reviewer. A reduction proves that a fragment is sufficient, not that it is the smallest one.
- A renamed test or image is found only when its content is equal. A rename with a change counts as a removal and an addition.
- The size class looks at the largest change of a test file. One counter that changes by 200 % makes the file "large"; the group of row names then says what moves.
