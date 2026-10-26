# Implementation plan: INET tests on opp_repl

Checkout and revision: INET `origin/master` at `ef0122b14e`, branch `topic/tests-on-opp-repl` in the worktree `/home/levy/workspace/inet-tests-on-opp-repl`. opp_repl `main` at `a502e9d` (`opp_ci` and `github_ci` at the same commit).
Plan revision: 2026-10-26, second draft (decisions D1 to D5 recorded).
Approval: 2026-10-26, the maintainer in the chat: "implement this plan, ask if design decision comes up", after the decisions of section 8. The approval covers this plan revision. Each push and pull request, a baseline change and a seal edit are asked when a step needs them.

## 1. Problem and intended behavior

INET keeps a second copy of opp_repl's test framework in `python/inet` (about 9600 lines), with its own command-line wrappers in `bin/`. Eight GitHub workflows still call that copy. The two copies drift apart: opp_repl received the fixes of the fingerprint work (for example the ini parser that lost every value with a comment after it), and INET's copy still has the faults (`python/inet/simulation/project.py:357`). A developer who runs a test on the command line and a CI job can therefore select different simulations.

The intended result:

- Each GitHub workflow runs an `opp_run_*` command of opp_repl, as the fingerprint workflow does since `1e28c15273`.
- `inet.opp` at the INET root describes INET for opp_repl: folders, features, test kinds, baselines. A change of INET's test setup lands with the INET change.
- `python/inet` keeps only INET-specific code. The `bin/inet_run_*` wrappers, `inet_python_repl` and `inet_build_project` go; the migration guide names each replacement.

## 2. Current implementation and evidence

### 2.1 The workflows (master `ef0122b14e`)

| Workflow | Command now | Mode | opp_repl command |
| --- | --- | --- | --- |
| unit, module, packet, queueing | `inet_run_<kind>_tests` | debug | `opp_run_<kind>_tests` |
| protocol | `inet_run_protocol_tests`, before it a packetdrill check with `inet_run_packetdrill` | debug | `opp_run_protocol_tests`; the packetdrill check stays |
| feature | `tests/features/featuretest`, 16 slices with `SPLIT_TOTAL/SPLIT_INDEX` | `MODE` | `opp_run_feature_tests --split-count 16` |
| statistical | `inet_run_statistical_tests -m release`, after a clone of `inet-framework/statistics` | release | `opp_run_statistical_tests -m release` |
| chart | `inet_run_chart_tests -m release -f showcases`, after a clone of `inet-framework/media` | release | `opp_run_chart_tests -m release` |
| validation | `inet_run_validation_tests -m release` | release | `opp_run_validation_tests -m release`, which calls `inet.test.validation:run_validation_tests` |
| fingerprint | `opp_run_fingerprint_tests --load @opp -p inet` (done) | both | — |

All workflows except fingerprint install `omnetpp/python/requirements.txt` first, which pins `pandas<3` and `numpy<3`. opp_repl's chart test needs that pin (pandas 3 crashed six unit-less charts on opp_ci in July).

### 2.2 Differences between the two runners

The two implementations are near-copies. Four surveys of the code (2026-10-26) and checks of the critical lines found these differences that can change a verdict or the set of tests:

- **Mode.** opp_repl's statistical, chart and feature tests default to debug (`opp_repl/test/statistical.py:263`, `chart.py:216`, `feature.py:118`); INET's default to release. The workflows already pass `-m release`, and `inet.opp` can set the default.
- **Unbounded configs.** opp_repl skips a simulation without a limit as "Unbounded simulation" (`opp_repl/simulation/task.py:509`); INET's runner has no such rule and runs it. On master, 24 concrete configs have no limit and no `bounded` marker: 22 QUIC configs, `examples/inet/hierarchical99 General` and `examples/inet/ipv4hook General`. All 24 have statistical baselines. The old branch's `c0046371c2` marks only 4 QUIC ini files with `# bounded = true`, and master does not have it.
- **opp_test kinds.** Discovery (`**/*.test` outside `work/`), the opp_test steps and the expected-result rules are the same. opp_repl gives a wider NED path (all project NED folders) and include path, does not write INET's `makefrag` with `-Wno-overloaded-virtual`, always links `tests/protocol/lib` for a protocol suite, and counts a build failure of a test that expects ERROR as expected. Unit, module and queueing tests sit flat in their folders, so the working directory is the same.
- **Feature test.** The workflow's `featuretest` is a shell script that builds each feature alone and without the feature, and runs one dummy network. opp_repl's feature test builds the same cases and runs the run-0 simulations of the feature's own packages: a stronger test, and a slower one. It has no skip list, where the script skips `VoipStream` and the `_examples` features. opp_repl's `get_package_folder()` matches `opp_repl.examples` where INET's matches `inet.examples` (`opp_repl/test/feature.py:155` against `python/inet/test/feature.py:132`), so each example feature falls back to `examples/empty`.
- **Validation.** `python/inet/test/validation.py` imports INET's generic layer (`inet.project.inet`, `inet.simulation.project`, `inet.test.simulation`) and uses `inet_project` in ten places. The old branch rewrote it to import from opp_repl.
- **Chart update.** `opp_update_chart_test_results` does not apply the project defaults, so it needs `-f showcases` (INET's update has no default either).

### 2.3 Consumers of `python/inet`

- The `.anf` chart scripts of `examples/ethernet/TenBaseT1S` and `showcases/tsn` import `inet.scave.plot` and `inet.scave.plca`; `inet.scave` must stay on `PYTHONPATH`, which `setenv` sets.
- `opp_run_validation_tests` imports `inet.test.validation`.
- `doc/src/conf.py` runs autoapi over `python/inet` when the Python API part is built.
- `inet.cffi` has users only inside the generic layer (`simulation/task.py:388`, `simulation/optimize.py:46`).
- `bin/`: the wrappers `inet_build_project`, `inet_python_repl`, `inet_run_all_tests`, `inet_run_chart_tests`, `inet_run_feature_tests`, `inet_run_module_tests`, `inet_run_packet_tests`, `inet_run_protocol_tests`, `inet_run_queueing_tests`, `inet_run_release_tests`, `inet_run_sanitizer_tests`, `inet_run_self_tests`, `inet_run_serializer_tests`, `inet_run_simulations`, `inet_run_smoke_tests`, `inet_run_speed_tests`, `inet_run_statistical_tests`, `inet_run_unit_tests`, `inet_run_validation_tests`; the legacy tools `inet_diffcharts` (replaced by `opp_diff_charts`), `inet_diffingerprints`, `inet_ssh_cluster_python`, `inet_start_jekyll_server`; INET's own tools `inet`, `inet_dbg`, `inet_release`, `inet_coverage`, `inet_profile`, `inet_sanitize`, `inet_gdb`, `inet_gdb_dbg`, `inet_nemiver`, `inet_nemiver_dbg`, `inet_valgrind`, `inet_root`, `inet_version`, `inet_selfdoc_json2xml`, `inet_run_packetdrill`.
- `inet_run_serializer_tests` runs `tests/serializer` with opp_test; no workflow calls it, and opp_repl has no kind for it, but `opp_run_opp_tests` runs the opp tests of the current folder.

### 2.4 Project descriptions

opp_repl loads `--load` files and folders, `@opp` for its bundled files, or every `*.opp` of the current folder; a later description with the same name replaces an earlier one (`opp_repl/simulation/workspace.py:117`). opp_ci already runs `--load @opp --load "$INET_ROOT" -p inet` (`opp_ci/executor.py:1602, 1674, 1983`), so an `inet.opp` in the INET root replaces the bundled copy there without an opp_ci change. mm1k already keeps its `mm1k.opp` in its repository with `root_folder="."`. opp_ci's host-side baseline lookup (`_resolve_test_baseline`, `executor.py:836`) may read only the bundled copy; step S0 checks it.

### 2.5 The old branch

`topic/migrate_to_opp_repl_master` is 1381 commits behind master. Its plan `plan/done/python-opp-repl-migration.md` classifies the modules of `python/inet` into generic (delete), mixed (trim) and INET-specific (keep); the classification still applies, but its commits do not, and its removal of all workflows is not wanted. Its `ExternalProcess` repair is on master (`b52bc21a34`); its QUIC markers are not.

## 3. Scope and acceptance criteria

Required work: steps S0 to S7, each in its own pull request except where section 6 joins them, and the opp_repl changes that they need.

Acceptance criteria:

- No GitHub workflow calls an `inet_run_*` command or `featuretest`; each one runs an `opp_run_*` command.
- For each switched workflow, the old and the new runner give the same verdict for each test on one build in the GitHub-job container, or the plan names each difference with its reason.
- `inet.opp` is in the INET root, and the workflows, the documentation and opp_ci use it.
- `python/inet` holds only `scave`, `test/validation.py`, the documentation helpers and the minimal package files. `bin/` has no tool that opp_repl replaces (decision D4).
- The developer guide, the release note and the migration guide name each replacement.

Preserved: the baselines (statistics, media, fingerprint store) do not change, except where a step names a change; the build scripts and the matrix of each workflow stay.

Optional follow-up, not in this plan: the opp_ci run of each kind on master, and an opp_repl kind for the serializer tests.

## 4. Proposed design and reasons

**INET owns its description.** `inet.opp` moves from `opp_repl/opp/inet.opp` into the INET root, with `root_folder="."`, `root_folder_environment_variable="INET_ROOT"` and `omnetpp_project="omnetpp"`, as mm1k does. The workflows load it with `--load $GITHUB_WORKSPACE/inet`, a developer with `--load $INET_ROOT` (or with no option from the INET root). The bundled copy stays in opp_repl as a fallback for INET versions without the file (decision D3); opp_ci loads the checkout's file after it, so the checkout's file wins where it exists.

**One workflow group at a time, each proved first.** Each step builds the commit in the GitHub-job container (ubuntu 24.04, clang 16, the same build scripts) in the workflow's mode, runs the old and the new command on the same build, and compares the verdict of each test. Only then does the workflow switch. The old runner stays until the last step, so each step can compare against it.

**Real limits for the configs without one (decision D2).** Each of the 24 configs gets a `sim-time-limit` in its ini file, so that every runner sees a bounded simulation. The first version of this design said that a limit at or above the natural end keeps the results; S4 found that this is wrong: with a limit, OMNeT++ stops at the limit and not at the last event, so the time averages include the time after the last event. Only a limit exactly at the natural end keeps the scalars, and such limits (for example 0.13559332s) must change with each timing change. So each config gets a round limit at or above its natural end, the limit of its run-0 fingerprint entry, and its statistical baseline moves once under TR-BASELINE-DELIBERATE.

**The remaining Python package is INET-specific.** After the last switch, the generic modules and the wrappers go. `inet.test.validation` imports from opp_repl, as on the old branch. `inet.scave` stays, because the `.anf` scripts import it.

## 5. Affected components and compatibility

- INET: `inet.opp` (new), the eight workflows, 24 ini files (markers), `python/inet`, `bin/`, `python/requirements.txt`, the developer guide, the Python package documentation, a release note and a migration guide section.
- opp_repl (`main`, then `github_ci` for INET CI and `opp_ci` for the opp_ci service): the feature test's package match and skip list, and the default of the chart update. The bundled `inet.opp` stays (decision D3); it can drift from INET's file, so a change of INET's file that matters for INET versions without the file also goes into the bundled copy.
- opp_ci: no change if its baseline lookup reads the checkout's description (S0 checks it); else a small change in S0.
- Users: the `inet_run_*` commands, `inet_python_repl` and `inet_build_project` go without aliases; the migration guide maps each one to its `opp_run_*` command. Projects that import `inet.*` generic modules (for example a project of a user that calls `inet.simulation`) must import from opp_repl.

## 6. Implementation steps and dependencies

- [x] **S0. `inet.opp` in INET.** Add `inet.opp` at the root from opp_repl's bundled copy with the header of section 4, and the release default mode for statistical, chart and validation. Switch the fingerprint workflow to `--load $GITHUB_WORKSPACE/inet`. Check opp_ci's `_resolve_test_baseline` with a checkout that has the file. Check: `opp_run_fingerprint_tests --load $INET_ROOT -p inet` gives the same tasks as with `--load @opp`. Done: `1115d5dd5f` (description and fingerprint workflow), `7acd596e1c` (documentation). Facts found: without `omnetpp_project="omnetpp"`, a description loaded without `@opp` has no OMNeT++ project, because only the bundled `omnetpp.opp` defines one; with the name, opp_repl defines it from `__omnetpp_root_dir` (`opp_repl/simulation/project.py:706`). In the container, `--load $INET_ROOT`, `--load @opp --load $INET_ROOT` and no option from the INET root each run a fingerprint test of `examples/empty` with 3 PASS, and INET's file and the bundled copy give the same 1840 runs with 4705 fingerprint tasks. opp_ci needs no change: its callers pass the job's `opp_file`, so its host-side baseline and mode lookups read the bundled copy, whose baseline entries equal INET's; and it passes `--mode` to its build stage and its test stage, so INET's mode default does not split them. The validation runner applies no project defaults, so its workflow keeps `-m release`.
- [x] **S1. unit, module, packet, queueing.** Compare the task lists and the verdicts of both runners in debug mode; examine each difference of section 2.2 (NED path, include path, `-Wno-overloaded-virtual`, build failures). Switch the four workflows. One pull request with S0. Done in `3bbbb06e21`, with opp_repl `b7b827b` (branch `topic/tests-on-opp-repl`, not pushed). Fact found: opp_repl skipped each test path that contained `/work/`, and the GitHub runner keeps the checkout under `/home/runner/work/`, so `opp_run_unit_tests` found 0 tests and exited with 0; the repair counts only the folders below the test folder, as INET's `b0c7a25e75` did. Result on one debug build: the same verdict for each of the 128 unit, 755 module and 77 queueing tests, and PASS for the packet test in both runners. The differences of section 2.2 (NED path, include path, `-Wno-overloaded-virtual`, build failures) change no verdict. Non-PASS tests in both runners: `MathFunction_1` (it fails on GitHub master too), and in the container only `ExternalProcess_1` to `4` (no `ping` and `killall`), `IPv4_dont_fragment_drop_reason` (no `gawk`) and the two `AarfRateControl` tests. The workflow commands in a fresh venv give the same results; `opp_test` needs only the OMNeT++ Python package from `setenv`.
- [ ] **S2. protocol.** The same comparison; check that `inet_run_packetdrill` finds `INETGPL_ROOT` through opp_repl, and keep the packetdrill check step. Switch the workflow.
- [ ] **S3. feature.** opp_repl first: the `inet.examples` match and a skip list in the project description (`VoipStream`, `_examples`). Then measure the time of the 16 slices, compare the cases with `featuretest`, and switch the workflow to `opp_run_feature_tests --split-count 16` (decision D1); remove `tests/features/featuretest`.
- [x] **S4. statistical.** Measure the natural end of the 24 unbounded configs, give each a real limit (decision D2), and move their statistical baselines. Then compare all statistical tests of both runners in release mode, and switch the workflow. Done on branch `topic/tests-on-opp-repl-statistical`: `938e6d3726` (ini limits) and the workflow switch; statistics `ca95f16` on the statistics branch of the same name, which goes to the statistics master with this branch. Facts found: all 24 configs stop by themselves, between 0.13s and 720.2s (release mode), and each stored fingerprint limit is at or above that time, so the ini limit is the stored limit and a run with the ini settings is the simulation of the fingerprint test. The statistics of 22 configs change in the values that depend on the end time; `examples/quic/shared_link` keeps its results, and `examples/quic/zerortt` `sendInvalidToken` keeps its old file, because it fails on master for older reasons. Before S4, master's statistical job failed for 706 of 929 configs since the push of `3fd8406ba3` and `666f0f0bc2` (2026-10-19), which changed 630 results without a statistics update; statistics `6b18bf5` (on the statistics master) updates exactly the configs whose results at `136e6c2198` equal the baseline and whose results at `666f0f0bc2` differ, both built in the same container. Then master fails for 77 configs: the 75 that failed before that push, one config without a baseline, and `showcases/tsn/combiningfeatures/frerandcs`, which a later commit changed. Result on the S4 build with these baselines: both runners give the same verdict for each of the 929 statistical tests (841 PASS, 10 expected SKIP, 76 unexpected FAIL, 1 expected and 1 unexpected ERROR). The workflow installs OMNeT++'s Python requirements into the venv, because the statistical test reads the results with `omnetpp.scave`, which needs `pandas<3`. The container runs at most 12 simulations at the same time; with 32, systemd-oomd stops it.
- [ ] **S5. chart.** Compare the chart tests of both runners on `showcases`. Switch the workflow.
- [ ] **S6. validation.** Compare both runners; switch the workflow to `opp_run_validation_tests -m release`.
- [ ] **S7. Remove the duplicate layer.** Rewrite `inet.test.validation` to import from opp_repl; remove the generic modules of `python/inet` and the tools of decision D4; rewrite `inet_run_serializer_tests` as a thin wrapper of `opp_run_opp_tests` in `tests/serializer`; trim `python/requirements.txt`; update the developer guide, the Python package documentation, the release note and the migration guide. Check: each workflow passes, the `.anf` scripts import `inet.scave`, and the documentation builds.

Dependencies: S0 before every other step. S1 to S6 in any order after S0, one pull request each (S0 with S1). S7 after S1 to S6. The opp_repl changes of S3 go to `main` and then to `github_ci` before the INET pull request of S3.

## 7. Verification methods and expected results

Planned checks, not results:

- Each comparison runs in the GitHub-job container with both runners on one build, and compares per-test verdicts, not only counts. Expected: the same verdict for each test; each difference gets a reason in the plan before its workflow switches.
- Each switched workflow passes on its pull request.
- S7: each workflow passes, `opp_run_validation_tests` imports the rewritten module, the documentation build passes, and a chart script with `inet.scave` runs.

What the checks cannot show: the opp_ci results of each kind (optional follow-up).

## 8. Risks, unresolved decisions, and approval scope

Risks:

- The wider NED path of opp_repl can make a test that relies on NED isolation behave differently. S1 and S2 look for it.
- The feature test of opp_repl is much slower than the script; 16 slices may not be enough (S3 measures it).
- The removal of the generic modules can break a user's project that imports them; the migration guide says what to import from opp_repl.

Decisions of the maintainer (2026-10-26): one pull request for each workflow group; `inet.opp` in the INET repository; no aliases for the removed commands.

Decisions of the maintainer, later on 2026-10-26:

- **D1, feature test:** switch to opp_repl's feature test, which also runs the simulations of each feature.
- **D2, unbounded configs:** real limits for the 24 configs (section 4). Revised in S4, because a limit above the natural end moves the baselines: round limits, the stored fingerprint limits, and one deliberate move of 22 statistical baselines. A refresh of master's statistics for the push of `3fd8406ba3` and `666f0f0bc2` comes first, as its own statistics commit.
- **D3, bundled `inet.opp`:** keep it in opp_repl as a fallback for INET versions without the file.
- **D4, tools:** remove only the tools that opp_repl replaces: the `inet_run_*` wrappers with an `opp_run_*` command (all, chart, feature, module, packet, protocol, queueing, release, sanitizer, simulations, smoke, speed, statistical, unit, validation), `inet_run_self_tests` (it tests the removed layer; opp_repl has `opp_repl_self_test`), `inet_build_project` (`opp_build_project`), `inet_python_repl` (`opp_repl`) and `inet_diffcharts` (`opp_diff_charts`). Keep `inet_diffingerprints` and `inet_start_jekyll_server`, which do not use the layer, and `inet_ssh_cluster_python`, which opp_repl's `common/cluster.py:54` calls on the remote hosts.
- **D5, serializer tests:** no opp_repl kind; `inet_run_serializer_tests` stays as a wrapper of `opp_run_opp_tests`.

Separate permissions: each push and pull request; the baseline changes of D2 under TR-BASELINE-DELIBERATE; seal permission if a rule text changes.
