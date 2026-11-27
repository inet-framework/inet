# INET `master`: guards and automated tests (target state)

This file describes how the system works when all agreed and proposed parts are in place. It gives no steps and no current state.

**Notation.** Each rule has an ID. The state column says:

- **D**: decided by the user earlier (the date or the plan is in brackets).
- **P**: a proposal. Accept, change or reject it.

## 1. Actors

| Actor | GitHub role | Short name |
| --- | --- | --- |
| A person with no role, or a first-time contributor | read (fork) | contributor |
| A person with the role write | write | collaborator |
| A person with the role maintain | maintain | maintainer |
| A person with the role admin, or an organization admin | admin | admin |
| A workflow with the token of GitHub Actions | — | workflow |
| The landing job (`Check: baseline masters`), with the deploy keys of `statistics` and `media` | — | landing job |

## 2. Who can do what

| ID | Rule | State |
| --- | --- | --- |
| A1 | Each person can open a pull request into `master`: from a fork, or from a branch of the repository (collaborator and up). | D |
| A2 | The workflows of a pull request from a first-time contributor start only after a collaborator approves the run. | D (2026-11-26) |
| A3 | A collaborator can merge a pull request only when all required checks (section 3) are green. | D (2026-11-25) |
| A4 | The only merge button is "Rebase and merge". "Create a merge commit" and "Squash and merge" are off, because they can lose the pin trailers and the `Change:` trailers of the commits. | D (2026-11-26) |
| A5 | A person with the role write or more can push to `master` directly only a trivial change (S1 to S6). Each other change needs a pull request. | D (2026-11-26) |
| A6 | A maintainer and an admin bypass the required checks only through a pull request (the bypass mode "pull requests only" of GitHub): they can merge a pull request before its checks are green, and the pull request shows each use. The bypass is for the repair of a red `master` and for a required check that is broken. Nobody pushes directly to `master`, except a trivial change (S1 to S6). | D (2026-11-26) |
| A7 | Only an admin changes the ruleset, the environments and the secrets. | fact of GitHub |
| A8 | No workflow pushes to INET `master`. | fact of the design |
| A9 | Nobody adds comments to the pull requests of other authors to ask for checks or updates; the red checks and the guides say what to do. | D (2026-11-25) |
| A11 | No rule asks for a review by a second person. A collaborator can merge their own pull request when the checks are green. The exception is sealed source (R9, R10). | D (2026-11-26) |
| A12 | A pull request does not have to be up to date with `master` to land. M8 lets one landing at a time onto a `master` that is not checked yet, so a red from old results names that landing, and its revert is the repair. | D (2026-11-26) |

The two ways into `master`:

```
pull request ──(all required checks green)──────► "Rebase and merge" ──► master   (collaborator and up)
push to a branch ──(push check green: trivial)──► push of the same commit ──► master   (collaborator and up)
```

### A trivial change without a pull request

| ID | Rule | State |
| --- | --- | --- |
| S1 | A push to a branch other than `master` runs one job, the push check, and nothing else. It builds nothing and runs no test. | D (2026-11-26) |
| S2 | The push check looks at the commits of the branch since `master`. The change is trivial when each changed file is trivial (S3), no commit has a pin trailer, and the gates of `Mechanical audit` pass, without the description. A pin trailer moves a baseline at the landing, also when the commit changes only documents. | D (2026-11-26) |
| S3 | A trivial file change is one of these: (a) a change of an `.rst` or `.md` file; (b) a change of a `.cc`, `.h` or `.ned` file that changes only comments and keeps the line number of each line of code. The check removes the comments from the old and the new file, keeps the line breaks, and compares the two. A comment that adds or removes lines is not trivial: it moves `__LINE__`, which reaches the outputs that the module and protocol tests compare. Never trivial: files under `.github/`, `_scripts/`, `doc/project/enforcement/` and `tests/`, sealed source paths, sealed documents (for example `rule/sealing.md`), the files of the seal registry, `.txt` files (simulations read some of them, for example `examples/mobility/vehicle_waypoints.txt`), `.ini` files (their comments can hold test keys such as `#expected-result`), and `.msg` files (they generate C++ code). | D (2026-11-26) |
| S4 | For a trivial change, the push check marks each required context of `master` as green on the head commit, with the token of GitHub Actions. The exception is `Master green`, which keeps its real state (M8). It reads the list of contexts from the rules of `master`, so it needs no copy of the list. For another change, it marks nothing and reports "this change needs a pull request". | D (2026-11-26) |
| S5 | The collaborator then pushes the same commit to `master`. If `master` moved in the meantime, the collaborator rebases and pushes the branch again. | D (2026-11-26) |
| S6 | The push check takes its scripts from the copy on `master`, and it reads the pushed commits only as data. For a push, GitHub runs the workflow file of the pushed branch, so the workflow file itself cannot come from `master` (see the limit). | D (2026-11-26) |

Limit: a person with write access can always write a workflow of their own that marks the required contexts as green. The rules stop mistakes, not a collaborator who wants to break them.

Tested in step G6 of [required-checks.md](required-checks.md) (2026-11-27): a commit status of the workflow token counts for a required context that must come from GitHub Actions (W6), and GitHub accepts the push of a commit with green required contexts from a person with the role write.

## 3. Required checks

A required check must be green before a merge. A required job always runs: GitHub gives a job that its `if:` skips the name without its matrix value, for example `statistical-tests` for `statistical-tests (release)`, and the required context then never reports.

| ID | Context | Workflow | Reports on | It is red when | State |
| --- | --- | --- | --- | --- | --- |
| R1 | `Mechanical audit` | Check: pull request | each event: opened, reopened, push, edit of the description | a gate finds a violation in the commits or in the description (section 6) | D (2026-11-25) |
| R2 | `Baseline merge` | Check: pull request, and the landing job | each event; again after each landing, on the open pull requests with a pin trailer | the baseline branch of a pin trailer cannot land on its baseline `master`: the rebase fails, or a result file is common with a result that landed since | D (2026-11-25) |
| R3 | `Attribution runs` | Check: baseline attribution | opened, reopened, each push to it | the commit that moves a pin or a fingerprint does not give the old values at its parent, the new values at itself, and the values that its reduction claims | D (2026-11-25) |
| R4 | `statistical-tests (release)` | Test: statistical | opened, reopened, each push to it | a selected statistical test fails against the baseline that the landing will make | D (2026-11-25) |
| R5 | `chart-tests (release)` | Test: chart | opened, reopened, each push to it | a selected chart test fails against the same baseline | D (2026-11-25) |
| R6 | `fingerprint-tests (debug, 0..3)`, `fingerprint-tests (release, 0..3)` | Test: fingerprint | opened, reopened, each push to it | a selected fingerprint test fails against `tests/fingerprint/store.json` | D (2026-11-26) |
| R7 | the jobs of the kinds unit, module, queueing, protocol, packet and validation | their test workflows | opened, reopened, each push to it | a selected test of the kind fails | D (2026-11-26) |
| R8 | `Enforcement checker tests` | Tests: project enforcement | opened, reopened, each push to it | a unit test of the gate scripts fails | D (2026-11-26) |
| R9 | `Source seal policy` | Check: sealing | opened, reopened, each push to it | a change of a sealed source path has no approval. The approval is a review in the environment `sealed-source-change` by a member of the team `inet-core` (the core group). | D (2026-11-26) |
| R10 | — | Check: sealing | — | In the environment `sealed-source-change`, a person cannot approve a run that they started ("Prevent self-review"): another member of `inet-core` approves a sealed change. | D (2026-11-26) |

Not required: `main` of `Build: Linux`, because its trigger has a path filter (rule W1) and each test job builds INET.

### Rules for a required job

| ID | Rule | State |
| --- | --- | --- |
| W1 | A required job reports a result on each pull request. Its workflow has no path filter in its pull request trigger. A job with a matrix has no job-level `if:`: when the job has nothing to test, its steps skip and it reports green, or it reports `Selected 0 of M`. A job-level `if:` on another required job is true for each event of a pull request, because a skipped job counts as passed. | D (2026-11-26) |
| W2 | A change that renames or removes a required job, or changes its matrix, lands together with the change of the ruleset. Otherwise each pull request waits for ever. | D (2026-11-26) |
| W3 | A new job becomes required only after it reports on pull requests. | D (2026-11-26) |
| W4 | `Mechanical audit` is red for a pull request that renames or removes a required context. | D (2026-11-26) |
| W5 | `doc/project/enforcement/README.md` lists the required contexts and the rules W1 to W4. The ruleset stays the source. | D (2026-11-26) |
| W6 | Each required context must come from GitHub Actions. This includes `Baseline merge`, after a test shows the app that GitHub records for a commit status of a workflow. | D (2026-11-26) |

## 4. A pull request

```
event of the pull request (opened, reopened, push, edit)
 │
 ├─ trusted jobs: code of master, the pull request only as data, a token that can post statuses
 │    ├─ R1  Mechanical audit           the gates of section 6
 │    ├─ R2  Baseline merge             can each baseline branch land?
 │    └─ R9  Source seal policy         sealed paths need a review in the environment
 │
 └─ jobs of the pull request code: a read-only token, no secrets
      ├─ R4 R5 R6 R7  test jobs          the selected tests of each kind (T1 to T4)
      ├─ R3  Attribution runs            the runs that prove the cause of moved values
      └─ R8  Enforcement checker tests
                       │
        all required checks green ──► "Rebase and merge"
```

| ID | Rule | State |
| --- | --- | --- |
| T1 | A test job of a pull request runs only the tests that the change can affect. The selection compares the change with the test map of the newest finished push run whose commit is an ancestor of the pull request, else with the weekly map. The job uses the commits of omnetpp and opp_repl that the header of the map names. | D (test selection plan, 2026-11-26) |
| T2 | A test job always reports `Selected N of M`, also for N = 0. | D (test selection plan) |
| T3 | With the label `full-tests`, a test job runs all tests of its kind. | D (test selection plan) |
| T4 | A change of a CI file (`.github/workflows/`, `_scripts/github/`) selects all tests. | D (test selection plan) |
| T5 | A test job tests the merge of the pull request with `master`, against the baseline that its landing will make: baseline `master` with the steps of the pin trailers on it. | D (pin trailers plan) |
| T6 | A test job only tests. It records no baseline and uploads no candidate values. The author records the baseline on their own machine. | D (2026-11-23) |
| T7 | A pull request with no event after a new required check landed has no result for that check, and it cannot merge. The author starts the checks with a push. An edit of the description starts only R1 and R2. No run by hand starts them for the author. | D (2026-11-25) |
| T8 | A test job selects its tests before the build. With `Selected 0 of M`, it skips the build and is green after a few minutes. This needs a list of all tests of the kind at the head without a build, because the selection always selects a new test that the map does not know. The fingerprint tests have this list in `tests/fingerprint/store.json`. For the opp tests (unit, module, queueing, protocol), the `.test` files can give it later. The statistical, smoke, chart and validation tests need the build to list their configurations. So a kind with no such list uses a list of paths that can change a result before the build, as the statistical and chart jobs do now. The test selection plan has this step since opp_repl 50d28b9. | D (2026-11-26) |

## 5. A push to `master`

```
push to master (a landing by "Rebase and merge", or a trivial push)
 │
 ├─ landing job (Check: baseline masters)
 │    ├─ for each new commit with a pin trailer: put its baseline step onto master of statistics or media
 │    │    (fast-forward if possible, else rebase), then add one mark commit
 │    ├─ post Baseline merge (R2) on each open pull request with a pin trailer
 │    └─ then: statistical and chart tests, all tests, against the baseline that it landed
 │
 ├─ all tests of each kind (R4 to R7), with the newest omnetpp and opp_repl
 │    └─ the debug fingerprint jobs run in coverage mode and save the test map of the commit
 │
 └─ check V4: each test whose result changed since the previous push must be one that the selection gives
```

| ID | Rule | State |
| --- | --- | --- |
| M1 | Each push to `master` runs all tests of each kind that has a test job (R4 to R7). Nobody waits for this run. | D (test selection plan) |
| M2 | The landing job lands the baseline steps. If it fails at an overlap, a maintainer repairs the baseline by hand and adds the mark. | D (pin trailers plan) |
| M3 | After a rebase of a baseline step, a maintainer closes the baseline pull request; after a fast-forward, GitHub marks it as merged. | D (pin trailers plan) |
| M4 | Check V4: a change of a test result between two pushes that the selection did not give is a defect of the selection, and the check names the rule. | D (test selection plan) |
| M7 | Each week: the coverage run makes the weekly test map and checks the selection (V2); the feature tests and the builds on macOS and Windows run too. | D |
| M8 | A landing waits for `master`. The required context `Master green` is green only when the full run of the head of `master` is finished and green. While that run is in progress or red, nothing else lands, so each landing is checked before the next one, and a red names one landing. When the run is red, only the repair or the revert lands: a maintainer merges it with the bypass (A6). A full run needs 25 to 40 minutes, so at most about two landings fit into one hour. | D (2026-11-26) |
| M9 | A landing whose changes are all trivial (S3) starts no full run, and it does not change `Master green`. Without this rule, each typo blocks `master` for the time of a full run. | D (2026-11-26) |
| M10 | For M8, the full run is the test jobs of R4 to R7 and the landing job, on the head of `master`. The check V4 and the weekly runs do not count. For a false red, a collaborator runs the failed job again, and `Master green` follows the newest attempt. A red counts also when its cause is a new omnetpp or opp_repl; the report of the run names the commits of both, for this run and for the last green run. | D (2026-11-26) |

## 6. What each change must carry

| ID | Kind of change | The author supplies | Checked by |
| --- | --- | --- | --- |
| C1 | Each commit | A subject "area: kind: what it does"; a `Change:` trailer; a body for a commit above 50 changed lines; no attribution trailer; no merge commit or fixup commit in the series; no whitespace-only file inside a functional commit, and no rename together with a change of the content; each `#include` and each `par()` valid at its own commit | R1 |
| C2 | A change that moves fingerprints | The new values of `tests/fingerprint/store.json` in the same commit. The `Baseline changes:` list with a reason on each line. At most 100 changed source lines (a note above 30), or a patch after `Reduction:` with `Attribution: forward` or `Attribution: reverse`. | R1, R3, R6 |
| C3 | A change that moves statistical or chart results | The new values, recorded on the author's own machine, on a branch of `statistics` or `media` with the record (the list with reasons, `INET-Subject:`, `Attribution:`). The trailer `Statistical-Baseline:` or `Chart-Baseline:` with the full hash in the cause commit. The same size limit as C2. | R1, R2, R3, R4, R5 |
| C4 | Each pull request that moves a baseline | A section "Baseline changes" in the description: the subject of each commit that moves a baseline, with a summary | R1 |
| C5 | A release note or a migration section | A file of its own in `doc/whatsnew/` or `doc/src/migration-guide/development/` | R1 |
| C6 | A change of a sealed source path | A review in the environment `sealed-source-change` | R9 |
| C7 | A change of documents or plans only | C1 only; the test jobs select no test | R1 |
| C8 | A change of a CI file | C1; the test jobs run all tests (T4) | R1, all test jobs |

## 7. The baseline repositories (`statistics`, `media`)

| ID | Rule | State |
| --- | --- | --- |
| B1 | The baseline commits of a series are on a branch with the name of the INET branch, or on a pull request from a fork. | D |
| B2 | Only the landing job, a maintainer and an admin can push to `master` of a baseline repository; a ruleset there enforces this, with a bypass for the deploy key and for these two roles. The maintainer is in the list for the repair of M2. | D (2026-11-26) |
| B3 | The baseline repositories have no workflow and no token of their own. | D (pin trailers plan) |

## 8. Record of the review

All rules have the state D, and no proposal is open. A review on 2026-11-26 corrected W1, S2, S3, S4, S6, T7 and B2. After it, the user decided these changes:

- M8: a landing also waits while the full run of `master` is not finished.
- A11: no rule asks for a review by a second person.
- M9, M10 and A12.
- A6: the bypass works only through a pull request. M5 and M6, the rules for a direct push of a maintainer, left the target state with this change.

Small points: T8 names the smoke tests, which have no job; the label of T3 starts no run before the next push.

Rejected on 2026-11-26: A10, auto-merge of a pull request when its checks are green.
