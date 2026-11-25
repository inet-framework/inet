# Baseline pins in commit trailers

**Status:** pending. Written 2026-11-25 from `master` at `28f1dc534c` (equal to `origin/master`). On 2026-11-25 the user chose this design ("yes write the trailer design as a plan"). No step has started. Each step needs the approval of the user.

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

## Definitions

- **Pin**: the full hash of a baseline commit that an INET commit names. Today: the `ref` in `inet.opp`. With this plan: a trailer in the message of the cause commit.
- **Cause commit**: an INET commit that changes a baseline. Its message has a pin trailer.
- **Baseline branch**: the baseline commits of one series. It starts on a commit of baseline `master`, its base.
- **Step**: the baseline commits that one pin adds. A step goes from the pin of the previous cause commit of the series, or from the base of the baseline branch, to the pin.
- **Mark**: the trailer `Baseline-Of: <INET hash>` of a merge commit on baseline `master`.
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

The value is the last commit of the step, on the baseline branch of the series. The rules of a step do not change: the step has no merge commit, at most one of its commits changes results, and that commit has the record (`TR-BASELINE-PIN`, D12 of the earlier plan). The pinned commit is in the upstream baseline repository, on a branch with the name of the INET branch or as the head of a pull request there (D2 of the earlier plan).

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
3. M is the newest commit on the first-parent line of baseline `master` whose mark names B or an ancestor of B. If no mark does, M is the `ref` of the newest ancestor of B that has a `ref` (section 5).
4. S is the pin of the newest commit in `B..X` that has the trailer. If no commit has one, the baseline is M.
5. Otherwise the baseline is the merge of S into M. The function makes the merge commit locally, with a fixed author, committer and date, so its hash is the same on each machine. The function stops with an error if the merge conflicts or if the two sides overlap. The message names the files.

This one function serves all callers:

- **V1**, a pull request run on the merge ref: B is the head of `master`. The baseline is the baseline branch merged into the newest baseline `master`. This is the baseline that the landing will make.
- **V2**, the attribution run at a parent and at a cause commit: B is the base of the pull request. The two baselines differ only by the step of the cause commit.
- **A push run on `master`**: no trailer is in `B..X`, and M is the merge that the merge job made for the newest cause commit.
- **A bisect, opp_ci, a developer machine**: the same function, on any commit.

### 3. The merge job

`check-baseline-masters.yml` keeps its trigger, its environment `baseline-masters`, its deploy keys and its concurrency group (F9). `baseline_masters.py` changes from a fast-forward to a merge:

1. For each new commit X of the push, in first-parent order, and for each pin trailer of X: fetch the pin S from the upstream baseline repository. Check the overlap of S with the head of baseline `master`. Make a merge commit with `--no-ff`. Its subject is `Merge the baseline of INET <short hash>: <subject of X>`, and its trailer is `Baseline-Of: <full hash of X>`.
2. Push baseline `master`. The push is a fast-forward and is never forced.
3. If an overlap or a conflict stops a merge, the job pushes nothing for that kind and fails. Its log names X, S and the files. A maintainer repairs it by hand: run the moved tests of the common files at X, commit their results as the resolution of the merge, with the mark, and push to baseline `master`.
4. After the push, post the status `Baseline merge` on the head of each open pull request that has a pin trailer (section 4).

The merge keeps the commits of the baseline branch, with their hashes and records:

- The pin of the INET commit stays a commit of baseline `master`.
- `git blame` of a baseline file still shows the commit with the record.
- GitHub shows the pull request of the baseline repository as merged, because its head is in `master`, as today.

The statistical and chart tests of `master` start after the merge job. Their trigger for `master` changes from `push` to a `workflow_run` of the merge job (R5).

### 4. The status `Baseline merge`

The command `baseline_masters.py check <INET commit>` does steps 2 to 5 of section 2, but it makes no merge commit. It examines only the conflict and the overlap. It needs git and no simulation, so it is fast.

- `check-pull-request.yml` posts the status on each event of a pull request: opened, reopened, synchronize, edited.
- The merge job posts the status on each open pull request with a pin trailer, after each move of baseline `master`. This is the only case where a pull request becomes red with no event of its own. The commit status API works on the heads of pull requests from forks (F5).
- `Baseline merge` becomes a required check of `master`. Rudolf Hornig sets the required checks (D11 of the earlier plan).

### 5. The switch

The gate of `master` examines the pull request that changes the gate (R8), and the gate of today fails a commit whose baseline entry has no full-hash `ref` (F10). Thus the switch lands in two parts:

- **Part 1** accepts the two forms. The gates, the merge job and the tools read a `ref` or a trailer. INET `master` keeps its `ref`, and the merge job keeps the fast-forward for a `ref`.
- **Part 2** is the switch commit. `inet.opp` gets the `trailer` keys and loses the `ref`. The rules and the guides change. The switch commit moves no pin, so the first mark comes with the first cause commit after it. Until then, step 3 of section 2 gives the `ref` of the parent of the switch commit, which is the head of baseline `master`.

After part 2, the gates reject a `ref` in a new commit. opp_repl still reads a `ref` in old commits (R7).

### 6. What changes for the author

1. Record the values on your machine (D10 of the earlier plan), on a branch of the baseline repository that starts on the head of baseline `master`. Write the record as today.
2. Add the pin trailer to the cause commit. Do not change `inet.opp`.
3. If a baseline commit changes later (a new record or new values), its hash changes. `baseline_pins.py retrail` writes the new hashes into the trailers of the series, in the order of the steps.
4. After another pull request lands, do nothing, unless `Baseline merge` is red. If it is red, record the common files again on the new head of baseline `master`, then run `retrail`.

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
| INET, part 1 | `doc/project/enforcement/baseline_masters.py` | The merge of section 3 and the command `check` of section 4. |
| INET, part 1 | `doc/project/enforcement/baseline_attribution.py:128-170, 188-222` | `find_steps` and `add_fingerprints` read the pin trailers. The checkout gets the baselines from the function of section 2. |
| INET, part 1 | `.github/workflows/check-baseline-masters.yml`, `check-pull-request.yml` | The status `Baseline merge`. |
| INET, part 1 | `.github/workflows/statistical-tests.yml`, `chart-tests.yml` | On `master`: `workflow_run` of the merge job. The path filter (lines 74-75) also reacts to a pin trailer. |
| INET, part 1 | `doc/project/enforcement/test_*.py` | The fixtures of `test_baseline_pins`, `test_check_baselines`, `test_baseline_masters`, `test_baseline_attribution`, `test_baseline_description`, `test_fingerprint_attribution` and `test_check_classification` write trailers, and the two forms are tested. |
| INET, part 2 | `inet.opp:17-47` | The `trailer` keys, no `ref`, and a new comment. |
| INET, part 2 | `doc/project/rule/testing.md:196, 210, 214-228, 230-254` | `TR-BASELINE-PIN`: the pin trailer, the merge job, the overlap instead of the conflict in `inet.opp`. `TR-BASELINE-PROVENANCE`: `INET-Subject:` is the subject of the cause commit. `TR-BASELINE-COMMIT`. `TR-BASELINE-PARENT` and `TR-BASELINE-ATTRIBUTION`: "the old pins" becomes "the baseline of the parent". |
| INET, part 2 | `doc/project/rule/pull-request.md:191, 447, 561`, `doc/project/rule/classification.md:201-246` | "A pin that moves over results" becomes "a pin trailer whose step changes results". |
| INET, part 2 | `doc/project/guide/change-a-baseline.md:106-228`, `run-the-gates.md:35, 49-50`, `review-a-pull-request.md:120`, `contribute-a-change.md:45` | The procedure of section 6, the merge job, the repair of section 3. |
| INET, part 2 | `doc/project/enforcement/README.md:56-58, 89-96`, `.github/pull_request_template.md:16` | The rows and the commands. |
| statistics, media | `README.md` | The merge commits and the mark. |
| outside | `~/workspace/ghci-standards-fixes/tools/check_series.py` (F13) | Use `opp_checkout_test_baselines` instead of the regular expression, when the tool is used again. |

## Steps

| # | Step | Approval at that time |
| --- | --- | --- |
| P1 | opp_repl: the `trailer` form, the function of section 2, the self-tests, the documentation. The self-tests use fixture repositories of INET and of a baseline: a commit on `master`, a branch, a merge ref, the switch, an overlap, a conflict. Commit on `main`, then on `github_ci` and `opp_ci`. | the push |
| P2 | opp_ci: the checkout through opp_repl, and its tests. | the push and the restart of the worker |
| P3 | INET part 1, on the branch `topic/baseline-pins-in-trailers` in a worktree: the tools, the gates, the merge job, the status, the workflows and the tests. One commit for each tool. | — |
| P4 | End-to-end test on local bare copies of INET, statistics and media. Two series with no overlap land one after the other, and the merge job makes their marks. A series with an overlap gets a red status, and the merge job stops for it. A bisect over the switch finds the baseline of each commit. Then a run of the merge job against the real repositories with no `--push`. | — |
| P5 | Land part 1. Check the first run of the merge job on `master`: it sees the `ref` and does the fast-forward as today. | the push |
| P6 | INET part 2: the switch commit, the rules, the guides, `enforcement/README.md`, the template. Run `check-seals.sh` (`SR-RULE-CHANGE-STALES`). Land part 2. | the push |
| P7 | The 5 open pull requests that move a pin (F6): `baseline_pins.py convert` (decision T8). | each push to a pull request |
| P8 | The required check `Baseline merge` (Rudolf Hornig). Record the results in this plan, and move it to `plan/done/`. | the required check |

## Progress

- [ ] P1 opp_repl
- [ ] P2 opp_ci
- [ ] P3 INET part 1
- [ ] P4 end-to-end test
- [ ] P5 land part 1
- [ ] P6 INET part 2 and its landing
- [ ] P7 open pull requests
- [ ] P8 required check, record, plan to done

## Decisions

| # | Decision | Proposal | State |
| --- | --- | --- | --- |
| T1 | The place of the pin | A trailer in the message of the cause commit. Rejected: a comment or the description of the pull request (not in the hash and not in git, F3); the `ref` in `inet.opp` (the conflict of the goal). This decision replaces D1 of the earlier plan. | settled, user 2026-11-25 |
| T2 | How a baseline lands on baseline `master` | A merge commit with `--no-ff` and a mark for each cause commit. Rejected: a rebase or a cherry-pick of the baseline branch. They make new hashes: the pin then names a commit that is not on `master`, GitHub does not show the baseline pull request as merged, and the job rewrites the commits of other authors. | proposal |
| T3 | The overlap | A common file since the base of the baseline branch. This is stricter than a merge with no conflict: two changes of the same result file mean that one simulation moved two times. | proposal |
| T4 | The check of the overlap before the landing | The status `Baseline merge`, posted by the pull request audit and by the merge job, and required. | proposal |
| T5 | The order of the jobs on `master` | The statistical and chart tests start with a `workflow_run` of the merge job. Rejected: a test that waits until the mark is there. | proposal |
| T6 | The form in `inet.opp` | The key `trailer`, a general feature of opp_repl. `ref` stays valid for old commits and for other projects. | proposal |
| T7 | The names | Trailers `Statistical-Baseline:` and `Chart-Baseline:`, mark `Baseline-Of:`. `INET-Commit:` keeps its meaning (F11). The rule ID `TR-BASELINE-PIN` and the names of the scripts stay; their texts change. | proposal |
| T8 | The 5 open pull requests that move a pin (F6) | The user converts the upstream branches (#1484, #1474, #1463) with `convert`. For the forks (#1488, #1459): the author, or a push by a maintainer when the pull request lets maintainers edit it. No comment on pull requests of other authors (D8 of the earlier plan). | open |
| T9 | A failed merge job | A maintainer repairs it by hand, as in step 3 of section 3. | proposal |

## Limits

- PR CI tests a pull request against the baseline `master` of the time of its run. A pull request that lands later can interact with it. The tests of `master` find such an interaction after the landing. Baseline changes with no common file are strong evidence, but not a proof. Code lands in the same way today, unless the branch protection of `master` requires up-to-date branches.
- A failure of the merge job leaves `master` with no baseline for its new commit until a maintainer repairs it (T9).
- After the merge job of one pull request, the new statuses need some time. In that time a person can land a second pull request with an old green status. The concurrency group runs the second merge job after the first one, and the second job fails on an overlap.
- The function of section 2 needs the history of INET `master` and of baseline `master`. With old clones, M is older. If no overlap exists, the merge with S still gives a correct baseline.
- The baseline of a commit that is not on `master` is a local merge commit, which no repository holds. Its hash is the same on each machine (step 5 of section 2), and the log prints it.
- The fingerprint store stays as it is. A conflict there is a real conflict (decision of the user, 2026-10-20).

## Permissions

- Each push (opp_repl, opp_ci, INET, statistics, media), each push to the pull request of another author, and each change of a required check needs the approval of the user at that time.
- The rule units of part 2 have no `🔒` flag (F14). A rule change can make a seal stale (`SR-RULE-CHANGE-STALES`), so step P6 runs `check-seals.sh`.
- This plan changes no baseline value.
