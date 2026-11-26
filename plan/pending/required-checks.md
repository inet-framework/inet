# Required checks of `master`

**Status:** in progress. Written 2026-11-26, and written again the same day after the user decided the target state. The target state is in [master-guards.md](master-guards.md): it has the rules, and each rule has an ID. This plan has the facts of today and the steps from today to that state. The user decided all rules on 2026-11-26. Steps G0, G1, G2 and G5 are done. The user approved G2a and G5 on 2026-11-26; no other step has an approval yet.

## Goal

The repository works as [master-guards.md](master-guards.md) says: a change lands on `master` through a pull request with green required checks, or through the trivial path; a landing waits for a green `master`; and a bypass is visible on a pull request.

## Facts

The facts are of 2026-11-26, with `master` at `0572d1152e`.

| # | Fact | Source |
| --- | --- | --- |
| F1 | The ruleset `master: required checks` (id 28155693) requires 14 contexts: `statistical-tests (release)`, `chart-tests (release)`, `Attribution runs`, `Mechanical audit`, the eight `fingerprint-tests (…)` contexts and `Enforcement checker tests` from GitHub Actions (integration 15368), and `Baseline merge` from any source. A pull request does not have to be up to date. The roles maintain and admin, and the organization admins, bypass it in the mode "always". | GitHub API |
| F2 | GitHub refuses a push to `master` of a commit that does not have the green required checks. A push of a bypass role lands, with the notice "bypassed rule violations". | the pushes of `f0df1e5383` and `0572d1152e` |
| F3 | Only an admin changes a ruleset or an environment. The user is admin of INET for a time, admin of `statistics`, and maintainer of `media`. | GitHub API |
| F4 | GitHub gives a job that its `if:` skips the name without its matrix value. The required context of such a job never reports. #1495 was blocked for this reason. The repair for the statistical and chart jobs is on `master` (`0572d1152e`): on the run of `master`, both jobs report under their full names. No pull request had an event after the repair, so the case of a pull request is not seen yet. | check runs of #1495; runs 45859706559, 45859706663 |
| F5 | The other jobs of a pull request: the eight `fingerprint-tests (…)` jobs have a matrix and no `if:`; `Attribution runs`, `Enforcement checker tests` and `Source seal policy` have fixed names and always report. | `.github/workflows/` |
| F6 | The unit, module, queueing, protocol, packet and validation tests run only on a push to `master`. No test workflow runs on a push to another branch. | `.github/workflows/` |
| F7 | For a `push`, GitHub runs the workflow file of the pushed commit. For `pull_request_target` and `workflow_run`, it runs the file of `master`. | GitHub documentation |
| F8 | Only the button "Rebase and merge" is on (G1). Auto-merge is off. | GitHub API |
| F9 | The environment `sealed-source-change` has no reviewer, and the repository has no variable `SEALED_SOURCE_APPROVAL`. The team `inet-core` has six members, who are all the people with write access or more: rhornig, avarga and levy (admin), torokati44, tabgab and adamgeorge309 (write). | GitHub API |
| F10 | `statistics` and `media` have no ruleset. The landing job writes to them with a deploy key. | GitHub API |
| F11 | The full run of a push to `master` is ten workflows, and it needs 25 to 40 minutes. A newer push does not cancel it. On `0572d1152e`, nine of them are green, and `Test: module` is red; it is red since `d88ae6c573` of 2026-11-25. #1495 changes the HCF module tests and is possibly the repair. | GitHub API |
| F12 | Nobody tested which app GitHub records for a commit status or a check run that the token of a workflow posts, and nobody tested a push to `master` by a person with only the role write. The trivial path depends on both. | — |
| F13 | The plan of the pin trailers is done ([baseline-pins-in-trailers.md](../done/baseline-pins-in-trailers.md)). The plan of the test selection (opp_repl `plan/pending/test-selection.md`, since `5930baf`) holds the rules T1 to T4 and T8, the jobs of R7, the check V4 and the test maps. That work is not finished and not pushed: on 2026-11-26 the local opp_repl `main` has 39 commits that `origin/main` does not have, the branch `topic/test-selection` has 46, and no remote branch has `5930baf`, so the plan is not on GitHub either. Its phase 5, the CI part, has not started. | opp_repl, git |
| F14 | The repository has about 110 open pull requests, and `master` gets about 120 commits each week. | GitHub API |

## From today to the target state

| Rules | State today | Step |
| --- | --- | --- |
| A1 to A4, A9, A11, A12, R6, R8, W2 to W5, T5 to T7, M2, M3, M7, B1, B3, C1 to C5, C7, C8 | in effect | — |
| W1 | repaired for the statistical and chart jobs; the case of a pull request is not seen yet | G0 |
| R9, R10, C6 | the environment is not set up, and the check is not required | G3 |
| B2 | no ruleset | G4 |
| W6 | `Baseline merge` comes from any source | G6 |
| A5, S1 to S6 | no push check | G6, G7 |
| M8, M10 | no context `Master green` | G8 |
| M9 | each push to `master` starts a full run | G9 |
| A6 | the bypass has the mode "always" | G10 |
| T8, the path part | the eight fingerprint jobs build and test on each pull request | G2a |
| R7, T1 to T4, T8, M1, M4 | the plan of the test selection makes the jobs and adds their contexts to the ruleset; that work is not pushed yet (F13) | the plan of the test selection |

## Steps

A step that changes a setting needs an admin at that time. A step with code lands as each change does at that time: by a direct push until G10, and through a pull request after it.

| Id | Step | Needs | Check |
| --- | --- | --- | --- |
| G0 | Done: the statistical and chart jobs always run and report under their full names (INET `0572d1152e`). | — | Left: the first pull request with no result path after the repair shows both contexts green. |
| G1 | Done 2026-11-26: A4, "Create a merge commit" and "Squash and merge" are off. | — | The API shows only the rebase button. |
| G2 | Done 2026-11-26: R6, R8, the eight fingerprint contexts and `Enforcement checker tests` are in the ruleset, from GitHub Actions. | — | Before: each context was on the head of #1495, from integration 15368. After: GitHub marks all nine as required on #1495. |
| G2a | T8, the path part, for the fingerprint tests: the workflow gets the job `Paths that can change a result`, as the statistical and chart workflows have. When no path of the pull request can change a fingerprint, the eight jobs run, skip their steps and report green with no build (W1). The paths are those of the statistical job, and `tests/fingerprint/` and the fingerprint workflow. The selection replaces the list later. | code | A pull request that changes only a document gets the eight contexts green in about one minute. |
| G3 | R9, R10: give the environment `sealed-source-change` the team `inet-core` as reviewers, with "Prevent self-review", and the variable `SEALED_SOURCE_APPROVAL`. Then add `Source seal policy` to the ruleset. | admin | A test pull request that changes a sealed path waits for the review, its author cannot approve it, and another member can. |
| G4 | B2: a ruleset in `statistics` and in `media` that lets only the deploy key and the roles maintain and admin update `master`. | admin of each repository; for `media`, Rudolf Hornig | The next landing with a pin trailer moves the baseline `master`. |
| G5 | Done 2026-11-26: W2 to W5. `check-required-contexts.sh` computes the contexts that the workflows report on a pull request, at the base and at the head, with the names that GitHub gives, and fails a series that renames or removes a required context, gives a required matrix job a job-level `if:`, or gives the workflow of a required job a path filter. The audit runs it with the list of the rules of `master`. `doc/project/enforcement/README.md` lists the 14 contexts and the rules. | — | 17 unit tests; on `master`, the gate computes all 14 required contexts; a temporary rename of the fingerprint job gives 8 violations. Left: the first audit on GitHub, which runs the gate with PyYAML from `actions/setup-python`. |
| G6 | F12, W6: a test on a scratch branch with a ruleset of its own. A workflow posts a commit status and a check run with its token; a pull request into the scratch branch shows if GitHub counts each for a context that must come from GitHub Actions. Then a person with only the role write pushes a marked commit to the scratch branch. With the result, give `Baseline merge` its source in the ruleset. | admin; one collaborator | The merge state of the test pull request, and the result of the push. |
| G7 | A5, S1 to S6: the push check. A script decides if the commits of a branch are trivial: the file rules of S3, with a lexer that removes comments from C++ and NED text and keeps the line breaks; no pin trailer; the gates of the audit. A workflow runs it for each push to a branch other than `master`, and marks the required contexts, but not `Master green`. The guides get the trivial path. | code; G6 | Unit tests: a comment that ends with a backslash, a comment marker in a string, a raw string, a moved line; a real typo fix lands with no pull request. |
| G8 | M8, M10: the context `Master green`. A trusted workflow computes the state of the full run of the head of `master` and posts it on each open pull request, at each change of that state and at each event of a pull request. Then add the context to the ruleset. | code; admin; a green `master` (F11) | A red job on `master` blocks a test pull request; a run of the job again clears it. |
| G9 | M9: a push to `master` whose changes are all trivial, by the script of G7, starts no build and no test, and it leaves `Master green` as it is. | code; G7, G8 | A typo fix on `master` does not block the next landing. |
| G10 | A6: change the bypass of the three actors to the mode "pull requests only". | admin; G7 with a real use; G8; a green `master`; the approval of the user at that time | A direct push of an admin is refused; a merge of a pull request with the bypass works. |
| G11 | Moved on 2026-11-26 to phase 5 of the plan of the test selection: after a new pull-request job of a kind reports on pull requests, an admin adds its context to the ruleset (R7, W3). | — | — |
| G12 | The rules of [master-guards.md](master-guards.md) go into the project documents: the access and landing rules into `rule/pull-request.md`, the test and check rules into `rule/testing.md`, and the procedures into `guide/run-the-gates.md` and `guide/contribute-a-change.md`. | code | `check-links.sh` and `check-seals.sh` pass. |
| G13 | Record the results, and move this plan and [master-guards.md](master-guards.md) to `plan/done/`. | G0 to G10, G12 | Two commits, as `PR-SPLIT-MOVE` asks. |

The order: G1 and G2 were first, because they needed only a setting and the user was admin. Then G5 and G6, which protect the ruleset and test the two assumptions. Then G3 and G4. Then G7, G8 and G9. G10 is the last step that changes how a change lands, because after it each change that is not trivial needs green checks on a pull request, also a change of an admin. G2a can land at any time; it is the first code step, because the fingerprint checks are required since G2. G11 is a step of the plan of the test selection now, which is not finished and not pushed yet.

## Permissions

Each change of a setting, of a ruleset or of an environment, and each push, needs the approval of the user at that time.

## Decisions

The rules and their decisions are in [master-guards.md](master-guards.md). The four questions of the first text of this plan are rules there now: Q1 is R2 (done on 2026-11-26), Q2 is R6, Q3 is W5, and Q4 is W4.

| # | Decision for a step | Proposal | State |
| --- | --- | --- | --- |
| K1 | The marks of the push check (G7) | A check run or a commit status, as the test of G6 shows which one counts for a context from GitHub Actions. | open until G6 |
| K2 | The repair of `Test: module` before G8 | The pull request #1495, if it is the repair; else a repair of its own. | open |
| K3 | The documents for the rules (G12) | As G12 says. | open |

## Limits

- After a new required context, an open pull request with no event has no result for it, and it waits for its next push (T7). G2, G3 and G8 each add such contexts for about 110 open pull requests.
- A false red of a required test, for example from a slow runner, blocks a pull request until somebody runs the job again. With G8, a false red on `master` blocks each landing.
- A person with write access can write a workflow that marks the required contexts as green. The rules stop mistakes.
- An admin can always change or turn off the ruleset. That is the last way out when a required check is broken and no pull request can become green.
