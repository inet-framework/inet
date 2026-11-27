# Run the gates

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [enforcement/README.md](../enforcement/README.md), [testing.md](../rule/testing.md)

What to run during focused development and before a push, in order, and what each result must say. The full inventory and the tier ladder are [enforcement/README.md](../enforcement/README.md). Run everything from the repository root unless a test command explicitly changes directory.

## Keep the tested library current

After compiled INET source or generated-code inputs change, build the INET library in the mode the test will load before running the test. This prevents a green result from describing an older `libINET_dbg` or `libINET` than the changed source tree:

```bash
make -j$(nproc) MODE=debug
# Before a release-mode test:
make -j$(nproc) MODE=release
```

Debug mode is the primary mode for local behavioral validation because it retains runtime assertions. Release mode remains a required compilation check before a contributor pushes; run behavioral tests in release mode as well when the changed contract or defect is mode-sensitive. A test-definition, script, configuration or documentation-only change does not require rebuilding the INET library unless it also changes compiled support inputs. Always record the mode and any required build status with the test result.

## During focused development

Map each changed path, symbol or behavioral contract to the directly related cases in the category required by [TR-CAT-MATCH](../rule/testing.md#tr-cat-match). Run those cases through an explicit case, tag or filter; for example:

```bash
doc/project/enforcement/check-architecture.sh src/inet/<affected-subtree>
opp_run_fingerprint_tests -f '<affected-case-regex>'
# Use the category runner's explicit selector for unit, module, protocol and other tests.
```

For protocol cases, follow the suite and test-file selection and build instructions in [AUTHORING.md](../../../tests/protocol/lib/AUTHORING.md#10-running-tests). For recorded statistical results, select the related simulation directories/configurations with `opp_run_statistical_tests -m debug -w '<simulation-directory-regex>' -c '<config-regex>'`. Use release mode when reproducing the statistical CI workflow, and report that mode explicitly.

Keep the working directory, exact build and test commands, configuration, run and seed where applicable, filter, exit status and artifact paths. An unfiltered category or repository-wide suite may add integration coverage, but it cannot replace a directly related case. If no such case exists, report the coverage gap instead of broadening the command until something green appears ([TR-FOCUSED-EVIDENCE](../rule/testing.md#tr-focused-evidence)).

The change-based test selection of opp_repl ([doc/test_map.md](https://github.com/omnetpp/opp_repl/blob/main/doc/test_map.md)) helps with the mapping. It reads a test map, which records what each test used. Get the map of the newest push or scheduled run of `master` first, and then ask which tests the change can affect:

```bash
# the map of the newest CI run of master whose commit is an ancestor of origin/master, into coverage/test-map/map
GITHUB_REPOSITORY=inet-framework/inet _scripts/github/get-test-map.sh fingerprint-tests.yml fingerprint origin/master coverage/test-map/map
opp_select_tests --changes origin/master...worktree --explain --ignore-environment
opp_show_test_map src/inet/<path>.cc --line <line>   # the tests that ran the function at this line
```

The selection lists each test that the change can affect, with the rule and the changed place that select it. Pick the directly related cases from this list. The local omnetpp, opp_repl or compiler is usually not the one of the map, and then the selection gives all tests. Thus a local selection adds `--ignore-environment`, which gives a warning for each difference instead, and the report says so. CI never uses this option.

A dry run previews selection; it supplies no executed-test evidence. Report a zero-case selection as not run. Distinguish observed passes, declared expected failures, unexpected outcomes and setup errors even when the runner exits successfully. For protocol expectations, use the failure classes in [derive-tests-from-a-standard.md](derive-tests-from-a-standard.md#the-class-of-a-failure-and-when-to-declare-it-expected).

Statistical and chart comparisons read the checkouts `statistics` and `media`. `opp_checkout_test_baselines` puts them at the baseline of the commit: the ref of `inet.opp` for an older commit, and else the baseline that follows from the pin trailers of the series and the marks on `master` of each baseline repository. A test gives a warning when a checkout does not have the files of that baseline. Statistical comparisons also record the baseline repository revision and local changes. For a base-versus-feature comparison, use the same baseline revision and environment on both sides; advancing the baseline during the comparison changes the experiment. Diagnose missing baselines separately from changed simulation results. Changes to recorded expectations follow [change-a-baseline.md](change-a-baseline.md).

## Before every push

The final pass keeps the focused evidence and adds the project-wide compilation and mechanical gates. It does not turn every local validation into an unfiltered test campaign.

```bash
# 1. compile the source in both supported modes
make -j$(nproc) MODE=debug && make -j$(nproc) MODE=release

# 2. run the rules a script can check
doc/project/enforcement/check-architecture.sh
doc/project/enforcement/check-naming.sh --base origin/master
doc/project/enforcement/check-pull-request.sh --base origin/master  # the series gates, as CI runs them
# when the series has a pin trailer, before the baseline branch is pushed:
doc/project/enforcement/check-baselines.sh origin/master..HEAD --baseline-repository statistical=statistics --baseline-repository chart=media
# and after the push: can the baseline branch land with no overlap (the status "Baseline merge")
python3 doc/project/enforcement/baseline_masters.py check HEAD --base origin/master
doc/project/enforcement/check-interfaces.sh                # every I<Stem> class holds no body
doc/project/enforcement/check-source-seals.sh --base origin/master

# 3. rerun each recorded, explicitly filtered test command against a fresh matching library

# 4. run the tests that the selection gives for the series, for each kind that the map holds
opp_run_fingerprint_tests --changes origin/master...HEAD --test-map coverage/test-map/map --ignore-environment
```

Step 4 is [TR-SELECTED-PASS](../rule/testing.md#tr-selected-pass). The map of a CI run holds the fingerprint tests. Report the line `Selected N of M`, the map commit, each environment difference that `--ignore-environment` ignored, and the reason when all tests are selected.

CI, integration and release campaigns may run broader suites and build matrices. Their result is additional evidence; it does not erase the focused change-to-test mapping. The fingerprint jobs of a pull request run only the selected tests, in the environment of the map; the label `full-tests` runs all of them, also for a change that needs a newer omnetpp or opp_repl than the map. A push to `master` runs the selected tests and a random sample of the other tests, and updates the map; the runs of `master` wait for each other, and a newer push cancels a run that waits; each night when `master`, omnetpp or opp_repl changed, a scheduled run runs all tests and makes a full map.

## A trivial change

A trivial change goes to `master` with no pull request ([contribute-a-change.md](contribute-a-change.md#8-submit-a-reviewable-change)). The push check decides, with the gate that you can also run first:

```bash
doc/project/enforcement/check-trivial-change.sh --base origin/master  # TRIVIAL, or NOT TRIVIAL with the reasons
git push origin HEAD:topic/<name>      # the job "Push check" marks the required checks of a trivial change
git push origin HEAD:master            # the same commit, after the push check is done
```

GitHub refuses the push to `master` when a required check of the commit is missing; the summary of the push check gives the reasons. A push to a branch other than `master` starts no build and no test. A trivial landing on `master` starts no full run, and `Master green` keeps the state of the commit before it (M9), so it does not block the next landing.

## When the change touches this document set

```bash
doc/project/enforcement/check-seals.sh          # the seal flags and the generated index
doc/project/enforcement/check-seals.sh --write  # ... and rewrite the index
```

## When the change is large or touches C++ broadly

```bash
doc/project/enforcement/check-cpp.sh src/inet/<subtree>
```

It needs a compilation database; build first, or set `INET_COMPILE_DB`.

## Scope a gate to your subtree

A scoped run is the one worth doing often while developing:

```bash
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ethernet
doc/project/enforcement/check-naming.sh src/inet/linklayer/ethernet
```

The naming command scans declaration names in every NED and MSG file under the subtree. The scoped run is not the final project-wide architecture and naming pass before a push.

## What none of them cover

The gates cover dependency direction, C++ identifiers, file names and the commit series. They do not cover the **semantic** project rules: substitutable operation outcomes, object ownership, externally exposed enum values or named numeric codes, visualization logic inside a protocol, a zero-time message standing in for a call, or a test whose category does not match its claim. Those are tier 4, and they need the [agent-review checklist](../enforcement/checklist/general.md).

The pull-request `enforcement-tests.yml` workflow runs the enforcement checker tests. `check-sealing.yml` evaluates paths sealed at the branch merge base and, when necessary, waits for the protected approval required by [SR-PR-APPROVAL](../rule/sealing.md#sr-pr-approval). `check-pull-request.yml` runs the series gates of `check-pull-request.sh` over every pull request to `master`: the commits, the `Change:` trailers, the includes and parameters of each commit, and the description. The tree gates above still depend on contributors running them by hand. They are the next CI coverage gap to close, and they need a comparison with the merge base first, because `master` has old violations.
