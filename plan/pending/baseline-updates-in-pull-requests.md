# Baseline updates in pull requests

**Status:** in progress. Written 2026-11-12 from `master` at `7adc565ce9` (equal to `origin/master`). The user settled D1 to D7 and D10 on 2026-11-12. D8, D9 and D11 are open; their steps come later. Approval: on 2026-11-12 the user approved the start of the steps O1, O3, C1 and C2. Each later step needs a new approval.

## Goal

A pull request that moves a statistical result or a chart supplies the baseline update, or its CI fails. A baseline update is more than new files. The update holds a summary of the moved results, an analysis that connects them to the change, and an attribution. The attribution is a change of INET that is small, easy to understand, and proven to cause the moved results. A reviewer accepts a baseline change because the reviewer can read that small change and see that it is correct.

The plan gives the rules and the CI checks. It also gives the link between an INET pull request and the two baseline repositories, and the method for the open pull requests.

## Facts

All facts come from the tree at `7adc565ce9`, the local clones, and the GitHub API on 2026-11-12.

| # | Fact | Source |
| --- | --- | --- |
| F1 | The workflows `statistical-tests.yml` and `chart-tests.yml` run on a push to `master` and by hand. No pull request runs them. The fingerprint workflow does run on pull requests. | workflow files |
| F2 | The two workflows clone the head of `master` of `inet-framework/statistics` and `inet-framework/media`. The INET tree does not record the baseline commit that goes with an INET commit. The folders `/statistics` and `/media` are in `.gitignore`. The job log does not print the baseline commits. | workflow files, `.gitignore`, F2 of the catch-up plan |
| F3 | `inet.opp` declares each baseline repository in `test_parameters[<kind>]["baseline"]` with `repository` and `folder`. opp_repl defines a third field, `ref`. opp_ci checks out `ref` when the field is there. INET does not set `ref`. | `inet.opp`, `opp_repl/opp_repl/simulation/project.py:273`, `opp_ci/opp_ci/executor.py:877` |
| F4 | The rules for a recorded expectation are `TR-BASELINE-DELIBERATE`, `TR-BASELINE-PROVENANCE`, `TR-BASELINE-COMMIT`, `PR-SPLIT-BASELINE`, `CR-OBL-BASELINE` and `CR-OBL-INERT`. The rules name the statistical baseline, but their procedure and their gates cover only files in the INET tree. No rule names the chart baseline. | `doc/project/rule/*.md`, `guide/change-a-baseline.md` |
| F5 | The gates read only tree paths: `check-commits.sh` uses `^tests/.*\.(csv\|json)$`, and `check-classification.sh` uses `tests/(fingerprint\|statistical)/`. A baseline in another repository is not visible to them. The obligations of the `Change:` trailer are `fingerprint` and `statistical`; `chart` is not a value. | the two gate scripts |
| F6 | The rule that a baseline update needs a test that passes on the parent commit is a user decision of 2026-11-09. No rule document holds it. | memory note, F14 of the catch-up plan |
| F7 | `statistics` holds 931 `.sca` files, 1.2 GiB in the work tree and 411 MiB as a pack. `media` holds 1,074 files, 226 MB and 350 MiB. | `du`, `git count-objects` |
| F8 | The statistical test compares each scalar row exactly and reads run 0 only. The chart test compares images with an RMSE metric; the default limit is 0. A failed statistical test writes a `.diff` file, and a failed chart test writes `-new.png` and `-diff.png`. CI uploads only these files. | opp_repl `test/statistical.py`, `test/chart.py`, workflow files |
| F9 | A baseline that a developer machine records can fail in CI. Error rates differ in the last digit with another compiler or C library, and the `gsandcs` schedule depends on the z3 version. The local copy of the GitHub job in podman gives the CI values. | memory note of the baseline recipe |
| F10 | A branch with the name of the INET branch is the present practice. `statistics` has `topic/standards-tests-{rip,igmp-mld,nd}-level2-fixes` on origin, and `topic/physicallayer-part-error-decisions` and `topic/physicallayer-content-errors` only local. `media` has `topic/physicallayer-part-error-decisions` only local. A person must merge each branch into `master` at the correct time, by hand. | `git branch -a` in the two clones |
| F11 | GitHub gives a commit by its SHA from the upstream repository when the commit is the head of a pull request from a fork. Test: a fetch of `bbbb09aeac`, the head of #1450, from `inet-framework/inet`. | `git fetch` on 2026-11-12 |
| F12 | 96 pull requests to `master` are open. 94 touch a path that can change a result. 39 change the fingerprint store. 42 come from a fork. 6 conflict with `master`. Authors: adamgeorge309 44, mgonzalezlopezudc 36, levy 6, tabgab 4, six others 1 each. | `gh pr list` |
| F13 | Since `9ccb5c8e59`, 70 commits on `master` move a fingerprint baseline together with a source change. Their changed source lines: median 11; 84 % have 60 or fewer, 87 % have 100 or fewer, 94 % have 200 or fewer; the maximum is 1,249. | `git log --numstat 9ccb5c8e59..7adc565ce9` |
| F14 | The user merges with a rebase. Thus the SHA of an INET commit changes when its pull request lands. | memory note, repository settings |
| F15 | `master` is red for the two tests: 72 FAIL and 9 ERROR of 937 statistical tests, and 19 FAIL of 229 chart tests. The catch-up plan repairs this state. | F6 of the catch-up plan |
| F16 | One CI run of each of the two tests needs 32 to 109 minutes with the wait for a runner. The job copy on this machine needs about 10 minutes for the build and the two tests. | `gh run list`, the catch-up plan |
| F17 | No rule unit that this plan changes has a `🔒` flag. | `grep` in the three rule documents |

## Definitions

- **Baseline repository**: `inet-framework/statistics` or `inet-framework/media`.
- **Pin**: the full SHA of a baseline commit, written in `inet.opp` as the `ref` of the baseline. Each INET tree has two pins.
- **Baseline commit**: one commit in a baseline repository. It holds the new files and the record.
- **Record**: the summary, the analysis and the attribution of one baseline commit.
- **Cause commit**: the INET commit that moves a pin. Its parent has the old pin.
- **Moved test**: a statistical test or a chart whose baseline file changes in a baseline commit.
- **Candidate**: a result file or an image that a run made. A candidate becomes a baseline only in a baseline commit.
- **Reduction**: a patch of INET source code that is smaller than the cause commit and reproduces the move of the results.

## Requirements

| # | Requirement |
| --- | --- |
| R1 | A pull request that moves a statistical result or a chart fails CI, unless it supplies the baseline update. |
| R2 | A baseline update holds the changed files, a summary of the moved results, an analysis of why the change causes them, and an attribution to a small change. |
| R3 | The attribution is one of two kinds. The preferred kind is a small change that produces the new results from the old code. The other kind is a small change that produces the old results from the new code. |
| R4 | A run proves the attribution. A statement in prose is not sufficient. |
| R5 | Each INET commit names the exact baseline that it passes with. Thus `master` has no red interval when a pull request lands, and a bisect knows the baseline of each commit. |
| R6 | The approval of a baseline change is explicit and is apart from the approval of the source change (`TR-BASELINE-DELIBERATE`). |
| R7 | A contributor with no CI environment and no push right to the INET repositories can obey the rules. |
| R8 | A pull request cannot change the gate that examines it. Code of a pull request runs only with a read-only token. |
| R9 | A baseline changes only for a test that passes on the parent commit, and only the moved baselines change. |

## Design

### 1. The link: each INET commit pins its baselines

`inet.opp` gets the `ref` field for the two baselines. The value is a full SHA, never a branch name.

```
"statistical": {..., "baseline": {"repository": "inet-framework/statistics", "ref": "<40 hex digits>", "folder": "statistics"}},
"chart":       {..., "baseline": {"repository": "inet-framework/media", "ref": "<40 hex digits>", "folder": "media", "read_folder": "doc/media"}},
```

The cause commit changes the source and the pin together. This is `PR-SPLIT-BASELINE` for a baseline in another repository: the commit is true alone, because its tests pass against its own pins. The pin gives these properties:

- **The link is exact for each commit.** The baseline commits of INET commit C are the range from the pin of its parent to the pin of C. No trailer and no branch name is necessary to find them.
- **The link survives a rebase.** The pin is in the tree, so a rebase of the INET series keeps the pin.
- **No red interval.** CI checks out the pins. Thus `master` of INET and `master` of a baseline repository do not have to move at the same moment.
- **The baseline is a fixed input.** Today the head of `statistics` and of `media` is an input that moves between CI runs (F2).

**Where the pinned commit lives before the pull request lands.** A member pushes a branch with the name of the INET branch to the baseline repository, as today (F10). A contributor from a fork opens a pull request from a fork of the baseline repository to the upstream baseline repository. In the two cases CI can fetch the pinned commit by its SHA from the upstream repository (F11). The pull request in the baseline repository is the place for the explicit approval of R6: a maintainer approves the values there.

**After the pull request lands.** `master` of each baseline repository moves to the pin of INET `master` by a fast-forward. Thus `master` of a baseline repository is always the baseline of INET `master`, and the pinned commits stay reachable.

**Two pull requests that move a pin conflict in `inet.opp`.** After the first one lands, the second one rebases its baseline branch onto the new baseline `master`. Then it writes the new SHAs into its commits. A helper does the two actions. The conflict is a correct signal: the baseline of the second pull request must stand on the baseline of the first one.

### 2. The baseline commit and its record

One cause commit has at most one baseline commit in each baseline repository. If the moved results need more than one explanation, the INET commit holds more than one change, and `PR-SPLIT-ONE-CHANGE` divides it.

The record is in the message of the baseline commit. `git blame` on a baseline file then shows the commit that explains the values. The INET commit message keeps its present task: it says which behavior changes and why the new behavior is correct.

```
physicallayer: update the statistics of the wireless simulations with errors

<Analysis. For each group of moved rows: what moves, in which direction, by
how much, and why the INET change causes that move.>

Moved results:
  tests: 44 (showcases/wireless 30, examples/wireless 14)
  rows: 1203 changed, 12 added, 0 removed
  - packetErrorRate:* in **.radio.receiver: 620 rows, -3.1 % to +0.4 %
  - ...

INET-Subject: physicallayer: change: decide the header and the data of a reception separately
Attribution: reverse attribution/part-error-decisions.patch
```

- **Summary.** A tool makes the `Moved results:` block from the difference of the two baseline commits. The gate computes the block again and compares. Thus the summary is always true. For `media`, the block lists each changed, added and removed chart.
- **Analysis.** Prose by the author. Each group of the summary needs its explanation (`TR-BASELINE-PROVENANCE`). For a chart, the text says how the picture changes and why.
- **`INET-Subject:`** is the exact subject of the cause commit. A SHA is not possible, because the SHA changes when the pull request lands (F14). A re-record for a cause that is already on `master` uses `INET-Commit: <sha>`, as the catch-up plan does.
- **`Attribution:`** has one of the values of section 3.

A baseline commit changes only baseline files, and one reduction file when the attribution needs it.

### 3. The attribution

| Value | Claim | Proof |
| --- | --- | --- |
| `commit` | The cause commit is the small change. | The moved tests pass at the parent with the old pins and at the cause commit with the new pins. The commit is at or below the size limit. |
| `forward <file>` | The patch applies to the parent of the cause commit. The parent with the patch gives the new results. | The moved tests pass at the parent with the patch against the new pins. |
| `reverse <file>` | The patch applies to the cause commit. The cause commit with the patch gives the old results. | The moved tests pass at the cause commit with the patch against the old pins. |
| `first-record` | The cause commit adds the test. No earlier values exist. | Each file of the baseline commit is new. |
| `removal` | The cause commit removes the test. | The baseline commit only removes files. |
| `environment` | No INET source change is the cause: a new version of OMNeT++, of a tool, of a compiler or of a library. | The cause commit changes no model source, and its message names the version change. |

The best form is `commit`. `PR-SPLIT-PREPARE` already asks for it: the series first prepares with no change of results, and a last, small commit turns the behavior on. The author makes a reduction only when the cause commit cannot be small. A reduction is an experiment. It does not have to be code that can land.

The reduction file is in the baseline commit, in the folder `attribution/`. If one cause commit moves the two pins, the two baseline commits hold the same file.

Two size limits apply to the cause commit of the `commit` form and to each reduction (decision D5, with F13). Above 100 changed source lines the check fails. Above 30 changed source lines the gate gives a note. The note asks the reviewer one question: can a smaller change show the same move?

### 4. The checks

| Id | Check | Tier | Where it runs |
| --- | --- | --- | --- |
| B1 | Each commit of the series has two pins, and each pin is a full SHA. | T3 | new gate `check-baselines.sh`, inside `check-pull-request.sh`, in the trusted workflow |
| B2 | The pin of a commit is equal to the pin of its parent, or is a direct child of that pin in the baseline repository. | T3 | the same gate |
| B3 | Each pinned commit is in the upstream baseline repository. | T3 | the same gate |
| B4 | The record of each baseline commit is complete: the summary block is equal to the computed one, an analysis is there, the trailers are valid, and the reduction file exists when the attribution names one. The commit changes only permitted files. Each changed statistical file differs in a value from its earlier version, so a blanket re-record fails. | T3 | the same gate |
| B5 | The record agrees with the cause commit: `INET-Subject:` is its subject; its `Change:` trailer names `statistical` or `chart` and the depth `behavior`; the change is at or below the fail limit of D5, and the gate gives a note above the note limit; a reduction applies to the tree that its form names. | T3 | the same gate |
| B6 | The description of the pull request names the pull request or the branch in each baseline repository whose pin moves. | T3, note | `check-pr-description.sh` |
| V1 | The complete statistical test and the complete chart test pass on the merge ref of the pull request, against the pins of that tree. | T2 | the two test workflows, with a `pull_request` trigger |
| V2 | For each cause commit: the moved tests pass at the parent against the old pins and at the cause commit against the new pins, and the reduction gives the result that its form claims. | T2 | new workflow, `pull_request` trigger |
| M1 | After a push to `master`: `master` of each baseline repository moves to the pin by a fast-forward, or the job fails. | T3 | new job on `push` |

The gate of B1 to B5 reads the pull request only as data. It reads `inet.opp` with `git show` and the `ast` module. It fetches the baseline commits with a partial clone. It has a small reader for `.sca` files and needs no OMNeT++. Thus it can run in the trusted workflow (R8).

V1 and V2 run code of the pull request, so they use the `pull_request` event and its read-only token. Each workflow has a first job that examines the changed paths. When no path can change a result, the test job does not run, and the check is green.

**Candidates from CI.** When V1 fails, the job runs the update command for the failed tests and uploads an artifact. The artifact holds the candidate files, the computed summary, and the versions of OMNeT++, opp_repl and the compiler. The author puts the candidates into the baseline branch. Thus the values come from the CI environment (F9), and a contributor needs no local copy of that environment (R7).

**How R1 holds.** A pull request that moves a result and does not move the pin fails V1. A pull request that moves the pin with no record fails B4. A record with a false attribution fails V2. A pin that points to values that are not the results fails V1.

### 5. Procedure for the author

1. Push the pull request. If V1 is green, no baseline moves, and the work is complete.
2. If V1 is red, read the summary of the job. Decide for each moved test: is the move intended? A move that is not intended is a regression. Repair the code.
3. Make the cause commit small, or write a reduction and run it.
4. Download the candidate artifact. Commit the files to a branch of the baseline repository, with the record in the message.
5. Write the new pin into `inet.opp`, in the cause commit. Add `statistical` or `chart` to its `Change:` trailer.
6. Push the baseline branch, and open a pull request there if the branch is in a fork. Name it in the description of the INET pull request. Push the INET branch.
7. After another pull request lands and moves a pin: rebase the baseline branch, then run the helper that writes the new pins.

### 6. Rule and document changes

| Document | Change |
| --- | --- |
| `rule/testing.md` | New `TR-BASELINE-PIN` (each commit pins its baselines; CI reads no baseline that moves). New `TR-BASELINE-ATTRIBUTION` (section 3). New `TR-BASELINE-PARENT` (R9, F6). `TR-BASELINE-DELIBERATE` and `TR-BASELINE-PROVENANCE` name the chart baseline and the record of section 2. `TR-BASELINE-COMMIT` gives the pin as the form for a baseline in another repository. |
| `rule/pull-request.md` | `PR-SPLIT-BASELINE`: the pin moves in the cause commit. `PR-REQ-STORY`: the description names the baseline pull requests. Rows of the enforcement table. |
| `rule/classification.md` | New obligation value `chart`. `CR-OBL-INERT` includes a pin. `CR-OBL-BASELINE` goes from T4 to T2. |
| `guide/change-a-baseline.md` | The procedure of section 5, the record format, the candidates, the rebase of the pins. |
| `guide/review-a-pull-request.md`, `guide/contribute-a-change.md`, `guide/run-the-gates.md` | Short references to the new checks and to the guide. |
| `enforcement/README.md` | Rows for the new gate and the new workflows. |
| `.github/pull_request_template.md` | One line: name the baseline pull requests. |
| `README.md` in `statistics` and in `media` | New file: the role of the repository, the record format, the `attribution/` folder, a link to the INET guide. |

## The open pull requests

The rules apply to each open pull request, with no exception for its date. This is the same decision as for the `PR-*` rules on 2026-10-13.

1. **Pilot, before the gates are on `master`.** #1452 and #1443 already have baseline branches (F10). Convert them by hand: write the pins into the cause commits, and give the baseline commits the record format. Then run the gate and V2 in the local job copy. The pilot finds the faults of the rules and gives the first measured times. #1452 has two statistics commits for one behavior commit, so the pilot also tests the rule of one baseline commit.
2. **Sweep, after `master` is green and the checks are on.** An open pull request gets no event without a push. A workflow that a maintainer starts by hand runs V1 on the merge ref of each selected pull request and posts a commit status. It works as the audit sweep of `check-pull-request.yml`. The job that runs the code has a read-only token; a second job posts the status.
3. **Classes of the sweep result.**

| Class | Result | Next action |
| --- | --- | --- |
| No path that can change a result (2 of 96 today) | green | none |
| The two tests pass | green; no baseline moves, and the run proves it | none |
| Conflict with `master` (6 or more today) | no merge ref, so no run | the author rebases |
| A test fails | red, with the candidate artifact and the summary | the author does the procedure of section 5 |

4. **Number of pull requests with work.** The sweep gives the number. The 39 pull requests that change the fingerprint store change a trajectory, so most of them will be in the last class.
5. **Who does the work.** The author of each pull request. For the 6 pull requests of levy, this task does the work. For the 80 pull requests of the two frequent authors, the status and the guide tell them what to do (decision D8).
6. **Order.** Each pull request that moves a pin conflicts with the others after one of them lands. Each author rebases the pins one time, immediately before the pull request lands.

## Relation to the catch-up plan

The catch-up plan (`plan/pending/statistical-and-chart-baseline-catch-up.md` on `topic/baseline-catch-up`) repairs the baselines of the commits that are on `master`. This plan controls the commits that come after. This plan does not change the catch-up plan, its branches or its job copy.

- **The pins can go to `master` before the catch-up ends.** Their first values are the present heads: `statistics` `e8f63b9` and `media` `fd436d7`. CI then gives the same results as today, with a fixed baseline input.
- **V1 and V2 start on pull requests only after `master` is green** (decision D7). On a red `master`, each pull request shows the 100 failures of `master` (F15), and a check that is always red has no value.
- **Until then, the record format applies by hand** to each pull request that lands and moves a result. The catch-up walk reads that record when it comes to the commit.
- **The catch-up lands as one move of the two pins** to the heads of its chain. The chain does not descend from the present `master` of the baseline repositories. If its decision D5 chooses a merge commit, each new pin descends from the old one. The rule of one baseline commit does not apply to that push.
- **The record format is the same.** The catch-up uses `INET-Commit: <sha>`, which this plan permits for a cause on `master`.

## Steps

Each entry is one future commit. INET work is on the branch `topic/baseline-updates-in-pull-requests`, in the worktree `inet-baseline-updates-in-pull-requests`. opp_repl work is on a branch of `main`, and goes to `github_ci` and `opp_ci` after that.

| Id | Repository | Proposed subject | Depends on | Verification |
| --- | --- | --- | --- | --- |
| O1 | opp_repl | test: check out the pinned commit of a baseline repository | none | New command: clone or fetch, check out `ref` by SHA, print the commit. The two tests print the baseline commit and report a checkout that is not at the pin. Self-tests with two local repositories. |
| O2 | opp_repl | test: check the attribution of a baseline change | O1 | New command for V2: build the commits in worktrees, run the moved tests, apply the reduction. Check with the pilot. |
| O3 | opp_ci | executor: fetch the pinned baseline commit by its SHA | none | Today the code runs `git fetch origin`, which does not get a commit that only a pull request holds. Check with a pin on a pull request head. |
| C1 | INET | plan: add: baseline updates in pull requests | none | — |
| C2 | INET | tests: change: pin the commits of the statistics and media repositories | O1 | `inet.opp` and the two workflows. The push run on `master` prints the two pins and gives the failures of the run before it. |
| C3 | INET | doc: change: rules for a baseline in another repository | none | The three rule documents of section 6. `check-seals.sh` and `check-links.sh` pass. |
| C4 | INET | doc: change: procedure for a statistical or chart baseline | C3 | The guides of section 6. `check-links.sh` passes. |
| C5 | INET | enforcement: add: gate for the baseline pins and records | C2, C3 | `check-baselines.sh`, `check_baselines.py`, `baseline_summary.py`, `test_check_baselines.py`; the `chart` obligation and the pin in `check-classification.sh`; the gate in `check_pull_request.py`. Unit tests with small repositories: one case that passes and one that fails for each of B1 to B5. |
| C6 | INET | enforcement: add: write the new baseline pins after a rebase | C5 | `baseline_pins.py rebase`, with a unit test on a series of three commits and two pin moves. |
| C7 | INET | ci: add: statistical and chart tests on pull requests | C2, green `master` | `pull_request` trigger, the path job, the candidate artifact, the job summary. Check on the pilot pull request. |
| C8 | INET | ci: add: check of the attribution of a baseline change | O2, C7 | New workflow for V2. Check on the pilot: a correct reduction passes, and a changed reduction fails. |
| C9 | INET | ci: add: sweep of the baseline tests over the open pull requests | C7 | Start it for three pull request numbers, then for `open`. |
| C10 | INET | ci: add: move the baseline masters to the pins after a push | C2, D9 | Job M1. Needs a deploy key for each baseline repository. |
| S1, S2 | statistics, media | add the README | C4 | — |
| C11 | INET | plan: record the results, then move the plan to `plan/done` | all | Two commits (`PR-SPLIT-MOVE`). |

C1 to C6, O1 to O3 and the pilot do not depend on the catch-up. C7 to C9 wait for a green `master`.

**Negative checks on the pilot.** Each of these changes must turn the named check red:

- remove the pin move (V1);
- remove the `Attribution:` trailer (B4);
- change one line of the reduction (V2);
- add a baseline file whose values did not move (B4);
- pin a commit that is only local (B3).

## Decisions

| # | Decision | Proposal | State |
| --- | --- | --- | --- |
| D1 | The link between an INET commit and its baselines | A pin in `inet.opp`, moved in the cause commit. Rejected: only a branch with the same name (no link for each commit, a red interval when the pull request lands, R5 fails); git submodules (the same information, but each clone and each present checkout changes); the baselines in the INET tree (1.2 GiB, F7); a hash for each baseline file in the INET tree (conflicts only for the same file, but a new design of the two tests). The user accepts the conflict of the pins: a change that moves a baseline usually conflicts in the baseline files also. | settled, user 2026-11-12 |
| D2 | The place of the pinned commit before the pull request lands | A branch with the name of the INET branch in the upstream baseline repository, or a pull request from a fork to that repository. The approval of the values is the review of that pull request. | settled, user 2026-11-12 |
| D3 | The place of the record | The message of the baseline commit, and the reduction as a file in `attribution/` of that commit. Rejected: a record file in the INET tree; the description of the pull request (not in git). | settled, user 2026-11-12 |
| D4 | The forms of the attribution | The six values of section 3. No value for "the change does not reduce". A maintainer who accepts such a case merges with a red check. | settled, user 2026-11-12 |
| D5 | The size limits | Two limits of changed source lines, for the cause commit of the `commit` form and for a reduction. Above 100 lines the check fails (user). Above 30 lines the gate gives a note, and a note never changes the exit status. The user asked for a note limit, and agreed to the value 30 of this plan. Of the 70 commits of F13, 50 are at or below 30 lines (71 %), 11 are from 31 to 100 (16 %), and 9 are above 100 (13 %). | settled, user 2026-11-12 |
| D6 | Baseline commits for one cause commit | At most one in each baseline repository. | settled, user 2026-11-12 |
| D7 | Start of V1 and V2 on pull requests | After the catch-up makes `master` green. The pins and the gate of B1 to B5 start before. Rejected for now: a comparison with the last run of `master`, so that only new failures count. That method hides a move of a test that is already red. | settled, user 2026-11-12 |
| D8 | Open pull requests of other authors | No exception for the date. The sweep posts a commit status. A comment on each red pull request, with a link to the guide, needs the approval of the user before it is sent. | open |
| D9 | The move of the baseline masters | A job on each push to INET `master`, with a deploy key for each baseline repository. The user must make the keys. Alternative: the maintainer pushes by hand, and the job only checks. | open |
| D10 | The source of the candidates | The artifact of the CI run, or the local copy of the CI job. Values from a developer machine are not permitted. | settled, user 2026-11-12 |
| D11 | Required checks on GitHub | After the sweep: the two tests, the attribution check and `Check: pull request` become required for a merge. The user sets this in the repository. | open |

## Limits

- V1 tests the head. V2 tests only the cause commits, and only their moved tests. A commit in the middle of a series can move a result that a later commit moves back. Only a complete run for each commit finds this (`TR-CI-EVERY-COMMIT`, not in this plan).
- The pins fix the baselines. The head of `omnetpp-6.x`, the head of `opp_repl@github_ci`, the Python packages and the runner image stay inputs that move. Such a change can turn `master` red with no INET commit. The repair is a re-record with the `environment` attribution.
- A reduction is sufficient. No check proves that a smaller one does not exist.
- The size limits count lines. Whether a change is easy to understand stays a decision of the reviewer.
- The statistical test reads run 0 only. A chart shows all runs, but only for the showcases.
- A `pull_request` workflow comes from the merge ref, so a pull request can change the test workflow. The trusted gate and the push run on `master` stay as the control.
- GitHub does not start the workflows of a first contribution from a fork until a maintainer approves the run.
- Each pair of pull requests that move a pin conflicts, also when their baseline files are different. The user accepts this (D1), because such a pair usually conflicts in the baseline files also.

## Permissions

- The rule documents have the seal value `by rule`, and no unit that this plan changes has a `🔒` flag (F17). A rule change can make a seal stale (`SR-RULE-CHANGE-STALES`); step C3 runs `check-seals.sh`.
- This plan changes no baseline value. The pilot gives the present baseline commits of #1452 and #1443 a new message; their values need the approval of `TR-BASELINE-DELIBERATE` as before.
- Four actions each need the approval of the user at that time: a push to a repository, a comment on a pull request, a deploy key, and a required check.
