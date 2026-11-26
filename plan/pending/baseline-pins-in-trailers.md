# Baseline pins in commit trailers

**Status:** done 2026-11-26. Part 1 landed as `fef104126f`, part 2 as `4e768408d2`, and `Baseline merge` is a required check of `master`. Written 2026-11-25 from `master` at `28f1dc534c` (equal to `origin/master`). On 2026-11-25 the user chose this design ("yes write the trailer design as a plan"). The same day the user chose a rebase in the baseline repositories, as INET lands with "Rebase and merge" (T2), and a check that blocks the landing when that rebase would fail (T3, T4). On 2026-11-26 the user chose the mark in a commit of its own, a fast-forward when the steps are on top of baseline `master`, and a close of the other baseline pull requests by a maintainer, with no token (T2, T10). No step has started. Each step needs the approval of the user.

This plan changes the link of the plan `baseline-updates-in-pull-requests.md` (decision D1 there: the pin in `inet.opp`). The other decisions of that plan stay.

## Goal

Two pull requests that move the same baseline land one after the other with the GitHub button "Rebase and merge", with no new push, when their baseline changes have no common file.

Today the second pull request conflicts in the pin line of `inet.opp`. It needs a rebase of its baseline branch, new pins, a push and a new CI run. While the conflict is open, its statistical and chart tests cannot run at all (F12).

The design keeps the properties of the pin that are important: the link is in git, each INET commit has one exact baseline, a bisect finds it, and the record of each baseline change stays as it is.

## Facts

All facts come from the tree at `28f1dc534c`, the local clones of opp_repl and opp_ci, and the GitHub API on 2026-11-25.

| # | Fact | Source |
| --- | --- | --- |
| F1 | When the baseline changes of two series have no common file, the pin line is their only conflict. In `tests/fingerprint/store.json` an entry has 12 lines. Two series that change different entries change lines at least 10 lines apart, and git merges them with no conflict. | `tests/fingerprint/store.json` |
| F2 | Many places use the pin. INET: 4 workflows, 4 Python scripts (`baseline_pins.py`, `baseline_masters.py`, `check_baselines.py`, `baseline_attribution.py`) and 7 test files in `doc/project/enforcement/`, 3 rule documents, 4 guides, `enforcement/README.md`, the pull request template. opp_repl: `test/baseline.py`, `simulation/project.py`, `main.py`, the self-tests, `doc/statistical_tests.md`. opp_ci: `executor.py` and `tests/test_executor_baseline.py`. The section "Changes for each repository" gives the lines. | inventory of 2026-11-25 |
| F3 | GitHub attaches checks and commit statuses to the hash of the head commit. The description and the comments of a pull request are not part of that hash, and they are not in git. After an edit, the checks of the head stay green until a new run ends. | GitHub data model |
| F4 | "Rebase and merge" makes new commits with the same messages. A rebase merges the trees; a commit message never conflicts. The user lands pull requests with this button (F14 of the earlier plan). | git, repository settings |
| F5 | `check-pull-request.yml` already posts commit statuses on the heads of open pull requests, also of pull requests from forks. It has `statuses: write`. | `check-pull-request.yml:163` (the sweep job) |
| F6 | 116 pull requests to `master` are open. 5 of them move a pin: #1488 (statistics and media, fork), #1484 and #1474 (media, upstream branches), #1463 (statistics, upstream branch), #1459 (statistics, fork). #1464 and #1465 change other lines of `inet.opp`. | `gh pr list`, `gh pr diff` |
| F7 | opp_repl reads `ref` in `checkout_test_baseline` (`opp_repl/test/baseline.py:102`). With a full hash it fetches the commit by its hash when the clone does not have it. Without `ref` it takes the head of the default branch. The copy of `inet.opp` that opp_repl bundles for old INET versions has no `ref` (`opp_repl/opp/inet.opp:29, 34`). | the files |
| F8 | opp_ci gets the baseline entry through opp_repl (`executor.py:845`). Then it reads `ref` itself and runs its own git commands, in two places: `_baseline_checkout_shell` (`executor.py:884`, in the container) and `_checkout_baseline_on_host` (`executor.py:906`). | the file |
| F9 | `check-baseline-masters.yml` runs on each push to `master`, in the environment `baseline-masters`, with `concurrency: baseline-masters` and `cancel-in-progress: false` (lines 21-23). The statistical and chart tests of a push to `master` start at the same time. No workflow waits for another one. | the workflow files |
| F10 | `check_baselines.py` fails a commit whose baseline entry has no full-hash `ref` (`check_pins`, line 443). `check-pull-request.yml` uses `pull_request_target`, so the gate of `master` examines each pull request, also a pull request that changes the gate. | the files |
| F11 | `INET-Commit:` already has a meaning in a baseline record: the hash of a cause commit that is on the target branch already, for a re-record. | `rule/testing.md:196` |
| F12 | A pull request run of the statistical and chart tests tests the merge ref `refs/pull/N/merge`. When GitHub reports the pull request as not mergeable, the run stops with an error. Thus a pull request with a pin conflict gets no test result. | `statistical-tests.yml:61-66`, the same in `chart-tests.yml` |
| F13 | Outside the repositories, the local tool `~/workspace/ghci-standards-fixes/tools/check_series.py` reads the pin with a regular expression (line 21) and checks out the statistics at it (line 44). | the file |
| F14 | No rule unit that this plan changes has a `🔒` flag. | `grep` in `testing.md`, `pull-request.md`, `classification.md` |
| F15 | After a push from outside GitHub, GitHub marks a pull request as merged only when its head commit is on the base branch. The pull requests statistics #5-#7 and media #2-#4 come from a fork and show "Merged": their merge commit is their head, because a fast-forward moved baseline `master` to it. The button "Rebase and merge" marks a pull request as merged with no head on the base branch: the head `525f029e38` of INET #1487 is not on `master`, which has the rebased `b2ee64e54f`. | `gh pr list`, `gh pr view`, `git merge-base --is-ancestor`, 2026-11-26 |
| F16 | Only a contributor from a fork opens a pull request in a baseline repository. A member pushes a branch there (D2 of the earlier plan). Nobody merges a baseline pull request with the button. | `guide/change-a-baseline.md:201, 220-222` |
| F17 | statistics and media have no workflow. Actions are enabled in statistics. Our token cannot read the Actions setting of media. | GitHub API, 2026-11-26 |

## Definitions

- **Pin**: the full hash of a baseline commit that an INET commit names. Today: the `ref` in `inet.opp`. With this plan: a trailer in the message of the cause commit.
- **Cause commit**: an INET commit that changes a baseline. Its message has a pin trailer.
- **Baseline branch**: the baseline commits of one series. It starts on a commit of baseline `master`, its base.
- **Step**: the baseline commits that one pin adds. A step goes from the pin of the previous cause commit of the series, or from the base of the baseline branch, to the pin.
- **Mark**: a commit on baseline `master` with no change of files, after the steps that one run of the landing job adds. For each cause commit of these steps, its message has the trailer `Baseline-Of: <INET hash> <baseline hash>` (section 3).
- **Baseline of a commit**: the baseline commit that the tests of an INET commit compare against. Section 2 gives the function that finds it.
- **Overlap**: a file that the baseline branch changes and that baseline `master` also changed after the base of the branch.

## Requirements

| # | Requirement |
| --- | --- |
| R1 | Two pull requests whose baseline changes do not overlap land one after the other with the button. The second one needs no new push and no new CI run. |
| R2 | A pull request whose baseline overlaps with baseline `master` has a red required check before a person can land it. |
| R3 | The link is in git: in the INET commit message and in the baseline repository. It is not in a comment or in the description of a pull request (F3). |
| R4 | Each INET commit has exactly one baseline, and a tool finds it from the two git histories. This is true for each commit of `master` and of a branch, also after the commit lands with a rebase. |
| R5 | The tests of INET `master` never run against a baseline that does not hold the changes of the pushed commits. |
| R6 | The record (`TR-BASELINE-PROVENANCE`), the attribution (`TR-BASELINE-ATTRIBUTION`) and the parent rule (`TR-BASELINE-PARENT`) do not change. |
| R7 | Old INET commits with a `ref` keep their exact baseline. |
| R8 | A pull request cannot change the gate that examines it (R8 of the earlier plan). |

## Design

### 1. The pin trailer

The cause commit gets one trailer for each baseline that it moves:

```
Statistical-Baseline: <40 hex digits>
Chart-Baseline: <40 hex digits>
```

The value is the last commit of the step, on the baseline branch of the series. The rules of a step do not change: the step has no merge commit, at most one of its commits changes results, and that commit has the record (`TR-BASELINE-PIN`, D12 of the earlier plan). The pinned commit is in the upstream baseline repository, on a branch with the name of the INET branch or as the head of a pull request there (D2 of the earlier plan). After the landing, the pin names a commit of baseline `master` if the landing job did a fast-forward, or else the commit before the rebase of section 3. A commit before a rebase stays on its branch, or in the ref `refs/pull/N/head` of its pull request.

`inet.opp` names the trailer key instead of a `ref`:

```
"baseline": {"repository": "inet-framework/statistics", "trailer": "Statistical-Baseline", "folder": "statistics"}
```

The `trailer` key is a general feature of opp_repl. An entry has `ref` or `trailer`, never the two. An entry with neither of them takes the head of the default branch, as today (F7).

A commit message is part of the hash, so the required checks cover the pin (F3). "Rebase and merge" keeps the messages, and a message never conflicts (F4).

### 2. The baseline of a commit

One function in opp_repl finds the baseline of the INET commit X for one kind:

1. If `inet.opp` at X has a `ref`, the baseline is that commit (R7).
2. B is the merge base of X and INET `master`. B is X when X is on `master`.
3. A is the newest first-parent ancestor of B, B included, that a mark on baseline `master` names. M is the baseline hash that the mark gives for A. If no mark names an ancestor of B, M is the `ref` of the newest ancestor of B that has a `ref` (section 5).
4. S is the pin of the newest commit in `B..X` that has the trailer. If no commit has one, the baseline is M.
5. Otherwise the baseline is the baseline branch up to S, rebased onto M, as the landing job will rebase it (section 3). The function makes the rebased commits locally, with a fixed committer and date, so their hashes are the same on each machine. The function stops with an error if the rebase conflicts or if the branch overlaps with M. The message names the files.

This one function serves all callers:

- **V1**, a pull request run on the merge ref: B is the head of `master`. The baseline is the baseline branch rebased onto the newest baseline `master`. This is the baseline that the landing will make.
- **V2**, the attribution run at a parent and at a cause commit: B is the base of the pull request. The two baselines differ only by the step of the cause commit.
- **A push run on `master`**: no trailer is in `B..X`, and M is the baseline hash that the mark gives for the newest cause commit.
- **A bisect, opp_ci, a developer machine**: the same function, on any commit.

### 3. The landing job

`check-baseline-masters.yml` keeps its trigger, its environment `baseline-masters`, its deploy keys and its concurrency group (F9). `baseline_masters.py` keeps the fast-forward when a step starts on the head of baseline `master`. Else it does a rebase, as INET lands with "Rebase and merge" (T2). For each kind, the job makes the new baseline `master` on a local work branch that starts on the head of baseline `master`:

1. For each commit X of INET `master` after the newest commit that a mark names, in first-parent order, and for each pin trailer of X: fetch the pin S from the upstream baseline repository. If the step of S starts on the head of the work branch, take the step as it is. Else rebase the step onto that head, and check the overlap. The rebased commits keep their messages and their authors. Note the pair of X and the commit that is now the last of its step.
2. After the last step, add the mark: a commit with no change of files and this message:

   ```
   Baseline of INET <short hash of the last X>: <subject of the last X>

   Baseline-Of: <full hash of X> <full baseline hash>
   Baseline-Pull-Request: #<N>
   ```

   The message has one `Baseline-Of:` line for each pair of step 1. It has one `Baseline-Pull-Request:` line for each pull request of the baseline repository whose head is a pin of a rebased step. The job finds N with `git ls-remote origin 'refs/pull/*/head'`, which needs only git access. One mark after all steps, not one mark after each step: a mark between two steps of one series would stop the fast-forward of the second step.
3. Push baseline `master`. The push is a fast-forward and is never forced.
4. If a conflict or an overlap stops a rebase, the job pushes the steps before X with their mark, and fails. Its log names X, S and the files. A later run tries X again, because no mark names X. A maintainer repairs it by hand: run the moved tests of the common files at X, commit their results on the head of baseline `master` as the step of X, add the mark, and push.
5. After the push, post the status `Baseline merge` on the head of each open pull request that has a pin trailer (section 4).

A baseline pull request closes in one of two ways (T10):

- **Fast-forward.** The head of the pull request is now on baseline `master`, and GitHub marks the pull request as merged by itself (F15). This is the usual case for the first of two pull requests.
- **Rebase.** The head is not on baseline `master`, and the pull request stays open. The log and the job summary of the landing job name it, and a maintainer closes it, with no comment (D8 of the earlier plan). No token and no workflow of the baseline repository are necessary. The `#N` in the mark gives a reference on the timeline of the pull request (to check at the first rebase). The state of the pull request is "Closed", not "Merged", because only the button or a head on the base branch gives "Merged" (F15).

The rebase has the properties of "Rebase and merge" in INET:

- Baseline `master` stays linear, with no merge commit, as `TR-BASELINE-PIN` asks today.
- The rebased commits keep their authors, their changes and their records. `git blame` of a baseline file shows a commit with the record, never a mark, because a mark changes no file.
- After a rebase, the pin in the INET commit names the commit before the rebase (section 1). After the landing, the mark gives the exact baseline of the INET commit (section 2).

The statistical and chart tests of `master` start after the landing job. Their trigger for `master` changes from `push` to a `workflow_run` of the landing job (R5).

### 4. The status `Baseline merge`

The command `baseline_masters.py check <INET commit>` does steps 2 to 5 of section 2, but it keeps no commit. It examines two conditions: the rebase applies with no conflict, and the branch has no common file with baseline `master` since its base (T3). A rebase with no conflict is not sufficient: two pull requests can change different lines of one `.sca` file, and the rebased file then mixes the values of two runs. The command needs git and no simulation, so it is fast. Thus a rebase that would fail at the landing blocks the button before the landing.

- `check-pull-request.yml` posts the status on each event of a pull request: opened, reopened, synchronize, edited.
- The landing job posts the status on each open pull request with a pin trailer, after each move of baseline `master`. This is the only case where a pull request becomes red with no event of its own. The commit status API works on the heads of pull requests from forks (F5).
- `Baseline merge` becomes a required check of `master`. Rudolf Hornig sets the required checks (D11 of the earlier plan).

### 5. The switch

The gate of `master` examines the pull request that changes the gate (R8), and the gate of today fails a commit whose baseline entry has no full-hash `ref` (F10). Thus the switch lands in two parts:

- **Part 1** accepts the two forms. The gates, the landing job and the tools read a `ref` or a trailer. INET `master` keeps its `ref`, and the landing job keeps the fast-forward for a `ref`.
- **Part 2** is the switch commit. `inet.opp` gets the `trailer` keys and loses the `ref`. The rules and the guides change. The switch commit moves no pin, so the first mark comes with the first cause commit after it. Until then, step 3 of section 2 gives the `ref` of the parent of the switch commit, which is the head of baseline `master`.

After part 2, the gates reject a `ref` in a new commit. opp_repl still reads a `ref` in old commits (R7).

### 6. What changes for the author

1. Record the values on your machine (D10 of the earlier plan), on a branch of the baseline repository that starts on the head of baseline `master`. Write the record as today.
2. Add the pin trailer to the cause commit. Do not change `inet.opp`.
3. If a baseline commit changes later (a new record or new values), its hash changes. `baseline_pins.py retrail` writes the new hashes into the trailers of the series, in the order of the steps.
4. After another pull request lands, do nothing, unless `Baseline merge` is red. If it is red, record the common files again on the new head of baseline `master`, then run `retrail`.
5. If your series is from before part 2 and moves a `ref`, run `baseline_pins.py convert`. It removes the `ref` changes from `inet.opp`, adds the pin trailers to the cause commits, and rebases the series onto `master`.

## Changes for each repository

| Repository | Place | Change |
| --- | --- | --- |
| opp_repl | `opp_repl/test/baseline.py:76-181` | `checkout_test_baseline`: the `trailer` form and the function of section 2. `check_test_baseline`: compare with the tree of the baseline of HEAD. |
| opp_repl | `opp_repl/simulation/project.py:273, 611-615` | The documentation of the `baseline` entry: the `trailer` key. |
| opp_repl | `opp_repl/main.py:340-370` | `opp_checkout_test_baselines` prints B, M, S and the result. |
| opp_repl | `opp_repl/test/self/baseline.py`, `doc/statistical_tests.md:52-97` | Self-tests and the documentation of the `trailer` form. |
| opp_ci | `opp_ci/executor.py:878-959, 1738-1750, 2029-2040` | Use the checkout of opp_repl instead of its own git commands, in the container and on the host. Then opp_repl and opp_ci have one function. |
| opp_ci | `tests/test_executor_baseline.py` | Tests of the `trailer` form. |
| INET, part 1 | `doc/project/enforcement/baseline_pins.py` | Read the pin trailers. New `retrail` (section 6). New `convert`: change a series that moves a `ref` into a series with trailers (step P7). `rebase` stays for a `ref` until part 2. |
| INET, part 1 | `doc/project/enforcement/check_baselines.py:420-450, 512-546, 583-621` | `check_pins`, the step check and `check_attribution` take the pin trailer as the unit. A trailer only in a commit that changes source (as today for a pin). New section: the overlap. |
| INET, part 1 | `doc/project/enforcement/baseline_masters.py` | The fast-forward or the rebase, and the mark of section 3. The command `check` of section 4. The gates of the baseline commits accept a mark: no change of files, only mark trailers. |
| INET, part 1 | `doc/project/enforcement/baseline_attribution.py:128-170, 188-222` | `find_steps` and `add_fingerprints` read the pin trailers. The checkout gets the baselines from the function of section 2. |
| INET, part 1 | `.github/workflows/check-baseline-masters.yml`, `check-pull-request.yml` | The status `Baseline merge`. |
| INET, part 1 | `.github/workflows/statistical-tests.yml`, `chart-tests.yml` | On `master`: `workflow_run` of the landing job. The path filter (lines 74-75) also reacts to a pin trailer. |
| INET, part 1 | `doc/project/enforcement/test_*.py` | The fixtures of `test_baseline_pins`, `test_check_baselines`, `test_baseline_masters`, `test_baseline_attribution`, `test_baseline_description`, `test_fingerprint_attribution` and `test_check_classification` write trailers, and the two forms are tested. |
| INET, part 2 | `inet.opp:17-47` | The `trailer` keys, no `ref`, and a new comment. |
| INET, part 2 | `doc/project/rule/testing.md:196, 210, 214-228, 230-254` | `TR-BASELINE-PIN`: the pin trailer, the landing job, the overlap instead of the conflict in `inet.opp`. `TR-BASELINE-PROVENANCE`: `INET-Subject:` is the subject of the cause commit. `TR-BASELINE-COMMIT`. `TR-BASELINE-PARENT` and `TR-BASELINE-ATTRIBUTION`: "the old pins" becomes "the baseline of the parent". |
| INET, part 2 | `doc/project/rule/pull-request.md:191, 447, 561`, `doc/project/rule/classification.md:201-246` | "A pin that moves over results" becomes "a pin trailer whose step changes results". |
| INET, part 2 | `doc/project/guide/change-a-baseline.md:106-228`, `run-the-gates.md:35, 49-50`, `review-a-pull-request.md:120`, `contribute-a-change.md:45` | The procedure of section 6 with `convert`, the landing job, the repair of section 3. |
| INET, part 2 | `doc/project/enforcement/README.md:56-58, 89-96`, `.github/pull_request_template.md:16` | The rows and the commands. |
| statistics, media | `README.md` | The fast-forward or the rebase by the landing job, the mark, and the close of the pull requests by a maintainer. |
| outside | `~/workspace/ghci-standards-fixes/tools/check_series.py` (F13) | Use `opp_checkout_test_baselines` instead of the regular expression, when the tool is used again. |

## Steps

| # | Step | Approval at that time |
| --- | --- | --- |
| P1 | opp_repl: the `trailer` form, the function of section 2, the self-tests, the documentation. The self-tests use fixture repositories of INET and of a baseline: a commit on `master`, a branch, a merge ref, the switch, an overlap, a conflict. Commit on `main`, then on `github_ci` and `opp_ci`. | the push |
| P2 | opp_ci: the checkout through opp_repl, and its tests. | the push and the restart of the worker |
| P3 | INET part 1, on the branch `topic/baseline-pins-in-trailers` in a worktree: the tools, the gates, the landing job, the status, the workflows and the tests. One commit for each tool. | — |
| P4 | End-to-end test on local bare copies of INET, statistics and media. Two series with no overlap land one after the other, and the landing job makes their marks: the first one by a fast-forward, the second one by a rebase. A series with two steps of one kind lands by a fast-forward. A series with an overlap gets a red status, and the landing job stops for it. A bisect over the switch finds the baseline of each commit. Then a run of the landing job against the real repositories with no `--push`. | — |
| P5 | Land part 1. Check the first run of the landing job on `master`: it sees the `ref` and does the fast-forward as today. | the push |
| P6 | INET part 2: the switch commit, the rules, the guides, `enforcement/README.md`, the template. Run `check-seals.sh` (`SR-RULE-CHANGE-STALES`). Land part 2. Then push the README to statistics and media (I12). At the first rebase, check that the job summary names the pull request, and that the `#N` of a mark gives a reference on it. | each push |
| P7 | Try `baseline_pins.py convert` on local copies of the 5 open pull requests that move a pin (F6), with no push. Repair `convert` if it fails. The owners convert their pull requests (decision T8). | — |
| P8 | The required check `Baseline merge` (Rudolf Hornig). Record the results in this plan, and move it to `plan/done/`. | the required check |

## Progress

- [x] P1 opp_repl: PUSHED 2026-11-26 as `b3ea94e` on `main` and `github_ci`, and as `175852a` on `opp_ci` (a cherry-pick of `34c163d`: local `main` holds unpushed commits of the test selection, which stay unpushed). The self-tests pass on each base.
- [x] P2 opp_ci: PUSHED 2026-11-26 as `740d6fc` on `main`, and the worker `levy` restarted while idle. 476 tests pass; 4 tests fail on `main` too (I5).
- [x] P3 INET part 1: 5 commits on `topic/baseline-pins-in-trailers`, in the worktree `inet-baseline-pins-in-trailers`. 562 enforcement tests pass, and `check-pull-request.sh` passes.
- [x] P4 end-to-end test: 24 checks pass on 2026-11-26 (I11). The landing job against GitHub, with no `--push`, passes at `origin/master` `aa713a1ea8`: both masters are at their pins.
- [x] P5 land part 1: LANDED 2026-11-26 as `fef104126f` on `e74ac35bbf`. The first run of the landing job passed: it read the `ref` form, and both masters were at their pins (`0efeed3731`, `838086b3f9`). The statistical and chart tests of `fef104126f` started by `workflow_run` after it, and passed. The module tests fail on `master` since `d88ae6c573` (4 IEEE 802.11 tests), before part 1.
- [x] P6 INET part 2 and its landing: LANDED 2026-11-26 as `4e768408d2` on `ffe07d6437` (I14). The landing job passed with the trailer form and had nothing to land. The statistical and chart tests of `4e768408d2` passed; their checkout gave M = the old refs `0efeed3731` and `838086b3f9`, because no mark exists yet. The README commits are pushed: statistics `17be4498`, media `ad4b9b0`.
- [x] P7 `convert` tried on local copies of the 5 open pull requests on 2026-11-26 (I13). It works on all 5. 3 of them need a rebase of their baseline branch, which the owners do.
- [x] P8 required check, record, plan to done: on 2026-11-26 the user, now admin, let me add `Baseline merge` to the ruleset "master: required checks" (id 28155693), beside the four checks of D11. It has no `integration_id`, because the status is a commit status that a workflow posts with `GITHUB_TOKEN`, and the app that GitHub records for it was not certain. The bypass list stays: organization admins, and the repository roles maintain and admin. #1494, the first pull request with an event after part 1, has the status "success: no baseline change". The other open pull requests get the status with their next event.

## Implementation notes

What the implementation found and decided, in the order of the steps.

| # | Note |
| --- | --- |
| I1 | opp_repl: `resolve_test_baseline` is the function of section 2, and `TestBaseline` holds B, A, M, S and the result. The local rebase keeps the author, the committer, the dates and the message of each commit, so its hashes are the same on each machine; a fixed committer is not necessary. |
| I2 | opp_repl: a baseline branch usually starts on the mark after M, not on M. The mark changes no file, so the function compares the trees of the base and of M: equal trees need no rebase, and the result is the pin itself. |
| I3 | opp_repl: the target branch is the first of `origin/HEAD`, `origin/master`, `origin/main`, `master`, `main`, or `--target`. A cause commit on the target branch with no mark is an error (R5): the landing job did not land it yet, or it failed. |
| I4 | opp_repl: `opp_checkout_test_baselines --commit COMMIT` checks out the baseline of another commit than `HEAD`. The attribution run needs it for a reduction run, whose tree is a parent with a patch and whose expectations are those of the cause commit. |
| I5 | opp_ci: the trailer form goes through opp_repl: on the host, `checkout_test_baseline` in the process; in the container, `opp_checkout_test_baselines` with the project discovery of the build stage. The ref form keeps the git commands of opp_ci, which the workers use now. This is less than the plan row asked ("one function"), and it changes nothing for the ref form. The tests `test_worker_log_shipping` (3) and `test_remote_cli::test_seed_endpoints` fail on `main` too. |
| I6 | INET: `check_baselines.py` does not check the overlap. The overlap changes when another pull request lands, so only the status `Baseline merge` reports it. The plan row "New section: the overlap" is not done for this reason. |
| I7 | INET: the landing job computes the overlap of a step as the files of `base..pin` that also differ between the landed copy of the base and the head of the work branch. For a step on a base that the run rebased, the landed copy is the rebased commit. |
| I8 | INET: the rebased commits and the mark have the committer `github-actions[bot]`; the rebased commits keep their authors. |
| I9 | INET: the pull request audit posts `Baseline merge` in a new job `baseline-merge`, and the run by hand (`sweep`) posts it on each selected pull request. The landing job posts it only on the open pull requests with a pin trailer; the others keep their status. Before the status becomes required (P8), a run by hand with `open` gives each open pull request the status. |
| I10 | INET: the checkouts of the test workflows and of the landing job have `fetch-depth: 0` and `filter: blob:none`: the history of all commits, and a file content only when a tool reads it. |
| I11 | P4: bare copies of INET (the part 1 branch), statistics and media in `/var/tmp/claude-e2e-baseline-pins`, with the baseline masters at the refs of the branch; the driver script is `e2e.py` in the scratchpad of the session. A switch commit makes part 2. Three series start on it: A (one `.sca` file), B (another `.sca` file and one image) and C (the file of A). Results: the landing job after the switch has nothing to land; the baseline after the switch is the old ref; the three statuses are green before a landing; A lands by a fast-forward with a mark; then B stays green and C is red with the file of A; the merge ref of B gets B rebased onto A, with the same files as the later landing; B lands by a rebase in statistics (the mark names pull request #201) and by a fast-forward in media (no pull request line); the landing of C stops at the overlap, and the baseline of C on master is an error; a bisect over the switch gets the baseline of each commit. |
| I12 | The README of statistics and media is a commit on their `master`, which no INET commit pins. In the ref form, the landing job fails for a master that is ahead of its pin. Thus it lands after part 2, as commit `6de3d3bb` (statistics) and `2575021` (media) on the branch `topic/baseline-pins-in-trailers-readme`, in the worktrees `statistics-baseline-pins-in-trailers-readme` and `media-baseline-pins-in-trailers-readme` of the clones in `inet/statistics` and `inet/media`. After T10 changed, these commits have no workflow. |
| I13 | P7, on a copy of INET in `/var/tmp/claude-p7-convert` with a switch commit on `origin/master` `aa713a1ea8`: `convert` makes each series again on the switch with no conflict; `inet.opp` of each new series is the one of the switch, and each cause commit has its pin trailer. Then the gate and `Baseline merge` against GitHub: #1488 passes (its pins are on master already); #1484 passes (1 commit applies). #1474, #1463 and #1459 fail: their old refs (media `83ffedd1e8`, statistics `f76ce96731`) are not on master of the baseline repositories, so their baseline branches stand on another line, and the step from the merge base with master takes in other commits (3 and 25 with results) and overlaps. In the ref form, the landing job would stop at these pull requests with "different lines". The owners rebase those baseline branches onto master and record again. The gate message of such a step now names its start and the repair. |
| I14 | P6 is prepared on the branch `topic/baseline-pins-in-trailers-switch` (worktree `inet-baseline-pins-in-trailers-switch`), on top of part 1: the switch commit (`inet.opp`), then the rules, the guides, `enforcement/README.md` and the pull request template, one commit for each. `check-seals.sh` passes (20 documents), `check-pull-request.sh` passes over part 1 and part 2, and the 562 enforcement tests pass. The bold statement of `TR-BASELINE-PIN` is now "A commit that moves a result pins its baseline commit in a pin trailer"; the rule ID stays. The rule says that a red `Baseline merge` blocks the landing, which is true after P8. After a repair by hand, the guide tells the maintainer to start the landing workflow by hand: nothing else starts it before the next push. |

## Decisions

| # | Decision | Proposal | State |
| --- | --- | --- | --- |
| T1 | The place of the pin | A trailer in the message of the cause commit. Rejected: a comment or the description of the pull request (not in the hash and not in git, F3); the `ref` in `inet.opp` (the conflict of the goal). This decision replaces D1 of the earlier plan. | settled, user 2026-11-25 |
| T2 | How a baseline lands on baseline `master` | A fast-forward when a step starts on the head of baseline `master`, else a rebase, as INET lands with "Rebase and merge". After the steps of one run, a mark: a commit with no change of files. Rejected: the mark in the last commit of a step (its message changes, so the head of a baseline pull request never reaches `master`, F15); a mark after each step (the second step of a series then needs a rebase); a merge commit for each cause commit (baseline `master` then gets merge commits, against `TR-BASELINE-PIN`, and the two repositories land in different ways). | settled, user 2026-11-25, changed 2026-11-26 |
| T3 | The overlap | A common file since the base of the baseline branch. This is stricter than a rebase with no conflict: two changes of the same result file mean that one simulation moved two times. | settled, user 2026-11-25 |
| T4 | The check before the landing | The status `Baseline merge`: the rebase applies and has no overlap. The pull request audit and the landing job post it, and it is required. If the rebase would fail, the landing is blocked. | settled, user 2026-11-25 |
| T5 | The order of the jobs on `master` | The statistical and chart tests start with a `workflow_run` of the landing job. Rejected: a test that waits until the mark is there. | proposal |
| T6 | The form in `inet.opp` | The key `trailer`, a general feature of opp_repl. `ref` stays valid for old commits and for other projects. | proposal |
| T7 | The names | Trailers `Statistical-Baseline:` and `Chart-Baseline:`. In a mark: `Baseline-Of:` and `Baseline-Pull-Request:`. `INET-Commit:` keeps its meaning (F11). The rule ID `TR-BASELINE-PIN` and the names of the scripts stay; their texts change. | proposal |
| T8 | The 5 open pull requests that move a pin (F6) | The owner of each pull request converts it with `baseline_pins.py convert` (item 5 of section 6). This plan does not push to these pull requests and does not comment on them (D8 of the earlier plan). After part 2, such a pull request has a conflict with the switch commit in `inet.opp`, and the gate rejects its `ref`. Thus the owner sees the need to convert. | settled, user 2026-11-25 |
| T9 | A failed landing job | A maintainer repairs it by hand, as in step 3 of section 3. | proposal |
| T10 | The close of the baseline pull requests | After a fast-forward, GitHub marks the pull request as merged by itself (F15). After a rebase, the log and the job summary of the landing job name the pull request, and a maintainer closes it. Rejected: a GitHub App or a token with write access to `statistics` and `media` in INET; a workflow of the baseline repository with its own `GITHUB_TOKEN` (settled first on 2026-11-26, then rejected by the user the same day: "no token, a maintainer closes them"). | settled, user 2026-11-26 |
| T11 | The gap between a landing and the new statuses (Limits) | No merge queue now. GitHub's merge queue lands one pull request at a time and runs the check just before each landing. Add it only if the gap causes a real problem. | proposal |

## Limits

- PR CI tests a pull request against the baseline `master` of the time of its run. A pull request that lands later can interact with it. The tests of `master` find such an interaction after the landing. Baseline changes with no common file are strong evidence, but not a proof. Code lands in the same way today, unless the branch protection of `master` requires up-to-date branches.
- A failure of the landing job leaves `master` with no baseline for its new commit until a maintainer repairs it (T9).
- After the landing job of one pull request, the new statuses need some time. In that time a person can land a second pull request with an old green status. The concurrency group runs the second landing job after the first one, and the second job fails on an overlap. GitHub's merge queue closes this gap (T11).
- The function of section 2 needs the history of INET `master` and of baseline `master`. With old clones, M is older. If no overlap exists, the rebase of the branch onto M still gives a correct baseline.
- The baseline of a commit that is not on `master` is a local rebased commit, which no repository holds. Its hash is the same on each machine (step 5 of section 2), and the log prints it.
- A baseline pull request whose step needed a rebase stays open until a maintainer closes it, and then shows "Closed", not "Merged" (T10). Its mark on baseline `master` names it.
- The fingerprint store stays as it is. A conflict there is a real conflict (decision of the user, 2026-10-20).

## Permissions

- Each push (opp_repl, opp_ci, INET, statistics, media), each push to the pull request of another author, and each change of a required check needs the approval of the user at that time.
- The rule units of part 2 have no `🔒` flag (F14). A rule change can make a seal stale (`SR-RULE-CHANGE-STALES`), so step P6 runs `check-seals.sh`.
- This plan changes no baseline value.
