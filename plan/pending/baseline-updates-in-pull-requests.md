# Baseline updates in pull requests

**Status:** in progress. Written 2026-11-12 from `master` at `7adc565ce9` (equal to `origin/master`). The user settled D1 to D7 and D10 on 2026-11-12. D8, D9 and D11 are open; their steps come later. Approval: on 2026-11-12 the user approved the start of the steps O1, O3, C1 and C2. On 2026-11-13 the user approved the steps that do not depend on the catch-up: C3 to C6, O2 and the pilot. On 2026-11-13 the user also settled D12 and approved a copy of the CI job for the attribution run. On 2026-11-13 the user approved the rest of the steps ("Ok, continue"). C7 to C9 can land only after the catch-up makes `master` green, so they are on the stacked branch `topic/baseline-tests-on-pull-requests`. On 2026-11-13 the user asked to land the first branch and to delay the deploy keys; the first branch is on `master`. The same day the user asked to land the stacked branch too, before `master` is green (D7 revised).

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
  tests: 141 changed, 0 added, 0 removed
  rows of the changed tests: 138836 changed, 9 added, 9 removed
  folders: examples/wireless 54, showcases/wireless 25, ...
  groups of the changed tests, by the names of their moved rows:
  - 95 test(s): interference cache hit, reception cache hit, ...
  - 44 test(s): more than 12 names each
  - 1 test(s): reception decision cache hit
  names of the moved rows:
  - queueLength:timeavg: 908 changed, -100 % to +3.34e+04 %
  - ...

INET-Subject: physicallayer: change: decide the header and the data of a reception separately
Attribution: commit
```

- **Summary.** A tool makes the `Moved results:` block from the difference of the two baseline commits. The gate computes the block again and compares. Thus the summary is always true. The block puts the tests whose moved rows have the same names into one group, because tests that move in different ways usually need different explanations (a result of the pilot). For `media`, the block lists each changed, added and removed chart.
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
| O1 | opp_repl | test: check out the pinned commit of a baseline repository | none | New command: clone or fetch, check out `ref` by SHA, print the commit. The two tests log the baseline commit and give a warning for a checkout that is not at the pin. Self-tests with two local repositories. |
| O2 | INET | O2a: doc: add: the attribution run of a baseline change. O2b: doc: change: the rules and guides name the attribution run | C5a | `check-baseline-attribution.sh` and `baseline_attribution.py`, with unit tests on small repositories and fake commands; the pilot runs in a copy of the CI job. |
| O3 | opp_ci | executor: fetch the pinned baseline commit by its SHA | none | Today the code runs `git fetch origin`, which does not get a commit that only a pull request holds. Check with a pin on a pull request head. |
| C1 | INET | plan: add: baseline updates in pull requests | none | — |
| C2 | INET | build: change: pin the commits of the statistics and media repositories | O1 | `inet.opp` and the two workflows. The push run on `master` prints the two pins and gives the failures of the run before it. |
| C3 | INET | doc: add: rules for a baseline in another repository | C5a | The three rule documents of section 6. `check-seals.sh` and `check-links.sh` pass. |
| C4 | INET | doc: change: procedure for a statistical or chart baseline | C3 | The guides of section 6. `check-links.sh` passes. |
| C5 | INET | C5a: doc: add: gate for the baseline pins and records. C5b: doc: change: the classification gate knows the chart obligation and a pin that moves. C5c: doc: change: the pull-request audit runs the baseline gate | C2; C5b and C5c after C3 | `check-baselines.sh`, `check_baselines.py`, `baseline_summary.py`, `baseline_pins.py`, `test_check_baselines.py`; the `chart` obligation and the pin in `check-classification.sh`; the gate in `check_pull_request.py`. Unit tests with small repositories: one case that passes and one that fails for each of B1 to B5. |
| C6 | INET | doc: add: write the new baseline pins of a series after a rebase | C5a | `baseline_pins.py rebase`, with unit tests on small repositories. |
| C7 | INET | build: add: the statistical and chart tests on pull requests | C2, green `master` | A job selects the paths of the pull request from the GitHub API; `_scripts/github/baseline-candidates.py` makes the candidates after a failure; unit tests with a fake update command; a run of the script in the copy of the CI job. |
| C8 | INET | build: add: the attribution run on pull requests | O2, C7 | `check-baseline-attribution.yml`; the run scripts pass `bash -n`. |
| C9 | INET | build: add: test one open pull request by hand and post its status | C7 | An input `pull-request` of the two test workflows and a status job; the loop over the open pull requests is in the enforcement README. |
| C10 | INET | build: add: move each baseline master to its pin after a push | C2 | `check-baseline-masters.yml` runs `baseline_masters.py` after each push to `master`; unit tests with local bare repositories. D9 stays open: the deploy keys. Until they exist, the job fails after a push that moves a pin and gives the command. |
| S1, S2, C10a | statistics, media, INET | doc: describe the repository and how INET pins it (in each baseline repository); build: change: pin the baseline commits that add the README files | C4, D12 | `check-baselines.sh` with local clones: the pins move over commits that change no result, with no record. |
| C11 | INET | plan: record the results, then move the plan to `plan/done` | all | Two commits (`PR-SPLIT-MOVE`). |

C1 to C6, O1 to O3 and the pilot do not depend on the catch-up. C7 to C9 wait for a green `master`.

**Negative checks on the pilot.** Each of these changes must turn the named check red:

- remove the pin move (V1);
- remove the `Attribution:` trailer (B4);
- change one line of the reduction (V2);
- add a baseline file whose values did not move (B4);
- pin a commit that is only local (B3).

## Progress

All branches have the name `topic/baseline-updates-in-pull-requests`. All branches are local, and nothing is pushed.

- [x] O1: opp_repl `9f11f80`, 2026-11-12.
- [x] O3: opp_ci `09bb0b0`, 2026-11-12.
- [x] C1: INET `b2f26dc036`, 2026-11-12.
- [x] C2: INET `878a1f28a7`, 2026-11-12.
- [x] C5a: INET `01035cb42b`, 2026-11-13.
- [x] C3: INET `9788533835`, 2026-11-13.
- [x] C5b: INET `f79688dc83`, 2026-11-13.
- [x] C5c: INET `0ecef6c178`, 2026-11-13.
- [x] C6: INET `f97ff1f7ed`, 2026-11-13.
- [x] C4: INET `59a8ebff3f`, 2026-11-13.
- [x] Pilot, part 1: the gate on a copy of #1452, 2026-11-13. The branches `topic/baseline-pilot-pr1452` are in INET, `statistics` and `media`.
- [x] O1b: opp_repl `155ac50`, 2026-11-13.
- [x] D12: INET `0a6d0310bd`, 2026-11-13.
- [x] O2a: INET `4019646d74`, 2026-11-13.
- [x] O2b: INET `09a147878c`, 2026-11-13.
- [x] Pilot, part 2: the attribution runs, 2026-11-13.
- [x] S1: `statistics` `f76ce96`, S2: `media` `6e835bc`, on the local branches `topic/baseline-updates-in-pull-requests`, 2026-11-13.
- [x] C10a: INET `7087b297d6`, 2026-11-13.
- [x] C10: INET `67731d2e46`, 2026-11-13.
- [x] Fix: INET `55f297abc5`, the chart images when git cannot read the work tree, 2026-11-13.
- [x] C7: INET `ff1d003a01`, C8: `5487bdb77f`, C9: `e8f3b14038`, on the stacked branch `topic/baseline-tests-on-pull-requests`, 2026-11-13.
- [x] Landing steps 1 to 5, 2026-11-13: opp_repl `155ac50` on `main`, `github_ci` and `opp_ci`; opp_ci `09bb0b0` on `main`, worker restarted; `statistics` `f76ce96` and `media` `6e835bc` on `master`; INET `master` at `55f297abc5`.
- [x] Landing step 6, first part: the stacked branch on `master`, 2026-11-13 (D7 revised).
- [ ] Landing step 6, rest: the sweep of C9, when `master` is green; then the required checks (D11).
- [ ] Landing step 7 (C11).
- [ ] C11, after the landing.

## Results of the implementation

- **O1.** The command is `opp_checkout_test_baselines [KIND ...]`. The Python functions are `checkout_test_baselines` and `checkout_test_baseline`. For a full hash, the command fetches only when the clone does not have the commit. A checkout that is at the pin stays on its branch. git stops a checkout that would overwrite a local change, and the command never uses force. The tests do not print the baseline commit, which the first text of this plan said. They log the commit at the `INFO` level, and they give a warning when the checkout is not at the pin or does not exist. The CI log gets the commit from the command. The step adds 17 self tests; the complete self tests give 94 tests with the result OK.
- **O3.** The two checkout paths of opp_ci fetch a full hash by the hash. The step also repairs a second defect: with a branch name as `ref`, a clone that the worker used again stayed at its first commit. Without the change, the new tests of the pull request head and of the moved branch fail. Four other tests of opp_ci fail on the base commit too (`test_remote_cli`, `test_worker_log_shipping`); this step did not touch them.
- **C2.** The subject has the area `build`, as the earlier workflow commits have. Check with the real project: the command cloned `media` from GitHub and checked out the pin in 19 s. On 2026-11-12 the two pins are equal to the heads of the two baseline masters. The series gates pass for C1 and C2.
- **Not verified:** a run of the two changed workflows on GitHub. That run is possible only after the push.
- **Order for the push.** The INET workflows install `opp_repl@github_ci`. Thus O1 must be on `github_ci` before C2 is on INET `master`, or the two workflows stop at an unknown command. The three opp_repl branches (`main`, `github_ci`, `opp_ci`) stay at the same commit. The opp_ci service gets O3 with a push and a restart.

### The second group of steps, 2026-11-13

- **Order and names.** The gate commit C5a comes before the rules C3, because the rule documents link to the gate files, and a link must not point to a file of a later commit. Two small commits then turn the gate on: C5b for the classification gate, C5c for the pull-request audit. The subjects have the area `doc`, as all files of `doc/project/enforcement` have.
- **C5.** Three tools: `baseline_pins.py` reads the pins of a commit with the `ast` module and never runs `inet.opp`; `baseline_summary.py` reads `.sca` files and computes the block; `check_baselines.py` is the gate. The gate takes the repository of a baseline from the base of the range, so a series cannot send the gate to another repository. With `--baseline-repository` it reads a local clone, and it says that such a run does not prove B3. The gate prints the record of each baseline commit for the reviewer.
- **C3.** Three new rules: `TR-BASELINE-PIN`, `TR-BASELINE-PARENT`, `TR-BASELINE-ATTRIBUTION`. The rule for the source of the candidates (D10) is a paragraph of `TR-BASELINE-DELIBERATE`. The index of the seal list counts the new rules.
- **C6.** The command makes each commit again with git objects only. It replaces the pins of the parent and of the commit by the pins of the new head before the merge; thus a pin gives no conflict. A subagent wrote the tests and found one defect: each new message got one more empty line. The command now copies the raw message.
- **Checks.** 291 unit tests of the enforcement folder pass; 97 of them are new. A deliberate break of the gate in five places and of the rebase command in four places made the tests fail each time. The audit `check-pull-request.sh --base master` passes for the 10 commits of the branch, with 4 notes. `check-seals.sh` passes. `check-links.sh` finds no broken link in the rule, guide and enforcement documents.
- **Not changed:** `design/test-anatomy.md` does not have the chart test as a category. This plan did not add it.

### The pilot, part 1

The pilot is a copy of #1452 on the branch `topic/baseline-pilot-pr1452`: this branch, the five commits of #1452, and the two pins in the cause commit. The baseline commits have the values of the old topic branches and a new message in the record format. All pilot branches are local.

- **The form `commit` fits #1452.** The cause commit changes 79 source lines in 2 files, so the gate gives a note and no failure. The two commits before it prepare (155 and 120 source lines) and move no pin. This is the series of `PR-SPLIT-PREPARE`.
- **D6 needed a squash.** #1452 had two `statistics` commits for one cause commit. The pilot has one, with two groups in its analysis.
- **The first block hid a group.** The 490 rows of the cache counters were not in the 20 most frequent names of 138,836 rows. The block now has the groups of tests, and the gate gives a note when the tests move in more than one way.
- **The groups found two faults of the records.** My first pilot text said that `showcases/wireless/throughput` does not move; the group of one test shows that one of its rows moves. And `examples/communicationcache` `ReferenceCommunicationCache` computes 27 % more receptions and 20 % more interferences; no message of #1452 explains that move. This is an open point of #1452, not of this plan.
- **The charts have a move that the old message did not name.** In `showcases/wireless/coexistence`, the bar "Wifi + WPAN coexistence" goes down by 4.7 % for Wifi and by 6.8 % for WPAN. The release note of the cause commit gives the reason: the showcase uses the "mean" SNIR mode. The old message of the `media` commit said only that the random trajectory changes. The rule to say how a picture changes found this.
- **A changed baseline message moves the pin.** The pilot needed a new series two times by hand, and one of them went wrong. With `baseline_pins.py rebase`, one command moved the five commits to a new base and wrote the new pin. The result is equal to the old series apart from the pin line.
- **Results of the gate.** With the local clones, the gate passes the pilot in 3 s, with three notes. Against GitHub, the gate fails the pilot in 5 s, as it must: the two baseline commits are only local.
- **Part 2 is below.**

### The third group of steps, 2026-11-13

- **D12.** Of the 1,074 files of `media`, 219 are chart images: the charts of the analysis files export them, and each one exists in `media`. `baseline_summary.py` finds them from the `.anf` files and the `doc/media` links of an INET commit. The gate uses the charts of the cause commit and of its parent, so a chart that the commit removes is a result too. A pin moves forward over one or more commits with no merge, and at most one of them changes results. The check of `CR-OBL-INERT` for a pin moved from the classification gate to the baseline gate, because only the baseline gate knows if a pin passes a result. The rebase command gives a pin move as many new commits as it had before.
- **O2 is in INET, not in opp_repl.** The run needs the record format, the result filter and the code of the gate. opp_repl gives only the commands that the run calls. The build, checkout and test commands are options, so the same tool runs in CI and in a copy of the CI job.
- **O1b.** A container saw a baseline checkout that git could not read (a worktree whose repository is outside the container). The baseline report of the tests stopped the test with an error. It now gives a warning, and the test runs.
- **Tests.** 356 unit tests of the enforcement folder pass. Subagents wrote the new tests; a deliberate break of the new code in ten places made the tests fail each time.
- **The job copy** is `~/workspace/ghci-baseline-updates-in-pull-requests`. It takes the built OMNeT++ (`66ab32fb91`), the Python environment and the compiler cache of `ghci-statistical`, and it has its own clones of `statistics` and `media`. The container gets 12 cores, and opp_repl comes from its worktree on this branch. An incremental INET build needs 6 to 45 s.

### The pilot, part 2

The runs are in the job copy, with the tool of O2a. The logs are in `evidence/attribution-*` of the job copy.

| Variant | Attribution | Runs | Result | Time |
| --- | --- | --- | --- | --- |
| `topic/baseline-pilot-pr1452` | `commit` | parent, commit | PASS: 141 statistical tests and 11 charts in each run | 142 s |
| `topic/baseline-pilot-pr1452-reverse` | `reverse`, the complete revert of the source change (79 lines) | parent, commit, reverse reduction | PASS in the three runs | 265 s |
| `topic/baseline-pilot-pr1452-false` | `reverse`, a patch that only adds a comment line | parent, commit, reverse reduction | FAIL in the reduction run: 141 statistical tests and 6 of 11 charts fail | 285 s |

- **The run finds what the gate cannot see.** The gate passed the false variant: the patch applies and is small. Only the run shows that the patch does not give the old results.
- **The tests of a run are counted.** A cross-check by hand gave the same counts: at the cause commit with the old pins, the 141 statistical tests and 6 charts fail; with the new pins they pass.
- **Limit.** The reverse reduction of the pilot is the complete revert, so it proves the mechanism and not a small cause. #1452 has the form `commit` (79 source lines, a note), and that form needs no reduction.
- **Charts.** The chart test runs all the charts of a folder, so a chart run can select more charts than the baseline commit moves. The tool accepts more charts, but not fewer. A failed chart leaves its `-new.png` and `-diff.png` in the `media` checkout; the next checkout is not stopped by them.

### The fourth group of steps, 2026-11-13

- **The README files go in by a pin.** `master` of a baseline repository is always the pin of INET `master`, so a commit of a baseline repository reaches `master` only when an INET commit pins it. C10a moves the two pins over the README commits. Under D12, the two commits change no result and need no record.
- **C10 never forces a push.** The first text of the tool used `--force-with-lease`, which can force; it now uses a plain push, and a unit test checks that the push refuses a move that is not a fast-forward. The workflow pushes only on `master`; a run by hand on another branch only checks.
- **Do not merge a baseline pull request with the GitHub button.** A merge commit or a rebase makes a commit that no INET commit pins. The job moves `master` to the pin, and GitHub then shows the pull request as merged. The guide says so.
- **Against GitHub**, `baseline_masters.py` passes INET `878a1f28a7` (both masters at their pins) and fails this branch, because the README commits are not pushed. For the same reason, `check-baselines.sh` without local clones fails this branch: the gate does what it must.
- **The catch-up lands as one pin move** (decision D5 of that plan). If that pin is not a descendant of the baseline masters, this job fails, and the masters move by hand.

### The fifth group of steps, 2026-11-13

- **The candidates reproduce the reviewed baseline (D10).** In the copy of the CI job, the cause commit of the pilot ran against its old pins, as a pull request that moves results and no pin. The 141 statistical tests and 6 charts failed, and `baseline-candidates.py` made the candidates. The 141 files have the values of the reviewed baseline commit, and the block is equal to the block of its record; they differ only in the run name, the date and the process number. The 6 images have the same pixels; they differ only in the kernel version in the `Creator` text of the PNG file. The chart test compares pixels, so this difference has no effect.
- **The script copies only result files.** A unit test found that it copied a `.diff` work file when the baseline repository does not ignore it.
- **A container cannot read the INET worktree with git.** The summary tool then found no chart image and called each changed image documentation media. The fix `55f297abc5` walks the folders when git fails.
- **A pull request with no path that can change a result** skips the tests, and the skipped job counts as passed. The path list is in the two workflows.
- **The selection job and the status job of C9** ran here against the GitHub API, read-only, with the push of the status replaced by an echo: #1452 is a pull request to test, #1286 and #1203 have a conflict, a merge that GitHub did not compute in 60 s is an error, and the six result combinations give the right status.
- **Not verified:** no run of the three workflows on GitHub, because they run only after the push. The attribution run builds two or three commits on a GitHub runner, which has about 14 GB of free disk; the chart workflow removes the build folders to free disk, and the attribution run cannot. The first run shows if the disk is sufficient.

### Landing order

1. opp_repl: `9f11f80` and `155ac50` to `main`, `github_ci` and `opp_ci`, which stay at one commit. The INET workflows install `opp_repl@github_ci`, so this comes first.
2. opp_ci: `09bb0b0` to `main`, then a restart of the service.
3. `statistics` and `media`: push the branches `topic/baseline-updates-in-pull-requests` with the README commits.
4. INET: the branch `topic/baseline-updates-in-pull-requests`. Its last commit pins the README commits, so `check-baselines.sh` passes only after step 3.
5. After the INET push, `check-baseline-masters.yml` moves the two baseline masters to the README commits. Without the deploy keys (D9), a maintainer runs the two commands of its log.
6. After the catch-up makes `master` green: the stacked branch `topic/baseline-tests-on-pull-requests`, then the sweep of C9, then the required checks (D11).
7. C11: record the landing, and move the plan to `plan/done`.

### The landing of the first branch, 2026-11-13

- **No remote branch had moved** since the start of the work, so no rebase was necessary. The 95 opp_repl self tests passed on `155ac50` before its push.
- **opp_ci.** The worker `opp_ci-worker@levy` had no running job, and it restarted with a new environment of `uv`.
- **The baseline masters moved by hand**, because the user delayed the deploy keys (D9). The README commits went to the two masters by a fast-forward before the INET push. Thus INET `master` pinned commits that were already on the masters.
- **GitHub, INET `55f297abc5`:** `Check: baseline masters` passes: both masters are at their pins. The statistical run checks out `statistics` at `f76ce96` and the chart run checks out `media` at `6e835bc`, as their logs show. Both give the summary of the run of `7adc565ce9`: 81 unexpected statistical results (72 FAIL, 9 ERROR) and 19 failed charts, and the lists of these tests are equal. The new pins change no result.
- **The session of the catch-up** got a message with these facts: its landing (D5 of that plan) must set the two pins, and its chain must descend from the README commits for a fast-forward of the masters.

## Decisions

| # | Decision | Proposal | State |
| --- | --- | --- | --- |
| D1 | The link between an INET commit and its baselines | A pin in `inet.opp`, moved in the cause commit. Rejected: only a branch with the same name (no link for each commit, a red interval when the pull request lands, R5 fails); git submodules (the same information, but each clone and each present checkout changes); the baselines in the INET tree (1.2 GiB, F7); a hash for each baseline file in the INET tree (conflicts only for the same file, but a new design of the two tests). The user accepts the conflict of the pins: a change that moves a baseline usually conflicts in the baseline files also. | settled, user 2026-11-12 |
| D2 | The place of the pinned commit before the pull request lands | A branch with the name of the INET branch in the upstream baseline repository, or a pull request from a fork to that repository. The approval of the values is the review of that pull request. | settled, user 2026-11-12 |
| D3 | The place of the record | The message of the baseline commit, and the reduction as a file in `attribution/` of that commit. Rejected: a record file in the INET tree; the description of the pull request (not in git). | settled, user 2026-11-12 |
| D4 | The forms of the attribution | The six values of section 3. No value for "the change does not reduce". A maintainer who accepts such a case merges with a red check. | settled, user 2026-11-12 |
| D5 | The size limits | Two limits of changed source lines, for the cause commit of the `commit` form and for a reduction. Above 100 lines the check fails (user). Above 30 lines the gate gives a note, and a note never changes the exit status. The user asked for a note limit, and agreed to the value 30 of this plan. Of the 70 commits of F13, 50 are at or below 30 lines (71 %), 11 are from 31 to 100 (16 %), and 9 are above 100 (13 %). | settled, user 2026-11-12 |
| D6 | Baseline commits for one cause commit | At most one in each baseline repository. | settled, user 2026-11-12 |
| D7 | Start of V1 and V2 on pull requests | After the catch-up makes `master` green. The pins and the gate of B1 to B5 start before. Rejected for now: a comparison with the last run of `master`, so that only new failures count. That method hides a move of a test that is already red. Revised by the user on 2026-11-13: the tests on pull requests land before `master` is green, because no pull request merges until `master` is green. Until then, a pull request that changes a path that can change a result shows the failures of `master` in the two checks, and its candidates hold the files of those tests too; the author leaves them out (TR-BASELINE-PARENT). | settled, user 2026-11-12; revised by the user 2026-11-13 |
| D8 | Open pull requests of other authors | No exception for the date. The sweep posts a commit status. A comment on each red pull request, with a link to the guide, needs the approval of the user before it is sent. | open |
| D9 | The move of the baseline masters | A job on each push to INET `master`, with a deploy key for each baseline repository. The user must make the keys. Alternative: the maintainer pushes by hand, and the job only checks. | open |
| D10 | The source of the candidates | The artifact of the CI run, or the local copy of the CI job. Values from a developer machine are not permitted. | settled, user 2026-11-12 |
| D12 | A commit of a baseline repository that moves no test result | Facts: `media` holds 1,074 files, and the chart test has 229 charts. The other files are documentation media: network images and 179 videos. By their subjects, 7 of the last 14 commits on `media` `master` change documentation media or a repository setting. `statistics` gets a README in step S1. The gate of C5 reads each file of `media` as a chart, and a pin moves only to a direct child with a record. Thus a documentation image needs a record and an attribution today. Proposal: a pin can move over commits that change no result file; at most one commit of the step changes result files, and only that commit needs the record. A result file is a `.sca` file, or an image that a chart of an analysis file in the INET tree exports; the gate reads the chart names from the `.anf` files of the cause commit. Then the `media` pin also names the documentation media that an INET commit goes with. Alternative: keep the gate as it is, and give each `media` commit a record; a new image has the form `first-record`. | settled, user 2026-11-13 |
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
