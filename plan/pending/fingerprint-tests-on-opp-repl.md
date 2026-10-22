# Implementation plan: fingerprint tests on opp_repl

Checkout and revision: the measurements of section 2 are from `inet-master`, branch `master` at `60814876c5`. The work is on branch `topic/fingerprint-tests-on-opp-repl` in the worktree `/home/levy/workspace/inet-fingerprint-tests-on-opp-repl`, from `origin/master` at `02f24a3a4d` (one documentation commit later, no change in `tests/`, `python/`, `bin/` or `.github/`). opp_repl: `/home/levy/workspace/opp_repl`, branch `main` at `bb4d294` (examined); the work goes to branch `opp_ci` at `fdc632c` (decision D6), in the worktree `/home/levy/workspace/opp_repl-opp_ci`.
Plan revision: 2026-10-22, third draft (decisions D1-D7 and D9-D11 recorded).
Approval: 2026-10-22, the maintainer in the chat: "implement in a worktree", after the decisions of section 8. The approval covers this plan revision. The separate permissions of section 8 (the pushes in O5 and I8, the seal edits in I7) are asked when a step needs them.

## 1. Problem and intended behavior

INET has two fingerprint test tools now:

- `tests/fingerprint/fingerprinttest` (a wrapper of `bin/inet_fingerprinttest`) reads the 14 `tests/fingerprint/*.csv` files. GitHub CI calls it in [fingerprint-tests.yml](../../.github/workflows/fingerprint-tests.yml#L57). The CSV files are the authoritative baseline. Developers update them with the `.UPDATED` files that the tool writes.
- opp_repl's `opp_run_fingerprint_tests` reads `tests/fingerprint/store.json`. opp_ci uses it. On master, the store is stale: its last sync is `3a9cc276bb` (2026-09-09), and 88 commits changed the CSV files after it.

A developer must keep two baselines, and only one of them is correct. The intended result:

- `opp_run_fingerprint_tests` is the only fingerprint test tool. `tests/fingerprint/store.json` is the only baseline.
- The store holds every fingerprint that the CSV files hold, for all ingredients. It also holds run-0 fingerprints of the three CI ingredients for each concrete config that lacks them now.
- The old script, the CSV files, and INET's own Python fingerprint layer are gone. GitHub CI runs the new tool. All documentation describes the new tool.

Example: a developer changes the 802.11 MAC. Today they run `./fingerprinttest -m wireless`, then `mv examples.csv.UPDATED examples.csv`. After the change they run `opp_run_fingerprint_tests -f wireless`, then `opp_update_fingerprint_test_results -f wireless`. Git shows the changed store lines.

## 2. Current implementation and evidence

### 2.1 Earlier work on this topic

The branch `topic/migrate_to_opp_repl_master` (origin too, last commit `3f15e048ca` on 2026-07-17) holds the earlier attempt. It is 22 commits ahead of and 1331 commits behind master. It has two parts:

- The removal of the redundancy between INET and opp_repl, plan `plan/done/python-opp-repl-migration.md`. The work started on the branch `python/opp-repl-migration` (now deleted), and its commits are on this branch. It deletes the generic INET Python modules that opp_repl replaces, moves the project definitions to `.opp` files, removes INET's fingerprint framework (`874a991ec1`), and removes all `bin/inet_run_*` wrappers, `inet_fingerprinttest` and `inet_smoketest` (`45f8fec06b`, `aaddc14f81`). At its end, `python/inet` holds only `cffi`, `scave`, `documentation` and `test/validation.py`. It also removes all GitHub workflows (`1d40cd28b5 ci: Remove GitHub Actions workflows (replaced by opp_ci)`). Master's workflows still call 9 `inet_run_*` wrappers, so this part cannot land as it is. After the branch point, 13 master commits changed `python/` and `bin/` (for example the new `inet_run_packetdrill` and `inet_run_serializer_tests`). opp_repl now has runners for the unit, module, packet, queueing and protocol tests that the branch kept in INET, and it calls INET's validation tests through `inet.opp`.
- The fingerprint baseline, plan `plan/pending/fingerprint-csv-to-store-migration.md`. It did phase 1 only (a faithful CSV to store conversion, checked on opp_ci run 53 and run 74).

This plan is one slice of the redundancy removal, in the same direction: it removes INET's fingerprint layer the way `874a991ec1` did, and it adds the opp_repl installation to GitHub CI that the redundancy plan named as an open item. This plan reuses the lessons of that branch, but not its commits. Section 8 explains why, and decision D9 sets the order of the two pieces of work.

### 2.2 The CSV baseline (measured on `60814876c5`)

| Fact | Value |
| --- | --- |
| CSV rows | 1774 in 14 files |
| Concrete configs with a row | 839; every one exists in the tree |
| Rows by run selector | 853 run 0, 789 other run numbers, 132 iteration-variable filters (`-r '$datarate==... && $repetition==0'`) |
| Ingredient values | `~tNl` 1738, `tplx` 1419, `~tND` 1229, `tyf` 1004, `tpl` 34, `~tNlb` 22 |
| Expected results | 1773 PASS, 1 ERROR (`ethernet-nonstandardspeed.ini`, 5Gbps half duplex) |
| Rows without a limit | 160, all for `tests/fingerprint/ethernet-*.ini`; the ini file sets the limit |
| Rows with no CI ingredient | 22: all of `mipv6-refactoring.csv` (`~tNlb` only) and 2 PIM rows (`~tNlb` only) |
| Duplicate keys | 2: `tutorials/bgp Step1` (identical rows) and `showcases/tsn/cutthroughswitching` (same values, one row lacks `~tND`) |

CI checks only `tplx`, `~tNl` and `~tND` (`-f 'tplx' -f '~tNl' -f '~tND'`). A conversion of all ingredients must give 5439 entries for 1752 runs, and 4 entries expect ERROR. Of these, the three CI ingredients give 4381 entries for 1750 runs, and 3 entries expect ERROR. No duplicate key has two different values.

### 2.3 Coverage of run 0

opp_repl finds 1213 configs in the tree. 929 are concrete and are not emulation configs. Their run-0 coverage by the three CI ingredients:

| Run-0 coverage | Configs |
| --- | --- |
| All three ingredients | 675 |
| Some ingredients | 154 (136 lack only `~tND`; 12 have only `tplx`; 6 have only `~tNl`) |
| No ingredient | 100 (10 of them have rows only for other runs or filters) |

The 100 configs include special cases. 14 have no `sim-time-limit` in the ini file (for example `tutorials/protocol`, `examples/empty`, `showcases/visualizer/osg/earth`). `showcases/tsn/combiningfeatures/gsandcs SAT` depends on the z3 version. The osgEarth configs need a library that CI does not install. Some have long limits (`examples/wireless/ieee80211levelofdetail` 1000s, `showcases/wireless/errorrate` 500s). The four `tutorials/fingerprint` configs have their own `baseline.csv`.

### 2.4 The old tool

- [bin/inet_fingerprinttest](../../bin/inet_fingerprinttest#L493) runs each row as one `opp_run` with all selected ingredients, `--cpu-time-limit=600s`, `--vector-recording=false --scalar-recording=true`, and `--fingerprintcalculator-class=inet::FingerprintCalculator`.
- `~tND` adds `**.checksumMode="computed"` and `**.fcsMode="computed"` ([lines 65-66](../../bin/inet_fingerprinttest#L65)).
- `-n N -i I` runs one of N slices ([line 809](../../bin/inet_fingerprinttest#L809)). CI uses 2 modes × 4 slices, so 8 runners of 4 cores each. In master run [39753655162](https://github.com/inet-framework/inet/actions/runs/39753655162) (2026-10-22), each slice had 438 runs. The test times of the slices were 6.2, 5.9, 4.5 and 5.2 min in release mode, and 29.0, 15.4, 23.4 and 15.3 min in debug mode. The build took 1-7 min more for each job.
- `-m` and `-x` filter on the working directory plus the arguments. `-a` passes more `opp_run` arguments. The developer guide and the tutorial use `-a` for `--fingerprint-modules` and `--fingerprint-events`.

### 2.5 The new tool (opp_repl `main`)

Paths are relative to `/home/levy/workspace/opp_repl/opp_repl`.

- The bundled `opp/inet.opp` sets the fingerprint defaults: `store: tests/fingerprint/store.json`, `stored_only: True`, `select_fingerprint: shortest`, `ingredients_list: [tplx, ~tNl, ~tND]`.
- `test/fingerprint/task.py` `FingerprintTestGroupTask.run()` runs one simulation for all ingredients that share a limit. `~tND` also adds `**.crcMode="computed"`. Opp_ci run 74 showed no effect of that extra argument on the current values.
- Gap A, silent drop: `collect_fingerprint_test_groups()` keeps a task only `if fingerprint_test_task.sim_time_limit` (`task.py:295`). The importer `test/fingerprint/old.py` hard-codes the limits of five `ethernet-*.ini` files and stores `None` for the others. On the earlier branch, the 64 `ethernet-nonstandardspeed.ini` entries had `None`. From the code, opp_ci runs 53-74 did not run them, and also did not run the one expected ERROR. No run log confirms this yet. The opp_ci "parity" result is therefore not complete evidence.
- Gap B, the update command: `get_update_correct_fingerprint_tasks()` (`task.py:590`) enumerates every run of every config with the ini limit. It does not apply the project defaults, does not use the stored limits, and does not know the filter runs. `FingerprintUpdateTask` records one ingredient for each simulation. Thus `opp_update_fingerprint_test_results` cannot replace the `.UPDATED` workflow now.
- Gap C, the command line (`main.py:21-60`): there is no option to select ingredients, to pass `opp_run` arguments, or to run one slice of the tests.
- Gap D, the file format: `FingerprintStore.write()` writes `json.dump(indent=True)`, 12 lines for each entry (the master store has 53750 lines). The key order of an entry is the order in which the code made it, so it is not fixed. Each entry has a `timestamp` (the time of the insert) and an `itervars` field. On the earlier branch, `itervars` was `$repetition==0` in all 5439 entries, also for run 5 and for the filter runs. No fingerprint code reads that field. A line-based merge of this format corrupted the store during a rebase on 2026-07-16 (wrong values, no conflict).
- Gap F, ingredient arguments: `get_ingredients_append_args()` (`task.py:36`) returns no arguments for an ingredient that is not in its table, also not the INET calculator class. Thus a `~tNlb` run uses the wrong calculator. For `tyf`, opp_repl uses fake-GUI probabilities of 0.1, but the old tool used 0.001 ([lines 68-76](../../bin/inet_fingerprinttest#L68)). OMNeT++ itself has no fixed values. Thus the converted `tyf` values cannot match an opp_repl run now.
- Gap E, version: the last opp_repl tag `0.4` (2026-05-11) is 195 commits behind `main`. It does not have the filter-run support or the aggregate-result repair. The branch `opp_ci` (`fdc632c`, 2026-08-08) has them. It also has two fingerprint commits that `main` lacks: `639c871` (the update command no longer crashes on a result without a calculated fingerprint) and `fdc632c` (a shorter fingerprint entry in `inet.opp`). `main` has 12 commits that `opp_ci` lacks (the change summary tool and two `opp test` repairs). None of these 12 commits touches the fingerprint code.

### 2.6 Consumers in INET

A survey of the tree found these consumers:

- GitHub: only [fingerprint-tests.yml](../../.github/workflows/fingerprint-tests.yml) calls the old tool. [python/inet/common/github.py](../../python/inet/common/github.py#L26) and opp_repl's `inet.opp` name the workflow file.
- Commit gates: [check-commits.sh](../../doc/project/enforcement/check-commits.sh#L224) pipes the CSV diff of each commit into [fingerprint_moves.py](../../doc/project/enforcement/fingerprint_moves.py), which parses CSV rows (gate PR-SPLIT-ONE-CHANGE). [test_check_commits.py](../../doc/project/enforcement/test_check_commits.py) and [test_check_classification.py](../../doc/project/enforcement/test_check_classification.py#L58) have CSV fixtures. PR-SPLIT-BASELINE matches `^tests/.*\.(csv|json)$` and already covers the store.
- INET Python layer: `python/inet/test/fingerprint/`, `bin/inet_run_fingerprint_tests`, `bin/inet_update_correct_fingerprints`, and imports in `python/inet/test/__init__.py`, `all.py`, `statistical.py`, `self.py`, `main.py`, `__init__.py`.
- Other scripts: [fingerprinttest_selfdoc](../../tests/fingerprint/fingerprinttest_selfdoc) runs the old tool in debug mode with `--generate-selfdoc=true` to make `SelfDoc.json` ([releng/README.md](../../releng/README.md) names it). `tests/fingerprint/test-branch` diffs `.UPDATED` files. `tests/fingerprint/smoketest` and `bin/inet_smoketest` read `*.csv` from the current folder by default, that is, the fingerprint CSV files. `_scripts/travis/travis-stage-fingerprints.sh` and `_scripts/jenkins/config.xml` are old CI scripts.
- Documentation: [ch-testing.rst](../../doc/src/developers-guide/ch-testing.rst), the ten pages of [tutorials/fingerprint/doc](../../tutorials/fingerprint/doc/index.rst) and its `baseline.csv` (tags select tutorial steps), [change-a-baseline.md](../../doc/project/guide/change-a-baseline.md#L22), [run-the-gates.md](../../doc/project/guide/run-the-gates.md#L25), the rules TR-BASELINE-DELIBERATE ([testing.md](../../doc/project/rule/testing.md)), PR-SPLIT-BASELINE ([pull-request.md](../../doc/project/rule/pull-request.md#L171)), the fingerprint test identity in [naming.md](../../doc/project/rule/naming.md#L347), AR-QUAL-TRACEABILITY in [checklist/general.md](../../doc/project/enforcement/checklist/general.md#L97), and NV-15 in [naming-exceptions.md](../../doc/project/audit/naming-exceptions.md#L83) (the CSV tag column).
- Not affected: `bin/inet_diffingerprints` (eventlog diff), `tests/speed/speedtest`, the `.gitignore` patterns (other tools use them too), `tests/dependency.json`, and history text in `plan/done/` and `doc/project/history/`.

## 3. Scope and acceptance criteria

Required work:

1. opp_repl closes gaps A-D and F on its `opp_ci` branch. That branch closes gap E (decision D6).
2. The store holds the converted CSV fingerprints and the new run-0 fingerprints.
3. GitHub CI runs `opp_run_fingerprint_tests` in the same 2 modes × 4 slices.
4. The old tool, the CSV files, the INET Python fingerprint layer, and the scripts that only serve them are gone.
5. The gates, the guides, the rules, the developer guide, the tutorial and the release notes describe the new tool.

Acceptance criteria:

- `git ls-files 'tests/fingerprint/*.csv' tests/fingerprint/fingerprinttest bin/inet_fingerprinttest` is empty.
- A grep of the tree for `fingerprinttest` and `fingerprint/*.csv` finds only history text.
- Each of the 5439 converted entries is in the store with its CSV value, limit and expected result. A script proves this against the CSV files of the landing revision.
- Each concrete, non-interactive, portable config has run-0 entries for the three CI ingredients, at the limit of its ini file. Section 8 lists the exclusions with a reason for each.
- Two writes of the same entries give the same bytes.
- On one build, the old tool and the new tool give the same verdict for each of the 1750 converted runs.
- The GitHub fingerprint jobs are green on the pull request, in debug and release mode.
- `opp_update_fingerprint_test_results` changes only the store lines of the changed fingerprints.
- The gate tests in `doc/project/enforcement/` pass with the new fixtures.

Preserved conditions: the other GitHub workflows, the `tests/fingerprint/*.ini` and `*.xml` files, the statistical and chart baselines, and the INET Python layer for the other test kinds. The workflow file keeps the name `fingerprint-tests.yml`, so badges and links stay valid.

Optional follow-up work, not in this plan: lower the long limits (the measurement table of the earlier plan). The next plan (decision D9) does the redundancy removal of section 2.1 again on master and moves the other GitHub test kinds to opp_repl.

## 4. Proposed design and reasons

**One baseline, owned by the project.** The store replaces the CSV files. opp_repl already reads the store, opp_ci already uses it, and a second format would keep the drift of section 2.5 alive. opp_repl's `old.py` importer stays in opp_repl for other projects. INET uses it only once.

**All ingredients (decision D1).** The conversion keeps all six ingredients, also `tyf`, `tpl` and `~tNlb`, which CI does not check. The `ingredients_list` of `inet.opp` still selects the three CI ingredients, so the other entries are stored but not checked by default. A developer can check them with `--ingredients`. For this, opp_repl must run them with the arguments of the old tool (gap F): the INET calculator class for each `~` ingredient, and for `tyf` the fake-GUI arguments of the old tool (decision D10). The old values (probability 0.001) also make a `tyf` run much faster than the opp_repl values (0.1); with 0.1, a 100s BMac run needed about one hour. New run-0 entries get only the three CI ingredients. The old tool calls `tyf` "not regularly kept up to date", so a new `tyf` value would add a value that nobody keeps.

**Explicit limits.** Each entry names its limit. The conversion reads an absent CSV limit from the ini file. The runner reports an entry without a limit as ERROR, not as nothing (gap A). A reader of a store entry then sees the complete test.

**The current layout with a fixed field order (decision D2).** The store keeps the 12-line layout and all fields. `write()` puts the keys of each entry in one fixed order and sorts the entries by their key, so the same entries always give the same bytes. The conversion gives all entries one fixed `timestamp` (the time of the CSV revision), so a second conversion of the same CSV files gives no diff. An update keeps the `timestamp` of an entry whose value does not change. The gate does not parse diff hunks of this layout. It reads the store of the commit and of its parent and compares the entries by key.

**The update command uses the store (gap B).** `opp_update_fingerprint_test_results` applies the project defaults. With `stored_only`, it re-records each stored run at its stored limit, filter runs too. It records all ingredients of the run in one simulation. The test command and the update command share one function that collects the stored runs. To add a config, a developer selects it and turns `stored_only` off; the command then inserts run-0 entries at the ini limit or at `-t`.

**Command-line options (gap C).** `--ingredients`, `--stored-only/--no-stored-only`, `--append-args` (more `opp_run` arguments; the selfdoc script, the tutorial and the developer guide need it), and `--split-count/--split-index` (decision D11). The parallel run of opp_repl uses only the 4 cores of one GitHub runner, as the old tool did. The split spreads the tests over 4 runners for each mode. Without it, the debug tests on one runner need about 83 min instead of about 29 min for the slowest slice, before the new run-0 configs. The split does not depend on the test kind, so it is a generic option. It goes into `MultipleTasks` in `common/task.py`, next to the generic `randomize` and `chunksize` options, and into the shared parser `parse_run_tasks_arguments` in `main.py`. Thus every `opp_run_*` command gets it, also the commands that the next plan (decision D9) moves to opp_repl. The slice takes every N-th task of the list, as the old tool did (`testcases[args.i::args.n]`), because a stride balances the slow tasks better than equal blocks. It applies only to the top-level list: `FingerprintTestGroupTask` is itself a `MultipleTestTasks`, and a slice of a group would drop ingredients. The fingerprint defaults in `inet.opp` get `cpu_time_limit: 600s`, which the old tool used.

**New run-0 limits come from the ini file (decision D3).** For a config without run-0 entries, the new entries use the `sim-time-limit` of the config in its ini file. The store then needs no limit choice of its own. 14 of the 100 configs have no limit in the ini file. Each of them gets a `sim-time-limit` in its ini file first. Before that edit, check whether the run stops by itself, and whether the statistical and chart baselines of that config change. For a config with some ingredients, record the absent ingredients at the limit of the current entry; the same run must reproduce the current values.

**Record where CI runs.** Record new values in the podman copy of the GitHub job (`/home/levy/workspace/ghci-statistical`, ubuntu 24.04, clang-16). That copy reproduced the GitHub statistical results exactly. A local clang-23 build gave 6 `tplx` failures on wireless configs that GitHub does not show. Nobody found the cause.

**Reuse checked:** opp_repl `old.py` (conversion), `FingerprintStore` (schema), `FingerprintTestGroupTask` (one run for all ingredients), the `stored_only` filter-run code (shared with update), the podman job copy (recording). Additions are only the gaps above.

## 5. Affected components and compatibility

- opp_repl branch `opp_ci`: `common/task.py` (the generic split), `test/fingerprint/store.py`, `task.py`, `old.py`, `main.py`, `opp/inet.opp`, its docs. INET CI installs this branch (decision D6). The opp_ci workers also use opp_repl, so a push to `opp_ci` changes their fingerprint kind too.
- INET `tests/fingerprint/`: `store.json` new content; CSV files and scripts removed; `README` rewritten. `fingerprinttest_selfdoc` calls the new tool with `--append-args`.
- INET ini files of the 14 configs without a limit (`tutorials/protocol`, `examples/empty`, `examples/wireless/lan80211ac`, `showcases/visualizer/osg/earth`): a new `sim-time-limit`. This edit can change their statistical and chart baselines.
- INET `bin/` and `python/inet/`: the fingerprint parts removed.
- INET `.github/workflows/fingerprint-tests.yml`: installs opp_repl and calls the new tool.
- INET `doc/project/`: gate script, gate tests, guides, rules. The rule files are sealed "by rule" ([seal-list.md](../../doc/project/audit/seal-list.md)). The edits in `naming.md`, `pull-request.md`, `testing.md` and `classification.md` need seal permission under [sealing.md](../../doc/project/rule/sealing.md).
- Users: `bin/inet_fingerprinttest` is a user-visible tool. Other projects can call it. A WHATSNEW entry file and a migration guide entry file (RR-WHATSNEW-FILE, RR-MIGRATE-FILE) name the replacement commands and the opp_repl CSV importer.
- Open pull requests that change CSV files will conflict on deleted files. Their authors re-record with the update command. The pull request text of this series says so.

## 6. Implementation steps and dependencies

opp_repl steps come first, because INET CI installs opp_repl from its `opp_ci` branch. Each step is one commit or a short series.

### opp_repl (branch `opp_ci`, in its own worktree)

- [x] **O1. Explicit limits.** `old.py` reads an absent limit from the config's ini file and drops the hard-coded `ethernet-*.ini` values. The runner gives ERROR "no sim-time-limit in the store" for an entry without a limit. Check: a unit case with an empty CSV limit; a store entry with `None` gives ERROR. Done in opp_repl `74fdffc`. The selector path also dropped an entry without a limit: `get_fingerprint_test_task()` filtered by the ini limit, which never matches `None`; an entry with a limit now wins over one without. Checked: the import of `ethernet.csv` gives a limit for all 640 `tests/fingerprint` entries (64 with `5s`), and a stored run without a limit gives 3 unexpected ERRORs, with and without a selector.
- [x] **O2. Fixed field order.** `write()` puts the keys of each entry in one fixed order and keeps the 12-line layout. An update keeps the `timestamp` of an unchanged entry. The importer takes a fixed `timestamp` argument. Check: read the master store, write it, write it again: the second write gives no diff. Done in opp_repl `4efba7e`. `write()` also failed on a config with both run numbers and filter runs, and on a `None` limit; the sort key now handles both. The importer reads the CSV files in sorted order. Checked: the rewrite of the master store gives the same bytes; two imports of all CSV files with `timestamp=0` give the same bytes, 5439 entries, 1752 runs, 4 ERROR, no entry without a limit.
- [x] **O2b. Ingredient arguments.** Each `~` ingredient gets the INET calculator class. The `tyf` arguments become those of the old tool ([bin/inet_fingerprinttest:68-76](../../bin/inet_fingerprinttest#L68), decision D10): each fake-GUI probability 0.001, the two `numsteps` values 3, and the two animation-time parameters. Check: one `tyf` and one `~tNlb` entry from the converted store give PASS on the podman build. Done in opp_repl `9466479`. INET's calculator gives the base OMNeT++ values for ingredients without INET letters, so only `~` ingredients need the class. Checked on the local build: four `tyf` entries of `ethernet-switch.ini` and the `~tNlb` entry of `examples/pim/dm_ipv6` give the CSV values.
- [x] **O3. Update from the store.** Share the stored-run collection between test and update. The update applies the project defaults, uses the stored limit and the filter runs, and records all ingredients in one simulation. Check: on INET, update of one filter run, one run-5 entry, and one config without entries gives KEEP, KEEP, INSERT, and the store diff has only the new lines. Done in opp_repl `062b3cb`. Facts found: `SimulationTask` reads only its `sim_time_limit` attribute, so each limit gets its own task (`copy_simulation_task()`); the old update looked an entry up with `test_result="PASS"` and inserted a second entry for one that expects ERROR; `get_calculated_fingerprint()` could match `~tNlb` for `~tNl` and now matches the whole string. One result for each simulation; the store is written by key, so a process scheduler also works. Checked: 16 filter runs give 15 KEEP and the expected ERROR, run 5 gives KEEP, a config without entries gives INSERT, a run without `~tND` gives INSERT of `~tND` only, and the store diff holds only the 4 new entries.
- [x] **O4a. Generic split.** `MultipleTasks` gets `split_count` and `split_index`, and `parse_run_tasks_arguments` gets `--split-count/--split-index` for all `opp_run_*` commands. The slice applies to the top-level task list only, before `randomize`. Check: for the INET fingerprint tasks and for the smoke tasks, the union of the 4 slices is the full list, the slices do not overlap, and each fingerprint group keeps all its ingredients. Done in opp_repl `c4129bd`. The split is in `MultipleTasks.run()`, not in the constructor, because `get_simulation_tasks()` passes its arguments to the inner list. Fact found: the filter runs came in the order of a `set`, which changes between processes; they are now sorted. Checked: 160 groups in the same order with two hash seeds; 4 slices of 40 groups hold each group once with its 3 ingredients; `opp_run_smoke_tests` splits 11 configs into 6 and 5.
- [x] **O4b. Fingerprint options.** `--ingredients`, `--stored-only/--no-stored-only`, `--append-args`; `cpu_time_limit: 600s` in `inet.opp`. Check: `--ingredients tyf` selects only the `tyf` entries, and `--append-args` reaches the `opp_run` command line. Done in opp_repl `b85f12d`, documentation `43f9708`. `--append-args` is on the two fingerprint commands only, because the fingerprint tasks give their own `append_args`. Checked: `--ingredients tyf` runs only `tyf` entries; `--append-args --no-such-option=1` reaches `opp_run`, which rejects it; the bundled `inet.opp` gives `cpu_time_limit: 600s`.
- [x] **O4c. Repairs found in the container.** `e469285`: `import opp_repl` failed in a fresh venv with IPython 9.17, because `common/util.py` used `IPython.terminal.interactiveshell` without the import; INET CI would have failed the same way. `cc39de2`: the ini parser replaced each line with a comment by the text `//1` (a typo for the group), so `sim-time-limit = 1ps # TODO` gave no limit; 13 INET configs change, and their abstract, network and run counts stay the same.
- [ ] **O5. Push to `opp_ci`.** Push the O1-O4 commits to `origin/opp_ci` (an outward-facing action that needs the maintainer's word). Then run the opp_ci fingerprint kind on master once to check that the workers still work. Check: the push succeeds, and the opp_ci run gives the same verdicts as before O1-O4, except for the 64 `ethernet-nonstandardspeed.ini` entries that now run.

### INET (branch `topic/fingerprint-tests-on-opp-repl`, worktree `/home/levy/workspace/inet-fingerprint-tests-on-opp-repl`, off master; decision D7)

- [x] **I1. Convert the CSV fingerprints.** A script in `plan/evidence/fingerprint-tests-on-opp-repl/` clears the store and imports all ingredients from the CSV files with opp_repl's importer. It then compares the store with the CSV files: 5439 entries, 1752 runs, 4 ERROR entries on `60814876c5`. The store is a generated file. After each rebase, re-run the script; do not merge the store by hand. One baseline-only commit under TR-BASELINE-PROVENANCE. Done: script `3b56dff555`, store `e6c8520635`. The conversion ran in the container of the GitHub job with the timestamp 1792589940 (commit time of `dcae92dcbf`, the newest CSV change); it found no difference from the CSV files, and two conversions gave the same bytes. Against the CSV files, the old store had 2519 different values, 133 entries without a CSV row and 1093 absent values.
- [x] **I2. Prove the same verdicts.** In the podman job copy, on one build, run the old tool with the CI arguments and the new tool with the store. Compare the verdict of each run. A difference stops the plan until its cause is known. Evidence goes to the plan evidence folder. Done in release mode, see [same-verdicts.md](../evidence/fingerprint-tests-on-opp-repl/same-verdicts.md): the old tool gave 1751 PASS and 1 expected ERROR for 1752 rows, the new tool 4378 PASS and 3 expected ERROR for 4381 entries of 1750 runs; the ERROR is the same run. The debug check of the converted entries is part of I8.
- [x] **I3a. Give a limit to the 14 configs without one.** For each config, run it without a limit and find out whether it stops by itself. Propose a `sim-time-limit` for each config to the maintainer. After approval, add the limits to the ini files and run the statistical and chart tests of these configs. A changed baseline goes into its own commit under TR-BASELINE-DELIBERATE. Done: ini `76750c2edc`, statistics `41d3fb5` (statistics branch `topic/fingerprint-tests-on-opp-repl`, not pushed). With the parser repair `cc39de2`, 10 of the 14 configs had the placeholder `1ps # TODO`, and `examples/empty` had no limit; 3 others ask for user input. Measured with a 30 s CPU limit: `examples/empty` and the three `osg/earth` configs have one event; `tutorials/protocol` Network1 to Network7 run 4000 to 12000 events in each second of simulation time. Decision D8: Network1 to Network7 get 10s, `examples/empty` and `osg/earth` get 1s. The statistical results of 8 configs change (Network1 to Network7, `osg/earth NetworkNodes`); `inet_run_statistical_tests` then gives 11 PASS.
- [x] **I3b. Record the absent run-0 fingerprints.** Record the three CI ingredients of the 100 configs at their ini limits with the update command in the podman copy. Record the absent ingredients of the 154 configs at their current limits. Measure the CPU time of each new run, and report the CI time that the new runs add for each slice. Report each config that gives ERROR, is not portable, or is interactive, and ask the maintainer about each one (decision D4). One baseline-only commit that lists the new configs and limits. Done: script `dad464c1d8` and `0d6b6f5159`, entries `a2192f8070` and `23f91141ba`, report `record-run-0-report.jsonl`. Pass 1 records tplx, ~tNl and ~tND in release mode and checks them in debug mode; pass 2 records tplx and ~tNl for each config whose ~tND fails. Result: 324 new run-0 entries, no stored value changed; 117 runs cannot have ~tND (no serializer for QUIC, EIGRP, GPSR, DYMO, link state, IPsec ESP, VoIP, LMAC and others; headers too short for their serializer; and in debug mode "serialized length is incorrect" for three 802.15.4 configs). Decision D4: `showcases/wireless/sensornetwork StatisticLMac` stores tplx and ~tNl entries that expect ERROR. 10 configs ask for user input and have no fingerprint; `tests/fingerprint/README` names them.
- [x] **I4. Switch GitHub CI.** `fingerprint-tests.yml` creates a venv, installs opp_repl with `pip install git+https://github.com/omnetpp/opp_repl.git@opp_ci`, and runs `opp_run_fingerprint_tests --load @opp -p inet -m $MODE --no-build --split-count 4 --split-index $I`. The other steps stay. Check: the job log shows the expected test count for each slice. Done in `ef87fcdc1c`. Checked in the container with these commands in a fresh venv (pandas 3.1.0, no dask): in both modes the 4 slices hold 1828 runs with 4670 entries, 4667 PASS and the 3 expected ERRORs, and each slice exits with 0 (release 43 to 50 s, debug 162 to 185 s on 32 cores). This also checks the converted entries in debug mode.
- [x] **I5. Repair the gates.** `fingerprint_moves.py` reads `tests/fingerprint/store.json` of the commit and of its parent (`git show`), pairs the entries by key, and reports the changed ingredient sets. `check-commits.sh` gives it the commit instead of a CSV diff. The fixtures in the gate tests use store files. Check: the gate tests pass; a sample commit that moves values in two ways gives the note. Done in `d5ca527458`. The note says "runs" for "rows", and a new test shows that new runs are not moves. The 190 gate tests pass.
- [x] **I6. Remove the old tool.** Remove `tests/fingerprint/fingerprinttest`, `bin/inet_fingerprinttest`, the CSV files with `examples.csv.unstable` and `examples-TODO.csv_off`, `test-branch`, the travis and jenkins fingerprint scripts, and the INET Python fingerprint layer with its imports. Rewrite `fingerprinttest_selfdoc` for the new tool. Remove the smoke test tool (`bin/inet_smoketest`, `tests/fingerprint/smoketest`, `tests/fingerprint/gen_runallexamples.pl`, `tests/fingerprint/valgrind.supp`) by decision D5. Check: a tree grep; `inet_run_statistical_tests --dry-run` still imports. Part done in `bbc4c2498b`: the INET Python fingerprint layer and its two `bin` wrappers are gone; `inet.simulation.compare` keeps `get_ingredients_append_args()`, its only use of that layer. Rest done in `212a3c4756`: also `_scripts/smoketestgen` (it only made smoke test CSV files) and the Travis fingerprint stage. The Jenkins config stays: it names `tests/fingerprint/fingerprints`, `tests/smoke/smoketest` and `tests/statistical/runtest`, which have been absent for a long time, and not the old tool. The selfdoc script parses and reaches `opp_run`; its `--generate-selfdoc` needs a build with `-DINET_WITH_SELFDOC`.
- [x] **I7. Documentation.** Rewrite `ch-testing.rst` (run, update, add a config, diagnose with `--append-args --fingerprint-modules=...`) and `tests/fingerprint/README`. Move the tutorial baseline into the store and rewrite the tutorial pages with the new commands; a tag becomes a config filter. Update the guides and, with seal permission, the rules. Add the WHATSNEW and migration guide entry files. Check: the documentation build and the `check-whatsnew.sh` gate pass. Done: developer guide, README, guides, checklist and the sealed texts in `4c9b07b2a2` (seal permission of 2026-10-22), tutorial in `c49d20781f`, release note and migration guide section in `212a3c4756`. The tutorial commands give the shown results in the container. Not changed: `plan/pending/ieee80211-compressed-blockack-replacement.md` and the evidence of other plans, which show old `./fingerprinttest` commands as their working material.
- [ ] **I8. Final check.** Rebase onto the latest master, re-run I1, re-run I2 for the changed rows, and record new configs of master. Push the branch for a pull request (an outward-facing action that needs the maintainer's word) and get green GitHub jobs. Run the opp_ci fingerprint kind on the same commit.

Dependencies: O1-O4 before I1-I3, because they need the explicit limits and the update command. I3a before I3b, because I3b reads the new ini limits. O5 before I4. I5 together with I6. I8 last.

## 7. Verification methods and expected results

Planned checks, not results:

- Conversion (I1): the script exits 0 only if each CSV value is in the store, and the store holds no other converted entry. Expected on `60814876c5`: 5439 entries, 1752 runs, 4 ERROR. A second run of the script gives no diff. It cannot show that the CSV values are correct; I2 does that.
- Verdicts (I2): both tools on one build and the same 1750 runs. Expected: the same verdict for each run, with the old tool green as on CI. It cannot show anything about the new run-0 entries.
- New entries (I3): each new entry is recorded twice, in debug and release mode, in the podman copy; both values must agree. It cannot show that the GitHub runner gives the same values; I8 does that.
- CI (I4, I8): green GitHub jobs in both modes, and the sum of the slice counts equals the store's run count.
- Gates (I5): `python3 -m unittest` in `doc/project/enforcement/`.
- Expected baseline change under TR-BASELINE-DELIBERATE: no converted value changes. The only new values are the I3b entries, and the statistical and chart values that I3a changes, if any.

## 8. Risks, unresolved decisions, and approval scope

Risks:

- Master changes the CSV files often (88 commits in six weeks). I1 is a script for this reason, and I8 re-runs it.
- Recorded values are not portable between compilers (section 4). I3 records in the podman copy, and I8 checks on GitHub.
- The CI run time grows by about 100 runs at their ini limits. Some limits are long (`examples/wireless/ieee80211levelofdetail` 1000s, `showcases/wireless/errorrate` 500s). I3b measures the added time before CI uses it.
- The `itervars` field stays (decision D2), but its value `$repetition==0` is wrong for runs other than 0 and for the filter runs. No code reads it. A later change can correct or remove it.
- The bundled `inet.opp` loads the project through `INET_ROOT`, and the bundled `omnetpp.opp` must find OMNeT++ in the GitHub job. I4 checks this assumption. If it fails, INET adds its own `inet.opp` at the root.
- Why not the earlier branch: it is 1331 commits behind, its store came from older CSV files, and it removes every GitHub workflow. A rebase would corrupt its store (section 2.5).
- INET CI installs a branch, not a fixed version (decision D6). A later push to `opp_ci` can change or break the INET fingerprint jobs. Each push to `opp_ci` therefore needs a fingerprint run on INET master first.

Decisions of the maintainer (2026-10-22):

- **D1, ingredients:** convert all ingredients, also `tyf`, `tpl` and `~tNlb`.
- **D2, store format:** keep the current layout, with a fixed field order.
- **D3, limit of a new run-0 entry:** the `sim-time-limit` of the ini file. A config without one gets one in its ini file (I3a).
- **D5, smoke test tool:** remove it in I6, because it reads the deleted CSV files and `opp_run_smoke_tests` replaces it.
- **D9, order with the redundancy removal:** this plan first. A new plan then does the redundancy removal again on master, one workflow at a time.
- **D4, a run 0 that gives ERROR:** I3b reports each one, and the maintainer decides each config. The probable decision is an expected ERROR entry with a reason. Configs whose values are not portable (z3 `gsandcs SAT`, osgEarth) and interactive configs get the same question.
- **D6, opp_repl in CI:** INET CI installs the `opp_ci` branch of opp_repl. The O-steps go to that branch.
- **D7, branch:** `topic/fingerprint-tests-on-opp-repl` with the worktree `/home/levy/workspace/inet-fingerprint-tests-on-opp-repl`, off master.
- **D10, `tyf` arguments:** opp_repl takes over the `tyf` arguments of the old tool.
- **D11, CI split:** keep 2 modes × 4 slices with a new `--split-count/--split-index` option. The option is generic, for all opp_repl test commands, not only for the fingerprint test. The slowest job then stays near 35 min (debug), plus a quarter of the time of the new run-0 configs.

Open decisions:

- **D8, the limits of I3a:** the maintainer approves one limit for each of the 14 configs, when I3a proposes them.
- **D4 for each config:** asked during I3b.

Separate permissions: push permission for O5 (opp_repl `opp_ci`) and for I8 (INET); seal permission for the rule edits in I7; the baseline commits of I1, I3a and I3b under TR-BASELINE-DELIBERATE.
