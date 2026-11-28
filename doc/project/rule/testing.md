# Test rules

> **Kind:** rule · **Status:** current · **Seal:** by rule · **Owns:** `TR-*` · **Stands on:** [architecture.md](architecture.md), [test-anatomy.md](../design/test-anatomy.md)

Which test backs which claim, when a recorded expectation may change, and what CI must run. What the twelve test categories *are* is [design/test-anatomy.md](../design/test-anatomy.md); this document says what a change owes them.

**A fingerprint detects a difference in selected ingredients; it does not establish correctness.** A model can produce a stable fingerprint while it retains a defect. Use fingerprints for regression evidence within the selected runs and direct checks for the specific behavior claim.

A rule has a stable identifier `TR-<AREA>`, and the identifier is the heading: [TR-CAT-MATCH](testing.md#tr-cat-match).

## Index

Every test rule in document order.

**Which test**

| Rule | Statement |
| --- | --- |
| [TR-CAT-MATCH](#tr-cat-match) | The test category matches the kind of claim the change makes |
| [TR-FOCUSED-EVIDENCE](#tr-focused-evidence) | Validation runs the directly related cases and records reproducible evidence |
| [TR-SHIP-WITH](#tr-ship-with) | New behavior ships with its test, in the same pull request |
| [TR-PLAN-PROTOCOL](#tr-plan-protocol) | Planned protocol changes include explicit protocol test tasks and a reviewer document for each applicable commit. |
| [TR-FP-NOT-ENOUGH](#tr-fp-not-enough) | A fingerprint is never the only test of new behavior |
| [TR-VALIDATE-EXTERNAL](#tr-validate-external) | A claim about the real world is checked against something outside INET |

**Recorded expectations**

| Rule | Statement |
| --- | --- |
| [TR-BASELINE-DELIBERATE](#tr-baseline-deliberate) | A baseline changes only after explicit approval, never as a side effect |
| [TR-BASELINE-PROVENANCE](#tr-baseline-provenance) | A baseline change names its cause, and gives a reason for each category of its changes |
| [TR-BASELINE-COMMIT](#tr-baseline-commit) | A baseline update travels with the change that causes it |
| [TR-BASELINE-PIN](#tr-baseline-pin) | A commit that moves a result pins its baseline commit in a trailer |
| [TR-BASELINE-PARENT](#tr-baseline-parent) | A baseline changes only for a test that passes on the parent commit and that the change moves |
| [TR-BASELINE-ATTRIBUTION](#tr-baseline-attribution) | The code that moves baseline values is the smallest fragment that produces them, and a run proves it |
| [TR-BASELINE-REPOSITORY](#tr-baseline-repository) | The baseline commits of a change are on a branch of `statistics` or `media`, and only the landing job, a maintainer and an admin move its `master` |

**Determinism**

| Rule | Statement |
| --- | --- |
| [TR-DETERMINISTIC](#tr-deterministic) | The same seed gives the same trajectory, on every platform and every thread count |
| [TR-NO-AMBIENT](#tr-no-ambient) | A model draws only from its own random number generators |
| [TR-SELF-CONTAINED](#tr-self-contained) | A test uses declared inputs and infrastructure, independently of other tests |

**CI**

| Rule | Statement |
| --- | --- |
| [TR-CI-EVERY-COMMIT](#tr-ci-every-commit) | CI builds and tests every commit of a branch, not only its head |
| [TR-SELECTED-PASS](#tr-selected-pass) | Before a push, the tests that the change-based selection gives for the series pass |
| [TR-CI-SELECTION](#tr-ci-selection) | A test job of a pull request runs only the tests that the change can affect, and it reports `Selected N of M` |
| [TR-CI-MERGE](#tr-ci-merge) | A test job of a pull request tests the merge with `master`, against the baseline that its landing will make |
| [TR-CI-ONLY-TEST](#tr-ci-only-test) | A test job only tests: it records no baseline and uploads no candidate values |
| [TR-CI-FULL-RUN](#tr-ci-full-run) | Each push to `master` runs the test jobs of each kind, and the full run of the head of `master` decides `Master green` |
| [TR-CI-FEATURE-MATRIX](#tr-ci-feature-matrix) | A feature is built with its neighbours switched off |
| [TR-FLAKY](#tr-flaky) | A flaky test is repaired or disabled with an issue, never re-run until green |

**Required checks of master**

| Rule | Statement |
| --- | --- |
| [TR-REQ-GREEN](#tr-req-green) | Each required check of `master` is green before a landing, and each comes from GitHub Actions |
| [TR-REQ-REPORTS](#tr-req-reports) | A required job reports on each pull request, and a change of its name lands together with the change of the ruleset |

## Which test

### TR-CAT-MATCH

**The test category matches the kind of claim the change makes.**

| The change claims | Typical test infrastructure |
| --- | --- |
| a computation is right | `unit` |
| a module behaves so, given these inputs | `module` |
| a protocol interaction follows this sequence | `protocol` |
| a serializer round-trips | `unit`, plus the fingerprint `D` ingredient |
| a datapath element chains correctly | `queueing` |
| recorded simulation statistics remain unchanged | `statistical` result regression |
| a statistic has this distribution | `statistical` with an explicit distributional check and justified repetitions |
| the model matches the real world or an analytical result | `validation` |
| a feature builds alone | `features` |
| selected fingerprint ingredients match recorded runs | `fingerprint` |

The category identifies suitable infrastructure, not proof by itself. Verify that the fixture reaches the claimed production path and that its observations establish the expected result. A module fixture can check integration when it connects the required production components. A recorded scalar cannot establish frame encoding unless it directly measures the required byte-level property. A distributional claim needs appropriate samples and a statistical check, regardless of the directory that contains the test. The limits of statistical baseline evidence are in [test-anatomy.md](../design/test-anatomy.md#the-recorded-expectations).

For example, a hypothetical module test drives a production sender and checks the emitted header's receiver address. That test can establish the address claim. The category guidance alone does not require another test with the same assertion.

For planned protocol repairs and features, [TR-PLAN-PROTOCOL](#tr-plan-protocol) adds a specific requirement for protocol tests. Sufficient module coverage does not remove that requirement. Retain useful module tests as additional evidence.

*Enforced at T4 — agent review: does the test type match the claim?*

### TR-FOCUSED-EVIDENCE

**Validation runs the cases directly related to the changed contract through explicit filters, and records enough context to reproduce the result.**

Select cases from the changed paths, symbols and behavioral contracts, in the category required by [TR-CAT-MATCH](#tr-cat-match), and state that mapping. Invoke the test runner with an explicit case, tag or filter. An unfiltered category or repository-wide suite is useful integration coverage, but it is not a substitute for evidence that reaches the changed behavior. If no directly related case exists, report that coverage gap instead of hiding it inside a broader green suite; new behavior still owes a test under [TR-SHIP-WITH](#tr-ship-with).

A reported result includes the working directory, the exact build and test commands, build mode, configuration, run and seed where applicable, explicit filter, exit status, and the paths of any logs, captures or result artifacts used to support the claim. When compiled INET source or generated-code inputs changed, rebuild the matching INET library before the test so the executable and generated sources are current with the source tree.

*Enforced at T4 — agent review of the change-to-test mapping, command scope, library freshness and reported evidence.*

### TR-SHIP-WITH

**New behavior ships with its test, in the same pull request.**

Not in a follow-up, and not in an issue. A test written later is written against the code as it turned out, not against the behavior that was intended, and it will pass over a defect that was in the code from the start. The test is also the only durable statement of what the change was *for*.

*Enforced at T4 — agent review; T2 once coverage is gated.*

### TR-PLAN-PROTOCOL

**Planned protocol changes include explicit protocol test tasks and a reviewer document for each applicable commit.**

Apply this rule to each planned commit that repairs a protocol bug or adds a missing protocol capability. The [commit series](pull-request.md#pr-series-plan) identifies those commits. The plan must include explicit tasks to write or extend protocol tests and to write the associated `.md` reviewer document. A general instruction to add regression coverage is insufficient.

For each applicable commit, the plan must specify these items:

1. The protocol requirement, current bug or gap, and source or observed evidence.
2. The existing tests that expose the gap, or an explicit statement that no such test exists.
3. The exact test paths and cases to add or extend, with existing and proposed cases clearly distinguished.
4. Each test's initial state, configuration, stimulus, production path, protocol observation, and assertion.
5. The expected failure or missing behavior before the change, the expected result after it, and the reason for that difference.
6. The code action that changes the result, with its responsible component and relevant state changes.
7. The test runner, explicit selectors, build requirements, and expected coverage limits under [TR-FOCUSED-EVIDENCE](#tr-focused-evidence).
8. The reviewer document's exact path and planned commit identifier.

Use the protocol test infrastructure described by [derive-tests-from-a-standard.md](../guide/derive-tests-from-a-standard.md). Add required fixture support to the plan when the current infrastructure cannot express the scenario. A helper test or fingerprint alone cannot replace the protocol test task. Keep tests faithful to the requirement. Apply the guide's failure classes before any expected-failure declaration.

Include the tests and reviewer document in the commit that changes the protocol behavior. An earlier test commit is permitted when it respects [PR-SERIES-BUILDS](pull-request.md#pr-series-builds) and the expected-failure rules. In that case, identify both commits in the plan. The behavior commit must update its reviewer document and any test declaration that the repair changes. Do not leave test implementation or documentation to an unspecified later commit.

Write one reviewer document for each applicable commit. Store it beside the protocol tests, for example at `tests/protocol/<suite>/<commit-topic>.md`. Associate it through the stable plan identifier, proposed subject, and test paths. A future commit has no SHA; do not require one. Link the document from the plan and commit body. The document supplements the standalone explanation required by [PR-MSG-STANDALONE](pull-request.md#pr-msg-standalone).

The document must explain the protocol requirement, original scenario, cause or uncertainty, code correction, and every included protocol test. Explain the test setup, stimulus, expected exchanges or state, assertions, result before and after the change, and coverage limits. Use the human audience and explanatory style in [DR-EXPLAIN](documentation.md#dr-explain). A test list or command log alone is insufficient.

Include as many diagrams as the explanation needs. Show relevant participants, exchanges, event order, timer boundaries, or state transitions. Use separate diagrams when distinct scenarios or before-and-after behavior need separate explanations. Do not impose a diagram count. State why diagrams add no useful information if none are necessary.

Check the diagram rendering and its agreement with the test. Report unavailable rendering as unverified.

Keep planned results separate from executed evidence in both artifacts. Record test results under [TR-FOCUSED-EVIDENCE](#tr-focused-evidence) when execution occurs. A plan-only task requires concrete test designs and explicit future document tasks; it does not authorize code or test implementation.

*Enforced at T4 — review of each planned protocol commit, its test changes, and its reviewer document.*

### TR-FP-NOT-ENOUGH

**A fingerprint is never the only test of new behavior.**

A fingerprint says the trajectory differs from the recorded one. It cannot say which of the two is right, and for new behavior there is no recorded one at all — the first baseline simply records whatever the new code does, including its bugs. Fingerprints are the net that catches an *unintended* change; the claim needs its own test. See [REJ-09](../design/rejected-designs.md#rej-09) for the case in full.

*Enforced at T4 — agent review.*

### TR-VALIDATE-EXTERNAL

**A claim about the real world is checked against something outside INET.**

An analytical result, a measurement, a reference implementation, or a figure from the standard or a paper. A model checked only against itself is checked against nothing: it will reproduce its own error exactly. The external reference and the tolerance both go in the test, so a later reader can judge whether the agreement is good.

*Enforced at T2 — the validation suite; T5 for whether the reference is the right one.*

## Recorded expectations

### TR-BASELINE-DELIBERATE

**A recorded expectation changes only after its exact scope and reason receive explicit approval, never as a side effect of another change.**

A fingerprint in `tests/fingerprint/store.json`, a statistical baseline, a chart baseline, an expected output: each is a claim that *these values are correct*. Regenerating one to make CI green is the single fastest way to lose every regression guarantee the suite provides, and it is invisible in a large diff. Before regeneration, present the specific baselines or configurations that would move, the behavior that caused the movement, and why the new values would be right; the human responsible for accepting the change approves that proposal explicitly. Approval of the source change is not implicit approval to rewrite its recorded expectations.

The defence continues in the commit message: the commit that moves the values states which behavior moved and why the new values are right ([PR-SPLIT-BASELINE](pull-request.md#pr-split-baseline)).

The statistical and chart baselines are in other repositories. There, the proposal is a branch or a pull request of the baseline repository. Its commits are candidates until the maintainer approves them in that place, apart from the approval of the source change. The author records the values with the update commands, on their own machine.

*Enforced at T5 for explicit approval; T4 for review of the message against the moved values; T3 for a baseline-only commit that gives no reason.*

### TR-BASELINE-PROVENANCE

**A baseline change names the commit that causes it and the reason the new values are right.**

"Fingerprints updated" is not a reason. Name the change, say which behavior moved, and say why the new trajectory is the correct one — a standard clause, a repaired defect, a deliberate model change. Two years later this line is the only thing that can tell a bisecting developer whether the change was intended.

**Account for every row that moves.** Not the set as a whole: each row, traced from the change to the behavior that row records. A row that moves and cannot be explained from the diff is the finding — **an unexplained row is an unintended change until somebody shows otherwise.** A fingerprint says only that the trajectory differs; it cannot say which of the two is right, so the explanation is the only thing standing between a deliberate change and a regression that rode along with it.

Rows that share one explanation are named together. *"The 31 rows under `examples/inet/` all carry TCP traffic with the default algorithm, which this commit changes from `TcpReno` to `TcpCubic`"* is one explanation for 31 rows and it is complete. What is not complete is a count.

**The commit that changes a baseline holds a reason for each category of its changes.** A tool computes the categories, and the author writes a reason on each line, after " — ":

```
Baseline changes:
  statistical tests changed, large (44; examples/manetrouting 19, ...): queueLength, packetDrop and 1104 more — a reception can draw two random numbers, so the drops and the queues move
  fingerprint runs added (3; examples/foo 3) — the commit adds the example foo
```

| Baseline | The categories, one line each | The commit that holds the list |
| --- | --- | --- |
| Statistics | test files added, removed, renamed (equal rows) or changed; a changed test file goes to the line of its size class and of the group of its moved row names | the baseline commit in `statistics` |
| Charts | images added, removed, renamed (equal bytes) or changed | the baseline commit in `media` |
| Fingerprints | runs added, removed, renamed (equal fingerprints) or changed (one line for each pattern of changed ingredient sets); ingredient sets added or removed; a new limit or expected result | the INET commit that changes `tests/fingerprint/store.json` |

The size class of a changed test file comes from the largest relative change of its rows:

| Class | Largest relative change |
| --- | --- |
| large | 1 % or more, a change from or to zero, a change that is not finite, or a changed text |
| minor | less than 1 % |
| noise | at most 1e-9: the level of the differences of another compiler or C library |
| rows | no value changes; rows are only added or removed |

[baseline_summary.py](../enforcement/baseline_summary.py) computes the list of statistics and charts, and [fingerprint_moves.py](../enforcement/fingerprint_moves.py) the list of fingerprints. More prose in the message is welcome, but the list is the part that a gate checks.

**In a baseline repository, the commit message also holds the link.** `INET-Subject:` is the exact subject of the cause commit: the INET commit that has the pin trailer. A hash cannot be the link, because the hash changes when the change lands with a rebase. A re-record whose cause is on the target branch already uses `INET-Commit:` with the hash. `Attribution:` names one of the forms of [TR-BASELINE-ATTRIBUTION](#tr-baseline-attribution).

The INET commit keeps its own task: its message says which behavior changes and why the new behavior is correct. `git blame` on a baseline file then shows the commit that explains its values.

*Enforced at T3 — [check-baselines.sh](../enforcement/check-baselines.sh): the list is the computed one, each line has a reason, and a baseline commit has its link; T4 — agent review of the reasons against the diff, line by line.*

### TR-BASELINE-COMMIT

**A baseline update travels with the commit that changes the behavior. It stands alone only when no single commit causes it.**

The rule is [PR-SPLIT-BASELINE](pull-request.md#pr-split-baseline); it is repeated here because it is a test rule as much as a commit rule. The test suite is the reason for this form. A commit that moves a trajectory and leaves the recorded values behind fails its own fingerprint test, so `git bisect` over the suite names it as the first bad commit and teaches the next developer something false.

A re-record that no single commit causes — a compiler, tool or solver version change, or drift from outside the branch — keeps a commit of its own, because there is no commit beside it to hold the reason.

A baseline in another repository cannot be in the INET commit. The pin trailer of [TR-BASELINE-PIN](#tr-baseline-pin) takes its place: the commit that changes the behavior has the pin trailer.

*Enforced at T3 — a baseline-only commit that stands directly after the source commit that moves the values, and a pin trailer in a commit with no source change; T4 for the reason itself.*

### TR-BASELINE-PIN

**A commit that moves a result pins its baseline commit in a pin trailer, with a full commit hash.**

The statistical and chart baselines are in the `statistics` and `media` repositories. The `baseline` entry of each kind in `inet.opp` names the key of its pin trailer: `Statistical-Baseline` and `Chart-Baseline`. The commit that causes the move, the cause commit, has the trailer in its message, and `inet.opp` does not change:

```
Change: src.ieee80211 | behavior.change.fix | statistical test whatsnew
Statistical-Baseline: 0123456789abcdef0123456789abcdef01234567
```

The value is the pin: the last commit of a step of the baseline branch of the series. A step goes from the pin of the previous cause commit of the series, or for the first one from the base of the baseline branch, to the pin. It moves only forward, with no merge. The step holds at most one commit that changes results. That commit is the baseline commit of the INET commit, and it has the record of [TR-BASELINE-PROVENANCE](#tr-baseline-provenance). A commit message is a part of the hash of the commit, so the required checks cover the pin, and "Rebase and merge" keeps it.

**Not each file of a baseline repository is a result.** In `statistics`, the results are the `.sca` files. In `media`, the results are the images that a chart of an analysis file in the INET tree exports; the other files are documentation media, such as network images and videos. A commit that changes no result needs no record, and a pin can move over it. The `media` pin of an INET commit thus also names the documentation media that the commit goes with.

The baseline branch starts on the head of `master` of the baseline repository. Before the landing, the pinned commit must be in the baseline repository of the project: on a branch with the name of the INET branch, or as the head of a pull request there.

**The landing job lands the baseline.** After each push to INET `master`, [check-baseline-masters.yml](../../../.github/workflows/check-baseline-masters.yml) puts the steps of the new cause commits onto `master` of each baseline repository, the oldest first. A step that starts on the head of `master` lands by a fast-forward, and GitHub marks a pull request whose head is the pin as merged. The job makes another step again on the head of `master`, with its messages and authors, as "Rebase and merge" does; GitHub does not mark the pull request of that step as merged, so the log and the job summary of the job name it, and a maintainer closes it. After the steps, the job adds a mark: a commit with no change of files and a line `Baseline-Of: <INET hash> <baseline hash>` for each landed cause commit. Thus `master` of a baseline repository always holds the baseline of INET `master`, and its history is a line.

**Each INET commit has one exact baseline.** opp_repl finds it from the two histories: the baseline that the newest mark gives for the merge base of the commit with INET `master`, and the pin trailers after that merge base, rebased locally onto that baseline. A commit of `master` gets the baseline that its mark gives. A pull request is tested against the baseline that its landing will make. A commit of a series is tested against the baseline of its own base. A commit from before the trailer form has a `ref` in `inet.opp`, the pin of that time, and keeps it. A new commit has no `ref`. CI never reads a baseline that moves, such as the head of a branch.

**An overlap blocks the landing.** Two series that move the same baseline land one after the other with no new push, when no file is common to them. A file that the baseline branch changes and that `master` of the baseline repository changed since the base of the branch is an overlap: two series moved one result two times, and a merge of the text can mix the values of two runs. The commit status `Baseline merge` of a pull request with an overlap is red, and a pull request with a red `Baseline merge` does not land. The author records the common results again on the new head of `master` and writes the new pins with `baseline_pins.py retrail`. If a landing stops, the job fails, and INET `master` has no baseline for that commit until a maintainer repairs it ([change-a-baseline.md](../guide/change-a-baseline.md)).

*Enforced at T3 — [check-baselines.sh](../enforcement/check-baselines.sh): the form of each pin trailer, the step forward with at most one commit that changes results, and the presence of the commit in the baseline repository; [baseline_masters.py](../enforcement/baseline_masters.py) `check`, the status `Baseline merge` of each pull request: no overlap with `master` of the baseline repository; `baseline_masters.py`, after each push to `master`: the landing of the steps and the marks; T2 — the statistical and chart tests run against the baseline of each commit.*

### TR-BASELINE-PARENT

**A baseline changes only for a test that passes on the parent commit and that the change moves.**

A test that fails on the parent has an earlier difference. An update of its baseline hides that difference under the reason of the new change. Such a test keeps its baseline until its own repair, and the record of the new change names it as out of scope.

Regenerate only what moves. A blanket regeneration takes in drift that has no relation to the change. A baseline file whose values do not change is not a part of a baseline commit. A result file with another text for the same numbers is such a file.

*Enforced at T3 — [check-baselines.sh](../enforcement/check-baselines.sh): each changed result file of a baseline commit in another repository differs in a value; T2 — [check-baseline-attribution.sh](../enforcement/check-baseline-attribution.sh) runs the moved tests at the parent commit, for each pull request and by hand; T4 for an in-tree baseline.*

### TR-BASELINE-ATTRIBUTION

**The code that moves baseline values is the smallest code fragment that produces them, and a run proves it.**

A reviewer accepts new baseline values because the reviewer can read a small change and see that it is correct. Reasons are not sufficient when the cause is a large commit: the reader cannot see which part of the commit moves the results. A commit that changes 100 lines and moves 500 baseline entries is wrong when 10 changed lines move the same 500 entries. Then the 10 lines are the commit, and the other 90 lines go into a commit before it that moves no baseline. Or a reduction shows the 10 lines.

**The number of moved entries is no measure.** A change can move all fingerprints, charts or statistics for a good reason; the size of its code fragment is the measure.

The `Attribution:` trailer names one form. For statistics and charts it is in the baseline commit, and a reduction is a file in its folder `attribution/`. For fingerprints it is in the INET commit, and a reduction is the patch after a line `Reduction:` in its message; with no trailer, the form is `commit`, or the form of the baseline commits that the same INET commit pins.

| Form | Claim | Proof |
| --- | --- | --- |
| `commit` | The INET commit that has the pin trailer is the small change. | The moved tests pass at its parent against the baseline of the parent, and at the commit against its own baseline. |
| `forward <file>` | The patch applies to the parent of the INET commit, and the parent with the patch gives the new results. | The moved tests pass at the parent with the patch, against the baseline of the INET commit. |
| `reverse <file>` | The patch applies to the INET commit, and the commit with the patch gives the old results. | The moved tests pass at the commit with the patch, against the baseline of the parent. |
| `first-record` | The INET commit adds the test. No earlier values exist. | The baseline commit only adds files. |
| `removal` | The INET commit removes the test. | The baseline commit only removes files. |
| `environment` | No source change is the cause: a new version of OMNeT++, of a tool, of a compiler or of a library. | The INET commit changes no source, and its message names the version change. |

Prefer `commit`. [PR-SPLIT-PREPARE](pull-request.md#pr-split-prepare) gives that form: the series first prepares with no change of results, and a last, small commit turns the behavior on. Write a reduction only when that commit cannot be small. A reduction is an experiment. It does not have to be code that can land. One INET commit has one attribution: its baseline commits, and its own message when it also moves fingerprints, name the same form and the same patch. The forms `first-record`, `removal` and `environment` are for statistics and charts; a fingerprint change that only adds, removes or renames entries, or that changes no source line, needs no form.

**Small has a number.** The limit counts the changed lines of the files that can change a result: the model source, the simulations and the Python of the project, without documents and images. It applies to the INET commit of the `commit` form and to a reduction, for each kind of baseline. Above 100 lines the gate fails. Above 30 lines the gate gives a note, and the reviewer asks if a smaller change shows the same move. The numbers come from `master`: of the 70 commits between 2026-09-22 and 2026-11-11 that move a fingerprint baseline together with a source change, 71 % change 30 lines or fewer, and 87 % change 100 lines or fewer.

*Enforced at T3 — [check-baselines.sh](../enforcement/check-baselines.sh): the form, the two size limits, a reduction that applies, files that only come or only go, and one attribution for each INET commit; T2 — [check-baseline-attribution.sh](../enforcement/check-baseline-attribution.sh) runs the moved tests that prove the form, for each pull request and by hand; T5 — whether the small change is easy to understand.*

### TR-BASELINE-REPOSITORY

**The baseline commits of a change are on a branch of `statistics` or `media`, and only the landing job, a maintainer and an admin move its `master`**

The branch has the name of the INET branch, or the commits are on a pull request from a fork. After the landing in INET, the landing job puts each step of a pin trailer onto `master` of the baseline repository, by a fast-forward or a rebase, and adds a mark commit. At an overlap the landing job fails, and a maintainer repairs the baseline by hand and adds the mark. The baseline repositories have no workflow and no token of their own.

*Enforced at T2 — a ruleset on `master` of each baseline repository lets only its bypass actors update it: the deploy key of the landing job, the roles maintain and admin, and the organization admins.*

## Determinism

### TR-DETERMINISTIC

**The same seed gives the same trajectory: on every platform, in every build mode, at every thread count.**

Determinism is the precondition for every other guarantee in this document. A fingerprint over a non-deterministic model is noise, a bug report that cannot be reproduced cannot be repaired, and a published result that nobody can re-run is not a result. Anything that varies between runs — iteration over a container ordered by pointer value, a time or a hostname read at run time, a floating-point path that depends on thread count — is a defect, not a quirk.

*Enforced at T2 — fingerprint stability across seeds, modes and platforms.*

### TR-NO-AMBIENT

**A model draws only from the random number generators the kernel gives it.**

No `rand()`, no `std::random_device`, no static generator of its own. The number and the order of the draws are part of the result: a model that takes one extra draw in a branch changes every later draw in the run, so a refactor that "cannot change behavior" does.

*Enforced at T3 — a grep for the forbidden generators; T2 by fingerprint drift.*

### TR-SELF-CONTAINED

**A test uses only its declared inputs and test infrastructure, and does not depend on another test's execution.**

No file from another test's output, no fixed absolute path, no network, and no dependence on the order the suite happens to run in. A test that passes only after its neighbour has run is a test that will fail alone, in parallel, and on the machine of whoever is trying to reproduce a defect.

Declared infrastructure includes the matching INET build, shared suite fixtures and support libraries, and a separately provisioned statistical baseline checkout. Provision these before execution and record their versions; test execution must not fetch an unspecified network input. Generated work directories, logs and comparison diagnostics may be retained as evidence, but must not become undeclared inputs to later tests.

*Enforced at T2 — the suite runs in parallel and in isolation.*

## CI

### TR-CI-EVERY-COMMIT

**CI builds and tests every commit of a branch, not only its head.**

`git bisect` is the fastest tool for a regression nobody can explain, and one broken middle commit makes it useless. This is the mechanical half of [PR-SERIES-BUILDS](pull-request.md#pr-series-builds), which is otherwise a rule that nothing checks.

*Enforced at T2 — not yet configured; the workflows test the head only.*

### TR-SELECTED-PASS

**Before a push, the tests that the change-based test selection gives for the series pass, for each kind of test that the test map holds.**

The selection of opp_repl ([doc/test_map.md](https://github.com/omnetpp/opp_repl/blob/main/doc/test_map.md)) reads a test map, which records the files, functions, NED types and ini sections that each test used. It leaves out a test only if the series changes nothing that the test used, so the outcome of the test cannot change. The selected tests thus show that the series breaks no other test. They do not show that the change itself is correct, which still needs its directly related cases under [TR-FOCUSED-EVIDENCE](#tr-focused-evidence).

A kind of test that the map does not hold gets no obligation from this rule. A change of a file that the CI jobs run, a test that the map does not know, and a difference of the environment from the map select all tests of the kind, or that test. Report the line `Selected N of M` of each kind, the map commit, and the reason of each kind for which all tests are selected. [run-the-gates.md](../guide/run-the-gates.md#before-every-push) has the commands.

*Enforced at T2 — the fingerprint jobs of a pull request run the selected fingerprint tests. A push to `master` runs the selected tests and a random sample of the other tests, whose records must agree with the map (check V5); a run of `master` waits for the run before it, and a newer push cancels a run that waits, because the newer run also selects for the changes of that push, and the full run of each night runs all tests and checks the selection against the previous full run (check V4).*

### TR-CI-SELECTION

**A test job of a pull request runs only the tests that the change can affect, and it reports `Selected N of M`**

The selection compares the change with the test map of the newest finished push run whose commit is an ancestor of the pull request, or else with the weekly map, and it uses the commits of omnetpp and opp_repl that the header of the map names. The job reports `Selected N of M`, also for N = 0. With the label `full-tests`, it runs all tests of its kind. A change in `_scripts/github/` selects all tests, a change of the workflow of a kind selects all tests of that kind, and a change of another workflow selects all tests of each kind that does not name its workflow in `inet.opp`.

The job selects before the build, and with `Selected 0 of M` it skips the build. That needs a list of all tests of the kind without a build, because the selection always selects a new test that the map does not know. The fingerprint tests have this list in `tests/fingerprint/store.json`. A kind with no such list, such as the statistical and chart tests, uses a list of the paths that can change a result before the build.

*Enforced at T2 — the fingerprint jobs select their tests, and the statistical and chart jobs read their list of paths. The opp tests run only on `master` until the test selection gives them jobs for pull requests.*

### TR-CI-MERGE

**A test job of a pull request tests the merge with `master`, against the baseline that its landing will make**

That baseline is `master` of the baseline repository with the steps of the pin trailers of the pull request on it. A test of the head alone can pass on a pull request and fail after the landing, when `master` changed since the branch started.

*Enforced at T2 — the statistical and chart jobs check out the merge, and `opp_checkout_test_baselines` makes the baseline of the landing.*

### TR-CI-ONLY-TEST

**A test job only tests: it records no baseline and uploads no candidate values**

The author records a baseline on their own machine ([TR-BASELINE-COMMIT](#tr-baseline-commit)). A workflow that writes baselines lets a red test turn green with no person who looks at the change.

*Enforced at T4 — review of the workflows.*

### TR-CI-FULL-RUN

**Each push to `master` runs the test jobs of each kind, and the full run of the head of `master` decides `Master green`**

The full run is the newest attempt of each of the ten workflows of a push to `master`: the landing job `Check: baseline masters`, the six kinds of opp tests, and the fingerprint, statistical and chart tests. A scheduled run and the weekly runs do not count, and the checks V4 and V5 of the test selection, in the job `Test map`, do not count. A cancelled run is red. A red counts also when its cause is a new omnetpp or opp_repl; the status then names the commits of both, for this run and for the newest green commit of `master`. `Master green` follows a new attempt of a failed job: run a job again only for a red that the environment causes, such as a failed runner, never for a flaky test ([TR-FLAKY](#tr-flaky)). A trivial landing ([PR-LAND-TRIVIAL](pull-request.md#pr-land-trivial)) starts no full run, and `Master green` keeps the state of the commit before it.

*Enforced at T2 — [master_green.py](../enforcement/master_green.py), through the workflow `Check: master green`.*

### TR-CI-FEATURE-MATRIX

**A feature is built with its neighbours switched off, not only in the full build.**

The full build hides every accidental dependency, because everything a model reaches for happens to be present. The feature-off build is the only check that [AR-EXT-FEATURES](architecture.md#ar-ext-features) and the dependency direction of [AR-ORG-DOMAINS](architecture.md#ar-org-domains) are still real.

*Enforced at T3 — the feature workflow and `inet_featuretool`.*

### TR-FLAKY

**A flaky test is repaired, or disabled with an issue number. It is never re-run until it passes.**

A re-run until green teaches the whole project that a red CI means nothing, and from then on a real regression is waved through with the same reflex. A test that is flaky is either testing something non-deterministic — which is a model defect under [TR-DETERMINISTIC](#tr-deterministic) — or testing it wrongly. Both are worth finding.

*Enforced at T5 — human discipline; nothing can check this one.*

## Required checks of master

### TR-REQ-GREEN

**Each required check of `master` is green before a landing, and each comes from GitHub Actions**

The ruleset `master: required checks` of GitHub is the source of the list, and the section "Required checks of master" of [the enforcement README](../enforcement/README.md#required-checks-of-master) is its copy for the reader. The checks are `Mechanical audit`, the gates of the audit; `Baseline merge`, which says if each baseline branch can land; `Attribution runs`; the test jobs; `Enforcement checker tests`; `Source seal policy`; and `Master green`. Each must come from GitHub Actions, so a status from the token of a person does not count. Limit: a workflow on any branch can post such a status, so the rules stop mistakes, not a person who wants to break them.

*Enforced at T2 — the ruleset of `master`.*

### TR-REQ-REPORTS

**A required job reports on each pull request, and a change of its name lands together with the change of the ruleset**

Its workflow has no path filter in its pull-request trigger. A job with a matrix has no job-level `if:`: GitHub gives a skipped matrix job the name without its matrix values, so its context never reports. When the job has nothing to test, its steps skip and it reports green. A job-level `if:` on another required job is true for each event of a pull request, because a skipped job counts as passed. A change that renames or removes a required job, or changes its matrix, needs the change of the ruleset by an admin at the same landing; else each pull request waits for ever. A new job becomes required only after it reports on pull requests.

*Enforced at T3 — [check-required-contexts.sh](../enforcement/check-required-contexts.sh) in the pull-request audit.*
