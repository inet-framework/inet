# Implementation plan: coverage tests in GitHub CI

Checkout and revision: INET `origin/master` at `c8afbf7a96` (rebased onto `2f8d0b246f` for I1), branch `topic/coverage-tests` in the worktree `/home/levy/workspace/inet-coverage-tests`. opp_repl `main` at `a502e9d` (`opp_ci` and `github_ci` at the same commit). This plan builds on the plan `tests-on-opp-repl` (branch `topic/tests-on-opp-repl`, approved on 2026-10-26, steps S0 to S7 open).
Plan revision: 2026-10-26, steps O1 to O7 and I1 to I5 done, I6 open (decisions D1 to D4 of the maintainer, D5 to D10 approved with the plan).
Approval: 2026-10-26, the maintainer in the chat: "yes", to the question "Do you approve the plan, or do you want changes first?". The approval covers this plan revision with the proposed decisions D5 to D10. Each push, the pull request and the change of the Pages setting are asked when a step needs them.

## 1. Problem and intended behavior

INET CI runs the fingerprint, unit, module, packet, queueing and protocol tests, but nobody knows which parts of `src/` these tests execute. There is no coverage value for master, no history, and no list of the source lines that no test reaches. [TR-SHIP-WITH](../../doc/project/rule/testing.md#tr-ship-with) ("new behavior ships with its test") has only agent review as its enforcement; the rule says "T2 once coverage is gated". A gate needs a measurement first.

opp_repl has a coverage function, but it cannot do this job now (section 2.1).

The intended result:

- Each push to master runs a new workflow `coverage-tests.yml`. It builds OMNeT++ and INET in coverage mode, runs the fingerprint tests and the five opp_test kinds, and makes a line, function and branch coverage report of `src/`.
- The job summary shows the totals, the totals of each area of `src/inet/`, the totals of each test suite, and the change against the previous master report.
- `https://inet-framework.github.io/coverage/` shows the HTML report of the latest master commit. The `master` branch of `inet-framework/coverage` keeps one summary line for each reported INET commit.
- A manual start (`workflow_dispatch`) runs the same jobs on any branch. It gives the reports as artifacts and publishes nothing.

Example: a developer adds a retransmission timer to a protocol and wants to know if a test reaches it. Today they cannot find out without a local coverage build. After this plan, they start the workflow on their branch and open the HTML artifact: each line that no test executes is red.

Not in this plan (decision D2): a check on pull requests or on commits. Section 9 keeps the design of that check for a later plan.

## 2. Current implementation and evidence

### 2.1 opp_repl coverage (`main` at `a502e9d`)

Paths are relative to `/home/levy/workspace/opp_repl/opp_repl`.

- `test/coverage.py` `generate_coverage_report()` sets `os.environ["LLVM_PROFILE_FILE"] = "coverage-%p.profraw"`, runs `run_simulations(mode="coverage")`, collects the `coverage-*.profraw` files under the project root, merges them with `llvm-profdata merge`, and runs `llvm-cov show <executable> -format=html`. `run_coverage_tests()` (`opp_run_coverage_tests`) applies the `coverage` test parameters of the project and calls it.
- **Gap A, wrong object.** The executable is `simulation_project.get_executable(mode)`. For a project of build type "dynamic library", that is the OMNeT++ executable `bin/opp_run_coverage` (`simulation/project.py:719-722`). INET's code is in `src/libINET_coverage.so`, and llvm-cov reads the coverage mapping only from the objects that it gets. From the code, the report holds only the files of `opp_run`. No run confirms this yet; O1 checks it.
- **Gap B, simulations only.** The function calls `run_simulations`, so the fingerprint tests and the opp_test kinds cannot contribute. The bundled `opp/inet.opp` has no `coverage` entry, so `opp_run_coverage_tests -p inet` runs every run of every config.
- **Gap C, opp_test kinds.** `test/opp.py:124` and `test/opp.py:312` set `binary_suffix = "_dbg" if self.mode == "debug" else ""`. OMNeT++ names coverage binaries with `_coverage` (`Makefile.inc.in`, `D=_coverage`). In coverage mode, a test therefore links `-lINET` (the release library) and starts a binary without suffix.
- **Gap D, profile files.** The pattern `coverage-%p.profraw` is relative, so each simulation writes into its own working directory, one file for each process. Two facts are unresolved, and O1 checks both:
  - Each instrumented shared library can have its own copy of the profile runtime and write its own profile. With `%p` only, the libraries of one process then write to the same file name, and one overwrites the other. (O1 showed that this is not so: the runtime appends the profiles of all binaries of a process to one file.)
  - My estimate for one profile of the INET library is 10 to 20 MB. A fingerprint slice has about 460 runs (1828 runs in 4 slices), so the raw files can need 5 to 9 GB. A standard GitHub runner has 14 GB of SSD ([GitHub documentation](https://docs.github.com/en/actions/reference/runners/github-hosted-runners)).
- **Gap E, output.** The function writes HTML only. CI needs lcov to merge the reports of several jobs, and a summary that a program can read for the history and the delta.
- **Gap F, side effect.** `LLVM_PROFILE_FILE` stays set in the Python process after the call.
- Simulations and opp_test programs start as subprocesses with `simulation_project.get_env()`, a copy of `os.environ` (`simulation/subprocess.py:18`, `simulation/project.py:156`). A variable that the workflow sets reaches each process.

### 2.2 The build

- OMNeT++ 6.x sets `CFLAGS_COVERAGE` to `-fprofile-instr-generate -fcoverage-mapping` and `LDFLAGS_COVERAGE` to the same value by default (`configure.in:203-204`). `MODE=coverage` adds them to the debug flags. `make MODE=coverage` therefore works for OMNeT++ and INET without a configure change. INET already has [bin/inet_coverage](../../bin/inet_coverage).
- [build-omnetpp.sh](../../_scripts/github/build-omnetpp.sh) and [build-inet.sh](../../_scripts/github/build-inet.sh) take `MODE` from the environment.
- `src/libINET_dbg.so` is 955 MB on this machine, and the coverage library is larger. The report job must not need the library, so each test job exports a text report.
- The runner image is `ubuntu-24.04` (version 20261012.334.1 in the module test job of 2026-10-26). The workflows install the `clang` package version 1:18.0. The GitHub-job container of the earlier plans uses clang 16 as `clang` and reproduced the GitHub results exactly. The job log does not show the compiler version. The raw profile format changes between LLVM versions, so `llvm-profdata` and `llvm-cov` must have the version of the compiler. I2 records `clang --version` in the container, and I3 records it on the runner.
- The INET [.gitignore](../../.gitignore#L53) already ignores `/coverage` and `*.profraw`.

### 2.3 Size and time

- `src/` has 1519 `.cc` files and 445062 lines in `.cc` and `.h` files, without the generated `_m.cc` and `_m.h` files (242 `_m.cc` files in the built checkout `inet-master`). The third-party code is in `src/inet/transportlayer/tcp_lwip/lwip/`.
- Test files: 759 module, 128 unit, 824 protocol and 77 queueing `.test` files; the fingerprint store runs 1828 runs in CI.
- Job times with the build, in debug mode: the fingerprint slices took 31, 33, 73 and 72 minutes in run [40309654364](https://github.com/inet-framework/inet/actions/runs/40309654364) (2026-10-26). The module tests took 12 minutes (2026-10-26), the protocol tests 21 minutes (2026-09-25), and the unit tests 7 minutes (2026-10-21). The build part changes much with the ccache state.

### 2.4 The plan tests-on-opp-repl

That plan moves `inet.opp` into the INET root (S0) and switches the unit, module, packet and queueing workflows (S1) and the protocol workflow (S2) to opp_repl, after a comparison of the verdict of each test. Its opp_repl worktree `/home/levy/workspace/opp_repl-tests-on-opp-repl` has an open repair in `test/opp.py`: the filter of `work` folders also matched the checkout path `/home/runner/work/...` on a GitHub runner. This plan needs S0, because the `coverage` entry goes into INET's own `inet.opp`. It also needs S1 and S2, because the coverage workflow runs the opp_test kinds through opp_repl.

### 2.5 The coverage repository

The maintainer created `inet-framework/coverage` on 2026-10-26: public and empty, with issues and wiki off and the homepage `https://inet-framework.github.io/coverage/`. It has the deploy key "inet coverage workflow" with write access. `inet-framework/inet` has the matching repository secret `COVERAGE_DEPLOY_KEY`. GitHub Pages is off, because the API needs a branch that exists.

## 3. Scope and acceptance criteria

Required work:

1. opp_repl closes gaps A to F.
2. INET's `inet.opp` and opp_repl's bundled copy get a `coverage` entry.
3. INET gets `.github/workflows/coverage-tests.yml`.
4. The coverage repository gets a README, the history file and the Pages site.
5. The developer guide describes the reports and the local commands.

Acceptance criteria:

- On a push to master, all jobs of `coverage-tests.yml` pass. The Pages site shows the report of that commit, and the history file of the coverage repository has a line for it.
- Each file in the merged report is under `src/inet/`, is not an `_m.cc` or `_m.h` file, and is not in `lwip/`. Each compiled `.cc` file under these rules is in the report.
- Each of the six suites has a coverage value above zero in the summary.
- Two runs of the workflow on the same commit give the same set of covered lines, or the plan names each difference with its cause. This difference is the noise that a later check must allow.
- The summary gives the verdict counts of each suite. Each verdict that differs from the debug workflows on the same commit has a reason in the plan.
- A manual start on a branch other than master publishes nothing.
- opp_repl's `generate_coverage_report()` gives a report with the project's own source files for a small project.

Preserved: the other workflows and their ccache keys, all baselines, and the sources and tests of INET.

Optional follow-up, not in this plan: the check of section 9, a history chart on the Pages site, more suites (statistical, validation), and a coverage kind in opp_ci.

## 4. Proposed design and reasons

**Coverage is a build mode of the current suites.** The workflow runs the same test commands as the other workflows, with `-m coverage`. There is no second list of "coverage simulations" to keep up to date, and the value means "what the CI tests execute". `opp_run_coverage_tests` stays for a quick report of selected simulations.

**Collection in a profile pool.** In coverage mode, opp_repl gives each simulation and test process `LLVM_PROFILE_FILE=<profile folder>/%4m.profraw`, unless the caller set the variable. `%4m` names the file after the signature of the instrumented binary and lets the runtime merge the profiles of all processes into a pool of 4 files when each process ends. The disk use stays at 4 files for each instrumented binary, and two libraries never write to the same file. The test programs of the opp_test kinds are separate binaries, but their own code is small; the INET counters go to the pool of `libINET_coverage.so`. O1 checks this assumption. If a library does not write its own profile, opp_repl merges the raw files into a sparse `.profdata` file after each group of tasks, and the design of O1 changes before O3 starts.

**Export in each job, report in one job.** After its tests, each job runs a new opp_repl command `opp_export_coverage`. It merges the pool with `llvm-profdata merge -sparse` and writes lcov with `llvm-cov export -format=lcov`. The objects are the project's libraries in coverage mode, given with `-object`. The source folders of the project restrict the files, and the exclusions of `inet.opp` go to `-ignore-filename-regex`. The paths are relative to the project root, and a `TN:` line names the suite. The lcov file is text and small, so the library stays in its job (section 2.2).

A report job collects the lcov files of all jobs. A second new command `opp_coverage_report` adds the counts by file and line and writes four outputs:

- the merged lcov file
- `summary.json`, with the totals, the totals of each area, the totals of each suite, and the values of each file
- one history line
- Markdown for the job summary, with the delta against the previous history line

`genhtml` from the Ubuntu `lcov` package makes the HTML from the merged lcov and the INET checkout. `genhtml` needs no binary and gives totals for each folder; `llvm-cov show` needs the library and the profile.

**Metrics: lines, functions, branches.** Line coverage is the main value, because it is the most stable and every tool reads it. lcov has no regions, so the reports have no region coverage. There is no absolute minimum: CI cannot execute emulation, external interfaces or the Qtenv code, so an absolute value says little (decision D7).

**Suites (decision D5).** The workflow runs the fingerprint tests in 4 slices, as the fingerprint workflow does, and the unit, module, packet, queueing and protocol tests in one job. It does not run the other kinds:

- The statistical and chart tests run almost the same run-0 simulations at the same limits as the fingerprint tests.
- The smoke tests stop on CPU time, so the covered lines depend on the runner speed.
- The validation tests are long, and the feature tests check the build of each feature.
- The speed tests measure time.

The summary of each suite shows whether a later addition is worth its time.

**Verdicts (decision D6).** A failed test does not fail the coverage workflow, because the debug workflows own the verdicts. A coverage job that stays red because of one slow test hides the report. The summary lists the verdict counts. A job fails if its suite gives no lcov file or the export fails.

**CPU time limit.** `inet.opp` gives the fingerprint tests `cpu_time_limit: 600s`. Coverage mode is slower. A run that reaches the limit stops early, and its covered lines then depend on the runner speed. I2 counts these runs. If there are some, the coverage workflow passes a higher limit, and the plan records the value.

**Workflow layout (decision D8).**

```text
coverage-tests.yml   on: push to master, workflow_dispatch
  fingerprint (split 0..3) ──┐
  suites                     ├──> report ──> publish (push to master only)
```

- **Each test job:**
  - Check out `omnetpp-6.x` and INET.
  - Install the packages of the fingerprint workflow, plus the LLVM tools of the compiler version.
  - Build both projects with `MODE=coverage` and install opp_repl from `github_ci`.
  - Run its suite with `--load $GITHUB_WORKSPACE/inet -p inet -m coverage --no-build`.
  - Run `opp_export_coverage` and upload the lcov file as an artifact (retention 7 days).
  - Print `df -h` before and after the tests.
- **The ccache:** the key is `native-coverage-ccache-<run id>` and the restore key is `native-coverage-ccache`. Coverage objects differ from debug objects, so the key is separate. Only split 0 saves the cache, and only on master.
- **The report job:**
  - Install `lcov` and check out INET at the same path, because `genhtml` reads the sources.
  - Download the lcov artifacts and read the last line of the history file from the coverage repository.
  - Run `opp_coverage_report` and `genhtml`, and upload the HTML as an artifact (retention 30 days).
  - Write the Markdown into `$GITHUB_STEP_SUMMARY`.
  - If total line coverage drops more than 0.5 percentage points against the previous report, write a warning annotation. The job does not fail (decision D7).
- **Limits:**
  - `timeout-minutes: 240` at first; after I3, 2 × the measured time.
  - The concurrency group `coverage-tests-<ref>` without `cancel-in-progress`, so master reports do not stop each other.

**Publication (decision D10).** Only a push to master publishes. The step loads `COVERAGE_DEPLOY_KEY` into an `ssh-agent` with plain shell commands, without a third-party action, and pushes over SSH to `inet-framework/coverage`:

- `master` gets one line in `history.jsonl`: the INET SHA, the commit date, the totals, the areas and the suites, about 2 KB. It also gets `latest.json`, the full summary with the values of each file. A file for each commit with the values of each file would add about 150 KB for each master push.
- `gh-pages` gets the HTML, `summary.json` and `.nojekyll` as one new commit without a parent, with a force-push. The repository then does not grow with each report.

**Where the parameters live.** INET's `inet.opp` (after S0 of the other plan) gets this entry: `"coverage": {"report": {"exclude": ["_m\\.(cc|h)$", "/lwip/"], "areas": "src/inet/*"}}`. The bundled copy in opp_repl gets the same entry, because decision D3 of the other plan keeps it as the fallback. `report` is a new recognised aspect of `test_parameters` (O3), not `defaults`, so `apply_project_test_defaults()` does not pass these keys to `run_simulations`. The report covers the project's `cpp_folders` (INET: `src`).

**Reuse.** The plan reuses `MultipleTasks` with `--split-count/--split-index`, the fingerprint store and its CI ingredients, the build scripts of the workflows, `get_dynamic_libraries_for_running()` for the `-object` list, and the mode suffix rule of OMNeT++. The two new commands are the only new entry points. `generate_coverage_report()` uses the new export, so gaps A, D and F also close for the old function.

## 5. Affected components and compatibility

- **opp_repl:**
  - `test/coverage.py`: collection, export and report.
  - `test/opp.py`: the mode suffix.
  - `main.py` and `pyproject.toml`: the two commands.
  - `doc/coverage.md`, the bundled `opp/inet.opp`, and the self tests.

  The names and parameters of `generate_coverage_report()`, `open_coverage_report()` and `opp_run_coverage_tests` stay. Two things change: their report now has the project's code, and the profiles go to one folder.
- **INET:**
  - `inet.opp`
  - `.github/workflows/coverage-tests.yml` (new)
  - the coverage section of [ch-testing.rst](../../doc/src/developers-guide/ch-testing.rst)
  - a release note, if `check-whatsnew.sh` asks for one

  `python/inet/test/coverage.py` is INET's copy of the old function; S7 of the other plan removes it.
- **inet-framework/coverage:** a README, `history.jsonl` and `latest.json` on `master`, and the `gh-pages` branch.
- **opp_ci:** no change.
- **Users:** a developer gets the Pages site and two new commands. No command goes away.

## 6. Future commits and dependencies

### opp_repl (branch `topic/coverage-tests`, worktree `/home/levy/workspace/opp_repl-coverage-tests`, from `main` after the opp_repl commits of the other plan; decision D9)

The opp_repl worktree started from `main` at `b7b827b`, which already holds the opp_repl repair of the other plan (the `work` folder filter). The checks of O1 to O5 used the `fifo` sample of `omnetpp-6.x`, built with `make MODE=coverage` (clang 23 on this machine).

- [x] **O1. Profile pool.** Do an experiment first with an OMNeT++ sample in coverage mode, for example `fifo`. With `%p` and with `%4m`, record which files one process writes for the executable and for each library, and their sizes. Then, in coverage mode, each simulation and opp_test task gets `LLVM_PROFILE_FILE=<profile folder>/%4m.profraw`, unless the caller set it. The folder is the parameter `coverage_profile_folder`, with the default `<project root>/coverage/profiles`. Remove the change of `os.environ`. Check: two processes of the sample write into one pool, and the files of the libraries stay apart. After the call, `LLVM_PROFILE_FILE` is not in `os.environ`. The result of the experiment goes into this plan; if it contradicts section 4, the design changes before O3. Done in opp_repl `9db572b`. The experiment (an executable and a shared library, both instrumented) corrects the first assumption of gap D: with `%p`, the second binary of a process does not overwrite the first; the runtime appends both profiles to one file. The disk problem of `%p` stays. With `%4m`, each binary gets its own files, at most 4. The folder is not a parameter: `get_env(mode=...)` sets the pool, and `get_coverage_profile_folder()` gives the folder of a `LLVM_PROFILE_FILE` that the caller set. Fact found: four helper processes also start an executable of the mode (the run count query, the check for interactive configs, the query of an output file path and the module image test); without the mode they wrote `default.profraw` into the working directory. They now pass the mode, and their profiles go to the pool; they execute only code that the real runs also execute. Checked: 7 concurrent `fifo` simulations write 4 files for each of the 8 instrumented binaries, and the sizes do not depend on the number of processes; no `default.profraw` remains; `os.environ` is unchanged after `generate_coverage_report()`.
- [x] **O2. opp_test kinds in coverage mode.** Take the binary suffix from the mode with the OMNeT++ rule: release none, debug `_dbg`, and `_sanitize`, `_coverage` or `_profile` for the other modes. This fixes both places in `test/opp.py`. Check: one unit test in coverage mode builds `<test>_coverage` with `-lINET_coverage`, and the command lines in debug mode do not change. Done in opp_repl `bfb0987`, with the rule of `SimulationProject.get_executable()`. The debug and release suffixes are the same as before. Fact found: the `-l INET` option of the test run is safe, because OMNeT++ adds the suffix of the `liboppenvir` build mode (`-DLIBSUFFIX="$D"` in `src/envir/Makefile`), so a coverage process loads `libINET_coverage.so`. The INET check moves to I2, which has the INET coverage build.
- [x] **O3. Export.** Add `export_coverage()` and `opp_export_coverage` as section 4 describes. The tool names come from the options or from `LLVM_PROFDATA` and `LLVM_COV`, with the defaults `llvm-profdata` and `llvm-cov`. `generate_coverage_report()` uses the same objects for its `llvm-cov show`. Check: the report of the sample has only its own source files. In the GitHub-job container, one INET module test gives lcov with only `src/inet/` files, without `_m` and `lwip` files. Done in opp_repl `94988bc`. The tool choice goes one step further than planned: after `LLVM_PROFDATA` and `LLVM_COV` comes `<tool>-<major version of CXX>` if it is on the `PATH`. This machine shows why: `clang` is version 23, but the plain `llvm-profdata` is version 21; opp_repl chose `llvm-cov-23` by itself. The source restriction uses the project's `cpp_folders` (INET: `src`), so a separate key is not necessary. Fact found: opp_repl validates the aspects of each `test_parameters` entry, so the settings go into a new recognised aspect `report` (section 4). Checked: the `fifo` export holds its 5 source files with relative paths, two runs on one build give the same lcov bytes, and `generate_coverage_report()` gives HTML pages for the same 5 files. The INET check moves to I2.
- [x] **O4. Report.** Add `coverage_report()` and `opp_coverage_report` as section 4 describes, with an optional `--html` that runs `genhtml` when it is installed. Self tests with small lcov fixtures:
  - two files with the same line in two suites
  - a file in one suite only
  - a function and a branch in both suites
  - a delta against a previous history line

  Check: the self tests pass, the totals equal the counts by hand, and the merge of a file with itself keeps the covered set. Done in opp_repl `778002b`. The 9 self tests (`opp_repl.test.self.coverage`, now part of `opp_repl_self_test`) pass. A branch whose block never ran keeps the lcov `-`. The `--html` check moves to I2, because `genhtml` is not installed on this machine.
- [x] **O5. Parameters and documentation.** Add the `coverage` entry to the bundled `opp/inet.opp`, and rewrite `doc/coverage.md`: collection, export, report, the CI recipe and the limits. Check: each command in the document runs on the sample. Done in opp_repl `b7051d9` (the entry) and `944790f` (the guide). Checked: the commands of the guide run on `fifo`, including a second report with `--previous` against the history line of the first; the bundled `inet.opp` loads with the new entry. Fact found in I2: INET has a second piece of third-party code, `src/3rdparty/sgp4`, so the entry also excludes `/3rdparty/`. In the GitHub-job container, the export of two unit tests names 2557 files, all below `src/inet/`, without `_m`, lwIP and `3rdparty` files.
- [x] **O6. Push.** Push to `main`, then fast-forward `github_ci` and `opp_ci`. This is an outward-facing action that needs the maintainer's word. Check: `pip install` from `github_ci` in a new venv gives the two commands. Done on 2026-10-26 with the maintainer's word: `main` and `github_ci` b7b827b..944790f, `opp_ci` a502e9d..944790f (6 commits). Checked: a new venv with `pip install` from `github_ci` has `opp_export_coverage` and `opp_coverage_report`. The opp_ci service takes the new opp_repl only after a restart.
- [x] **O7. Quiet genhtml (found in I5).** `opp_coverage_report --html` runs `genhtml` with `--quiet`, so that it prints only its overall rates. Checked in the container: the HTML report of the unit tests prints 4 lines of `genhtml`. Pushed on 2026-10-26 with the maintainer's word as opp_repl `0f8b568` on `main` and `github_ci` (2a0817d..0f8b568), after a rebase onto three feature test commits of the other plan. The first push reached only `opp_ci` (944790f..8fc5dc6), because `main` and `github_ci` had moved; with the maintainer's word, `opp_ci` was then force-pushed to `0f8b568`, so the three branches are equal again.

### INET (branch `topic/coverage-tests`, worktree `/home/levy/workspace/inet-coverage-tests`)

- [x] **I0. This plan.** The commit `plan: add: coverage tests`.
- [x] **I1. The coverage entry.** Add the entry of section 4 to INET's `inet.opp`. Needs S0 of the other plan. Check: `opp_export_coverage --load $INET_ROOT -p inet` reads the exclusions from the INET file. Done in `18ab0ea512`, after S0 and S1 of the other plan landed on master (`e7b9bc2c02`, `4f0172a14f`) and the branch was rebased onto `2f8d0b246f`. Checked in the container: with `--load $INET_ROOT`, the export passes `_m\.(cc|h)$|/lwip/|/3rdparty/` to `llvm-cov` and names 2557 files, all below `src/inet/`. Fact found: the area gate of `check-classification.sh` did not count the root `inet.opp` as `build`, so a commit that changes only that file failed CR-SCOPE-AREA (the S0 commit passed only because it also changed a workflow). Repaired before I1 in `54d452f2ac` (`doc: fix: the classification gate counts the project description as the build area`), with a test that fails on the old gate; the 194 enforcement tests pass.
- [x] **I2. Measure in the GitHub-job container.** Use the podman copy of the job (`/home/levy/workspace/ghci-statistical`), with the workflow's package list and the LLVM tools of its compiler. Build both projects in coverage mode, run the six suites as the workflow will, export and report. Record these values:
  - the time of each suite and each fingerprint slice, measured on 4 cores, and the peak disk use of the profile pool and of the build tree
  - the size of the lcov files and of the HTML
  - the runs that reach the CPU time limit
  - the verdicts compared with debug mode on the same commit
  - the difference between two runs on one build

  The record goes to `plan/evidence/coverage-tests/local-run.md`. Then decide the number of slices, the timeout, the CPU time limit and a disk cleanup, and record each decision in section 8. Done on 2026-10-26, see [local-run.md](../evidence/coverage-tests/local-run.md). The other plan also uses `ghci-statistical`, so the measurement used a separate copy, `/home/levy/workspace/ghci-coverage`, with an image that adds `llvm-16`, `libclang-rt-16-dev` and `lcov`. Facts found:
  - The coverage link needs `libclang-rt-<version>-dev`, which `--no-install-recommends` leaves out; the workflow installs it.
  - `tests/packet/Makefile` linked `-lINET_dbg` in every mode; repaired in `04636b0c6c` (`tests: change: the packet test links the INET library of its build mode`).
  - `src/3rdparty/sgp4` is third-party code; the INET entry excludes it (O5).
  - inet-gpl could not build in coverage mode, and its packetdrill runner builds in debug mode only (decision D11).

  Results: line coverage 66.79% (138345 of 207142 lines), functions 67.59%, branches 51.99%. Two runs on one build cover the same lines, functions and branches in every suite: the noise is 0. Coverage mode costs about the same time as debug mode (fingerprint slice 0: 998 s against 1084 s at the same load). No run reached the CPU time limit. The only verdict that coverage mode changes is `Ieee80211BlockAckVariant_1.test`, which starts `inet_dbg` itself. A pool stays at about 130 to 170 MB, an lcov file at about 11.5 MB, and the HTML at 167 MB.

  Decisions from the measurement: 4 fingerprint slices, as in the fingerprint workflow (their coverage times differ by less than a factor of 1.4); `timeout-minutes: 240` stays until I3 gives the runner times; the CPU time limit stays at 600 s; no disk cleanup, because the build tree (5.6 GB) and a pool fit on the runner.
- [x] **I3. The workflow without publication.** Add `coverage-tests.yml` with the test jobs, the report job, the artifacts and the job summary. GitHub starts `workflow_dispatch` only for a workflow on the default branch. For the check, a throwaway branch `test/coverage-tests` therefore gets one more commit that adds a `push` trigger for itself, and nobody merges that branch. A push of the branch needs the maintainer's word. Check: all jobs pass, and the summary values equal those of I2 within the noise of I2. Record the job times and `clang --version`. The workflow is `731987b02f` (`build: add: a coverage workflow that publishes INET's coverage report`), with the publish step of I4 in the same commit: the two steps share one workflow file, and the check of both needs one run. The target branches of the publish step are the job values `HISTORY_BRANCH` and `PAGES_BRANCH`, so that the throwaway commit (`704aabe50a`) only adds its trigger, publishes on its push, and sets `test-master` and `test-gh-pages`. Each test job writes the verdict counts of its suites into its job summary (decision D6). The maintainer allowed the push of `test/coverage-tests` on 2026-10-26; run [40356855469](https://github.com/inet-framework/inet/actions/runs/40356855469). Result: all 6 jobs pass. Facts found:
  - The runner compiles with `Ubuntu clang version 18.1.3`, not with clang 16 like the GitHub-job container; the workflow chose `llvm-18` and `libclang-rt-18-dev` by itself. The run also showed the annotation that `ubuntu-latest` moves to Ubuntu 26 from 2026-10-19; the version choice follows that move without a change.
  - The runner has 78 GB of free disk, so no cleanup is necessary.
  - Times without a coverage ccache: OMNeT++ build about 2.5 min, INET build 27 to 31 min; tests of the fingerprint slices 29.0, 34.9, 30.5 and 30.1 min; suites 37.5 min (unit 5.5, module 10.7, packet 0.1, queueing 0.8, protocol 20.2). The longest job took 73 min with its build, so the timeout of the test jobs is now 150 min (2 × 73); the report job took 2 min.
  - Verdicts: fingerprint all PASS with the expected ERRORs; unit 128 PASS (with the master repairs `2f8d0b246f` and `df2a2eab26`); module 758 PASS and `Ieee80211BlockAckVariant_1` FAIL (it starts `inet_dbg`, see I2); packet and queueing PASS; protocol 395 PASS, 306 SKIP (D11), 47 expected FAIL and the 76 unexpected FAILs that the opp_repl runner also gives in debug mode (S2 of the other plan).
  - Pools 37 to 181 MB; artifacts: 1.7 MB for each fingerprint job, 7.0 MB for the suites job, 23.7 MB for the report with its HTML.
  - Report: 67.3% of lines (140427 of 208638), 67.8% of functions, 52.0% of branches. The values differ a little from I2, because the commit is newer (I1 rebase) and clang 18 gives a different coverage mapping than clang 16.
- [x] **I4. Publication.** Add the publish step. For the check, the throwaway branch publishes to the branches `test-master` and `test-gh-pages` of the coverage repository. Delete those branches after the check. Add the README to the coverage repository. Each push to that repository needs the maintainer's word. Check: the test branches hold the expected files, and a manual start on another branch publishes nothing. Done in the same commit and run as I3. Checked: `test-master` held `history.jsonl` (3.6 KB) and `latest.json` (573 KB), and `test-gh-pages` was one commit without a parent with the HTML (10817 tree entries) and `.nojekyll`. A manual start cannot run before the workflow is on master; the publish step runs only if the event is a push and the branch is master, and the throwaway commit changed exactly that condition. Fact found: the empty repository made the first pushed branch, `test-master`, its default branch, which GitHub does not let anyone delete. With the maintainer's word, the README went to a new `master` (coverage `af2131e1a6`), `master` became the default branch, and the two test branches and the throwaway branch were deleted. The first real run appends its history to that `master`.
- [x] **I5. Developer guide.** Add a coverage section to `ch-testing.rst`: the Pages site, the manual start, the local commands, and what coverage cannot show. Add a release note if `check-whatsnew.sh` asks for one. Check: the documentation builds, and the local commands run. Done in `8498d11297`. `check-whatsnew.sh` asks for no release note. Checked: `doc/src/build --only developers-guide` succeeds (its 2 warnings are in `ch-80211.rst` and come from the partial build), and the commands of the section run in the container: 3 module tests, the export and the HTML report. Fact found: `genhtml` printed a block for each of the 2557 files, so the report command printed 465 KB; opp_repl O7 makes it quiet.
- [ ] **I6. Land.** Rebase onto master, open the pull request and get green checks. After the merge, the first master run publishes. Then turn on Pages with `gh api -X POST repos/inet-framework/coverage/pages -f "source[branch]=gh-pages" -f "source[path]=/"`. Check: the Pages URL shows the report of the merge commit. Record the result, delete the throwaway branch, and move this plan to `plan/done/`.

Dependencies:

- O1 before O3, because the export reads the pool.
- O1 to O5 before I2, and O6 before I3, because CI installs `github_ci`.
- S0 of the other plan before I1. S1 and S2 of the other plan before I3, because then the opp_test kinds are proved on INET.
- I3 before I4. I6 last.

## 7. Verification methods and expected results

Planned checks, not results. The commands run in the GitHub-job container, because local results with clang 23 differ from the runner (see the earlier plans).

- **Collection (O1):** the experiment shows the file set of one process. Expected: one pool for each instrumented binary. This check cannot show the behavior on the runner; I3 shows it.
- **Export (O3) and report (O4):** the self tests and the file rules of section 3. They cannot show that a covered line was correct. Coverage shows that code ran, not that a test checked its result.
- **Workflow (I3, I4):** green jobs on the throwaway branch, the values of I2 within the noise, and the published files on the test branches.
- **Landing (I6):** the Pages URL and the history line of the merge commit.

What the checks cannot establish:

- the coverage of code that CI does not run, such as emulation, Qtenv and osgEarth
- whether a test checks the code that it executes, which stays with [TR-CAT-MATCH](../../doc/project/rule/testing.md#tr-cat-match) and the review

No baseline changes.

## 8. Risks, unresolved decisions, and approval scope

Risks:

- **Time.** Coverage mode is slower than debug mode, and the debug slices already take up to 73 minutes. I2 measures it; more slices are possible.
- **CPU time limit.** Runs that reach the limit make the covered lines depend on the runner. I2 counts them.
- **Disk.** If the libraries do not write separate pools, the raw files grow fast (O1).
- **Tool version.** A mismatch between the compiler and the LLVM tools stops the export with a format error. The workflow takes the tools of the compiler version.
- **The deploy key.** Every workflow of `inet-framework/inet` that runs with secrets can read `COVERAGE_DEPLOY_KEY`, also a `pull_request_target` workflow. The audit workflows never run pull-request code, so the risk is low. A stronger protection is a GitHub environment `coverage` that only `master` can use, with the secret moved into it. That needs an INET admin, and it is optional.

Decisions of the maintainer (2026-10-26):

- **D1, workflow:** a separate `coverage-tests.yml`, not a change of the fingerprint matrix.
- **D2, checks:** no check on pull requests or commits now (section 9).
- **D3, reports:** our own reports, no Codecov.
- **D4, publication:** the repository `inet-framework/coverage` with GitHub Pages. The maintainer created it with the deploy key and the secret.

Proposed decisions, which the approval of this plan covers:

- **D5, suites:** fingerprint (4 slices) and unit, module, packet, queueing and protocol (one job).
- **D6, verdicts:** test failures do not fail the coverage workflow; an export failure does.
- **D7, values:** lines, functions and branches; no absolute minimum; a warning annotation when line coverage drops more than 0.5 percentage points.
- **D8, triggers:** a push to master and `workflow_dispatch`; no `pull_request` trigger (D2).
- **D9, opp_repl branch:** `topic/coverage-tests` in its own worktree, then `main`, `github_ci` and `opp_ci`.
- **D10, storage:** `history.jsonl` and `latest.json` on `master` of the coverage repository; the HTML on `gh-pages` as one force-pushed commit.

Decision made during I2, open for the maintainer to change:

- **D11, inet-gpl:** the coverage workflow does not check out inet-gpl, so the 306 packetdrill wrappers of the protocol suite report SKIP ("inet-gpl not found"). inet-gpl could not build in coverage mode, because the precompiled headers of the coverage and profile modes were absent; the maintainer pushed that repair as inet-gpl `ca17ec6`. But the packetdrill runner of `tests/packetdrill` still builds in debug mode only (`-lINET_dbg`, `MODE=debug`, and `suite.py` looks for `packetdrill_inet_dbg`). In a coverage job, the wrappers would need a second, debug build of OMNeT++ and INET, and would add no coverage. A runner that follows the build mode is a follow-up in inet-gpl; then the coverage workflow can check out inet-gpl as the protocol workflow does.

Separate permissions: each push (opp_repl branches, the INET topic branch, the throwaway branch, the coverage repository) and the pull request. Pages is turned on in I6. No seal edit and no baseline change are planned.

## 9. Deferred: a check on pull requests

Decision D2 defers this check. The design from the discussion of 2026-10-26 is kept here, so that a later plan can start from it:

- **The check enforces TR-SHIP-WITH at T2.** The rule is: "a CI test enters each function that a `refactor` or `behavior` commit adds or changes." The line percentage stays in the report and is not a gate.
- **Which lines count:** the `Change:` depth of each commit decides. `git blame --ignore-revs-file` gets the `comment`, `format`, `location` and `name` commits of the pull request, so a line goes back to the commit that wrote its content. Only lines from `refactor` and `behavior` commits in the pull request count.
- **Exemptions:** a list file of the areas that CI cannot execute, and a line `Coverage-exempt: <function> — <reason>` in the pull-request description.
- **Where the check runs:** a `pull_request` run builds and uploads the result. A trusted `workflow_run` job reads the result as data and posts a commit status. On an `edited` event, a small `pull_request_target` job evaluates the stored result again.
- **Order of steps:** report only, then a red status that is not required, then a required status and the change of the "Enforced at" line of TR-SHIP-WITH.
