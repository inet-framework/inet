# Run the mechanical pull-request audit in CI

> **Status:** in progress · **Branch:** `topic/pull-request-audit-ci` · **Base:** `origin/master` `f26dcbc424` (2026-10-13)

## 1. Problem and intended behavior

The pull-request audit in [review-a-pull-request.md](../../doc/project/guide/review-a-pull-request.md) has a mechanical part and a judgment part. Steps 2 to 4 of the guide are mechanical: five T3 gates read the commit series and the description, and they need no judgment. A person must remember to run them now. [enforcement/README.md](../../doc/project/enforcement/README.md) marks each of the five gates "by hand; not yet in CI", and [run-the-gates.md](../../doc/project/guide/run-the-gates.md) calls this "the next CI coverage gap to close".

The effect is that a contributor learns about a violation only when a reviewer runs the gates. An outside contributor does not know the rules at all. The T3 tier in the enforcement README says "a script in this folder reports it in CI", so the rules already expect CI to run these gates.

Intended behavior: GitHub CI runs the five gates on every pull request to `master`. The run starts when the pull request opens, gets a new push, or gets a new description. A maintainer can also start one run over every open pull request, so that the pull requests that exist now get a result without a new push. A violation makes the check fail. A note never makes it fail. The report shows on the run page of the check.

## 2. Current implementation and evidence

All measurements are from 2026-10-13, with the gates of `f26dcbc424`.

**The five series gates** read only git objects of the range `merge-base..head`. They do not need a checkout of the pull-request tree, and they do not run code from it:

| Gate | Rules | Reads |
| --- | --- | --- |
| [check-commits.sh](../../doc/project/enforcement/check-commits.sh) | `PR-SPLIT-*`, `PR-SERIES-*`, `PR-MSG-*` | `git log`, `git show` of the range |
| [check-classification.sh](../../doc/project/enforcement/check-classification.sh) | `CR-*`, and the commit breakdown | `git log`, `git show` of the range |
| [check-includes.sh](../../doc/project/enforcement/check-includes.sh) | the static half of `PR-SERIES-BUILDS` | `git show`, `git ls-tree` per commit |
| [check-ned-params.sh](../../doc/project/enforcement/check-ned-params.sh) | the static half of `PR-SERIES-BUILDS` | `git show`, `git ls-tree` per commit |
| [check-pr-description.sh](../../doc/project/enforcement/check-pr-description.sh) | notes for `PR-REQ-STORY` | the description text, and `git diff --numstat` of the range |

Each gate returns 0 for clean, 1 for a violation, and 2 for invalid usage.

**A sample of open pull requests.** The command for each was `<gate> $(git merge-base refs/pr/<n> origin/master)..refs/pr/<n>`:

| PR | Commits | commits | classification | includes | ned-params | Longest gate |
| --- | ---: | --- | --- | --- | --- | ---: |
| 1312 | 3 | pass | pass | pass | pass | 0.2 s |
| 1311 | 1 | pass | pass | pass | pass | 0.1 s |
| 1309 | 10 | pass | pass | pass | **4 false** | 8.4 s |
| 1308 | 4 | pass | 4 (no `Change:`) | pass | pass | 2.2 s |
| 1306 | 1 | pass | pass | pass | pass | 0.1 s |
| 1297 | 1 | pass | pass | pass | pass | 0.1 s |
| 1155 | 61 | 15 | 63 | 4 | 8 | 25.8 s |

**A false positive in `check-ned-params.sh`.** The four violations on PR 1309 are all wrong. The gate takes the `par("x")` reads from the added lines of *every* file of a commit, but it looks for the declarations only in `.ned` files. The reads on PR 1309 are in `tests/module/Ieee80211HcfHeader_1.test`, and that file declares the parameters in its own embedded NED. The gate must take the reads only from `.cc` and `.h` files under `src/`, as its header says. A gate in CI with false positives teaches people to ignore it ([checklist/general.md](../../doc/project/enforcement/checklist/general.md): precision over recall).

**Commits that predate the classification rule.** The review guide says: "Do not report a missing trailer on a commit that predates the rule." `classification.md` entered the tree in `3e741f4773` on 2026-09-11. The gate does not apply this exception now; a reviewer applies it by hand. All 61 commits of PR 1155 have author dates of 2026-07-27 to 2026-07-29, and 61 of its 63 classification violations are missing trailers. In CI, the exception must be in the gate.

**The tree gates fail on master now.** On the tree of `f4617d7c47`: `check-architecture.sh` shows 26 violation lines, `check-interfaces.sh` 16, and `check-naming.sh --base origin/master` 24, all old debt. `check-seals.sh` passes. A tree gate in CI must therefore compare the head with the merge base and fail only on new findings. That is a different design, and it is out of the scope of this plan (see section 3).

**The existing trusted pattern.** [check-sealing.yml](../../.github/workflows/check-sealing.yml) runs on `pull_request_target`. It checks out the base commit, fetches the pull-request head as data, makes sure that the fetched head is the expected SHA, and runs the base-branch gate over it with a read-only token. This plan uses the same pattern.

**Clone size.** The INET pack is 330 MiB. The sealing workflow already does a full clone (`fetch-depth: 0`) for each pull-request event.

## 3. Scope and acceptance criteria

In scope:

- The five series gates in CI for every pull request to `master`.
- A manual run over a list of pull requests, or over every open pull request to `master`.
- The two gate corrections that CI needs: the `par()` false positive, and the exception for commits that predate the classification rule.
- The documentation that describes where the gates run.

Out of scope, each for a later plan:

- **The tree gates** (`check-architecture.sh`, `check-interfaces.sh`, `check-naming.sh`, `check-ned-msg-naming.py`, `check-seals.sh`). They need a ratchet against the merge base, because master has old findings.
- **A per-commit build** (`check-series-builds.sh`, the T2 half of `PR-SERIES-BUILDS`). It runs pull-request code, so it must not run on `pull_request_target`. It needs minutes per commit.
- **The T4 agent review** of [checklist/general.md](../../doc/project/enforcement/checklist/general.md). It needs an API secret in a workflow that reads untrusted text.
- **A required status.** A maintainer decides that after a trial, in the repository settings.
- **A pull-request comment.** It needs a write token, and it sends a notification on each run.

Acceptance criteria:

1. One command runs the five gates over a pull request, locally and in CI, and gives one exit status.
2. The unit tests of `doc/project/enforcement` pass, with new tests for each changed gate and for the new command.
3. On the sample of section 2, PR 1309 shows no `check-ned-params.sh` violation, and PR 1155 shows no violation for a missing trailer on its July commits.
4. The workflow runs no code from the pull request, and gives the job only the token permissions that it needs.
5. `actionlint` accepts the workflow, if it is available.

## 4. Proposed design and reasons

**D1. One entry command, `check-pull-request.sh`.** It takes `--base <ref>`, `--head <ref>` (default `HEAD`), and a description through `--description-file <file>` or `--pr <n>`. It computes the merge base, runs the five gates in the order of the review guide, and prints each output. It returns 2 if a gate returns 2, else 1 if a gate returns 1, else 0. With `--markdown <file>` it also writes the report as Markdown for the job summary. Reason: the same command serves the reviewer at step 2 of the guide and the CI job, so the two cannot drift. The workflow stays short. The command is a shell wrapper over a Python module, like `check-pr-description.sh`, because the Markdown report must escape untrusted text.

**D2. The workflow runs on `pull_request_target`, with the gates of the base commit.** The events are `opened`, `reopened`, `synchronize` and `edited`; `edited` runs the audit again when the description changes. Reasons:

- A pull request cannot change the gate that judges it. This is the reason in `check-sealing.yml`.
- Every pull request gets the gates of the current master, also a pull request whose branch is older than the gates. The fix of `f26dcbc424` from today shows why this matters.
- GitHub runs a `pull_request` workflow from a first-time outside contributor only after a maintainer approves the run. A `pull_request_target` workflow runs at once.

The risk of `pull_request_target` is a token of the base repository in a job that reads untrusted data. The design keeps that risk small: the job never checks out or runs pull-request content; the token has `contents: read` only; the job uses no secret; the description goes through an environment variable into a file, and never into a shell command; and the job stops the processing of workflow commands while a gate prints commit text, so a commit subject cannot inject a workflow command.

**D3. A manual sweep for the pull requests that exist now.** `workflow_dispatch` takes an input `pull-requests`: a list of numbers, or `open` for every open pull request to `master`. One job clones once, fetches all the selected heads, runs the entry command for each, and posts a commit status on each head with a link to the run. Reasons: an open pull request without activity gets no `pull_request_target` event, and a close and reopen of each pull request sends notifications to every author. One job with one clone costs less than a matrix of 100 clones. The sweep job alone gets `pull-requests: read` and `statuses: write`; the event job keeps the read-only token. The run summary holds a table of all results and the full report of each pull request, up to the size limit of the summary; an artifact holds all reports.

**D4. The exception for commits that predate the classification rule goes into the gate.** `check-classification.sh` gives a note, not a violation, for a missing `Change:` line on a commit with an author date before 2026-09-11. A commit that has a trailer gets all checks. The other `PR-*` rules apply to all commits, because the guide gives no exception for them. Reason: the guide already states the exception, and CI cannot ask a reviewer. The author date is the date that a rebase keeps. The gate is not a security control, so a false date is not a risk to defend against.

**D5. `check-ned-params.sh` reads `par()` only in `src/**/*.cc` and `src/**/*.h`.** Reason: section 2. The gate exists for `PR-SERIES-BUILDS` of the library; a test declares its own parameters in its own NED.

**D6. The report goes to the job summary, not to a comment.** The summary shows each gate with its result and its full output in an escaped `<pre>` block. Reason: no write token and no notification. A comment can come later, as a separate decision.

## 5. Affected components and compatibility

- New: `doc/project/enforcement/check-pull-request.sh`, `check_pull_request.py`, `test_check_pull_request.py`, `test_check_ned_params.py`, `.github/workflows/check-pull-request.yml`.
- Changed: `check-ned-params.sh`, `check-classification.sh`, `test_check_classification.py`, and the documents `enforcement/README.md`, `guide/run-the-gates.md`, `guide/review-a-pull-request.md`.
- The gates keep their command lines and exit statuses. The local use stays the same.
- The workflow is new, and no other workflow changes. It is not a required status, so it does not block a merge.

## 6. Implementation steps and dependencies

Each step is one commit, with a `Change:` trailer.

- [ ] **S1.** Commit this plan.
- [ ] **S2.** `check-ned-params.sh` reads `par()` only in `src/` C++ files (D5), with a new regression test.
- [ ] **S3.** `check-classification.sh` gives a note for a missing trailer on a commit that predates the rule (D4), with tests.
- [ ] **S4.** The entry command `check-pull-request.sh` (D1), with tests.
- [ ] **S5.** The workflow `check-pull-request.yml` (D2, D3, D6).
- [ ] **S6.** The documents: the gate inventory, the gates guide, and the review guide.
- [ ] **S7.** Run the sample again, record the results here, and move the plan to `plan/done/`.

S4 needs S2 and S3 only for the sample results. S5 needs S4.

## 7. Verification methods and expected results

- `python3 -m unittest discover -s doc/project/enforcement -p 'test_*.py'` passes. This is the command of `enforcement-tests.yml`.
- The entry command over the sample of section 2 gives the expected results of acceptance criterion 3, and the run times stay below one minute for each pull request.
- `actionlint` over the new workflow gives no error.
- The project gates for the branch itself pass: `check-commits.sh`, `check-classification.sh`, `check-links.sh`, `check-seals.sh`, `check-naming.sh --base origin/master` for the new workflow name.

**Not verified locally.** GitHub runs a `pull_request_target` or a `workflow_dispatch` workflow only from the default branch. So the first real run occurs after the workflow lands on `master`, or in a fork whose `master` has it. Do one fork run, or watch the first runs on `master` and the first sweep, before the check becomes required.

## 8. Risks, unresolved decisions, and approval scope

- **Noise on existing pull requests.** The sweep posts a failure on each old pull request with a violation. That is the purpose of the sweep, but it is visible to each author. Run it first with a short list of numbers.
- **A stale result.** A gate change on master does not run the audit again on an existing pull request. A new sweep does.
- **Unresolved:** a scheduled sweep, a pull-request comment, and a required status. Each is a later decision for a maintainer.
- **Seals and baselines:** no sealed path changes, and no baseline changes.
- **Approval:** the request of 2026-10-13 asks for this function. The draft [PLR-APPROVAL](../../doc/project/rule/planning.md#plr-approval) is not in effect for this task. Nothing is pushed; the maintainer reviews the branch before a push.

## Decision log

(Filled in during implementation.)
