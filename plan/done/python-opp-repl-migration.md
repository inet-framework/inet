# Migrate INET Python tooling onto `opp_repl`

## Goal

Remove from `inet-master/python/inet` all generic OMNeT++ simulation/testing
infrastructure that has been generalized into the standalone **`opp_repl`** package, and
rewire the surviving INET-specific code to import that functionality from `opp_repl`
instead of from `inet`.

End state: `python/inet` contains **only INET-specific code** and depends on `opp_repl`
for everything generic.

## Context / facts established

- `opp_repl` is the generalized successor of `python/inet`. It is pip-installed editable
  (`v0.5.dev80`) pointing at `/home/levy/workspace/opp_repl/opp_repl`, so `import opp_repl`
  works and `from opp_repl import *` exposes the generic API at top level.
- inet's Python is made importable via `PYTHONPATH=$INET_ROOT/python` (see `setenv`); it does
  **not** import `opp_repl` anywhere yet.
- Worktree for this work: `/home/levy/workspace/inet-master-pymigration`, branch
  `python/opp-repl-migration` (off `master`).

### opp_repl API confirmed available at top level
`define_simulation_project`, `get_simulation_project`, `determine_default_simulation_project`,
`run_simulations`, `get_simulation_tasks`, `build_project`, `build_project_using_tasks`,
`make_makefiles`, `clean_project`, `SimulationProject`, `SimulationTask`, `TestTask`,
`TestTaskResult`, `MultipleTestTasks`, `MultipleTestTaskResults`, `TaskTestTask`,
`run_command_with_logging`, `initialize_logging`, `SSHCluster`, `combine_signatures`,
`matches_filter`, `convert_to_seconds`, `run_smoke_tests`, `run_fingerprint_tests`,
`run_all_tests`, `get_opp_test_tasks`, `get_feature_test_tasks`, `run_release_tests`,
`register_key_bindings`, `enable_autoreload`, `import_user_module`,
`get_correct_fingerprint_store`, `Fingerprint`, `get_workspace_path` (different semantics — see G8).

### Confirmed gaps (names NOT in opp_repl that surviving INET files use)
- **G1 `get_omnetpp_relative_path`** — not in opp_repl. Keep in inet (INET/omnetpp env-specific).
- **G2 `get_inet_relative_path`** — not in opp_repl (uses `INET_ROOT`). Keep in inet.
- **G3 `update_correct_fingerprints`** — renamed to `update_fingerprint_test_results` in opp_repl.
  INET's `inet/test/fingerprint/task.py` defines its own `update_correct_fingerprints`; the
  surviving INET callers (`main.py`, `self.py`) keep using the INET name, so no change unless that
  file is deleted (it is Bucket 2 — see below).
- **G5 `SimulationRun`** — used at `inet/test/fingerprint/task.py:312`; should be `SimulationTask`.
- **G6 `get_all_simulation_configs(project)`** — called as a free function but only exists as a
  method; pre-existing latent bug in both repos.
- **G7 `MultipleTestResults`** — referenced at `inet/test/fingerprint/task.py:168`; pre-existing
  latent bug (likely `MultipleTestTaskResults`).
- **G8 `get_workspace_path`** — exists in opp_repl but with different semantics (`WORKSPACE_ROOT`
  env var vs inet's `__omnetpp_root_dir/..`). `inet/project/inet.py:32` relies on inet semantics
  → keep inet's `get_workspace_path`, import it last so it wins, or set `WORKSPACE_ROOT`.
- **G10 `multiprocessing` / G11 `itertools`** — inet files relied on these leaking through star
  imports of generic modules; add explicit `import` statements where used.
- **cffi inprocess runner** — opp_repl `simulation/task.py:575` hardcodes `import opp_repl.cffi`
  for the `inprocess` runner, but opp_repl has no `cffi` module (that's `inet.cffi`). The default
  runners (subprocess/ide) are unaffected. **Out of scope** for file removal; flagged as follow-up
  (opp_repl needs a registration hook for a project-specific inprocess runner).

## File classification

### Bucket 1 — fully replaced, delete (no INET-specific content)
```
common/cluster.py  common/compile.py  common/ide.py  common/summary.py  common/task.py  common/__init__.py
documentation/chart.py  documentation/__init__.py
simulation/build.py  simulation/compare.py  simulation/config.py  simulation/eventlog.py
simulation/fingerprint.py  simulation/iderunner.py  simulation/project.py  simulation/stdout.py
simulation/subprocess.py  simulation/__init__.py
test/chart.py  test/coverage.py  test/opp.py  test/profile.py  test/sanitizer.py
test/simulation.py  test/smoke.py  test/statistical.py  test/task.py
test/fingerprint/__init__.py  test/fingerprint/store.py
test/speed/__init__.py  test/speed/store.py  test/speed/task.py
__main__.py
```
(`test/fingerprint/store.py` and `test/speed/store.py` are byte-identical to opp_repl after
namespace rename.)

### Bucket 2 — mostly generic, carry a little INET glue (trim + delete or repoint)
| File | INET content | Disposition |
|---|---|---|
| `common/util.py` | `get_inet_relative_path`, `get_omnetpp_relative_path`, `get_workspace_path` | **Keep, trim** to only these INET helpers; body sourced from opp_repl |
| `common/github.py` | `start_validation_tests_github_workflow()` | **Keep, trim** to INET dispatch; `dispatch_workflow` from opp_repl |
| `common/__init__.py` | aggregator | **Keep** as facade: `from opp_repl.common import *` + INET util/github |
| `simulation/task.py` | cffi inprocess hook | **Delete**; inprocess path handled opp_repl-side (follow-up) |
| `simulation/optimize.py` | `inet.cffi.libinet` check | **Delete** (opp_repl has its own) |
| `documentation/ned.py` | `inet_project` | **Keep, repoint** imports to opp_repl + `inet.project.inet` |
| `test/all.py` | INET test suites (packet/queueing/protocol/module/unit) | **Keep, repoint**; generic part from opp_repl |
| `test/feature.py` | `inet.project.inet`, `inet.validation` | **Keep, repoint** |
| `test/release.py` | `inet_project`, SelfDoc | **Keep, repoint** |
| `test/fingerprint/old.py` | `inet_project`, `tests/fingerprint/*.csv` | **Keep, repoint** |
| `test/fingerprint/task.py` | `inet.project.inet` + G5/G6/G7 bugs | **Keep, repoint + fix bugs** |
| `test/__init__.py` | imports `inet.test.validation` | **Keep, repoint** facade |
| `repl.py`, `main.py`, `__init__.py` | INET project/validation glue | **Keep, repoint** |

### Bucket 3 — INET-specific, keep as-is (only fix imports if they referenced deleted modules)
```
cffi/*            project/*         scave/*
documentation/html.py               documentation/__init__.py
test/validation.py                  test/self.py
inetgdb/*         lldb/inet/formatter.py
```

## Rewiring strategy

**Facade approach** to minimize churn: keep the `inet`, `inet.common`, `inet.simulation`,
`inet.test`, `inet.documentation` packages as thin facades whose `__init__.py` re-exports the
generic API from `opp_repl` plus the surviving INET-specific names.

- `inet/common/__init__.py`: `from opp_repl.common import *` then `from inet.common.util import *`,
  `from inet.common.github import *` (INET helpers win on name clash → fixes G8).
- `inet/simulation/__init__.py`: `from opp_repl.simulation import *` (no INET-specific simulation
  modules survive).
- `inet/documentation/__init__.py`: `from opp_repl.documentation import *` then `from inet.documentation.html import *`, `ned`.
- `inet/test/__init__.py`: `from opp_repl.test import *` then INET test types (`all`, `feature`,
  `release`, `validation`, `fingerprint.old/task`).
- `inet/__init__.py`: `from opp_repl import *` then `from inet.project import *`,
  `inet.documentation`, `inet.scave`, `inet.test`; keep the INET docstring (trimmed).

Surviving INET-specific modules replace `from inet.<generic> import *` with `from opp_repl import *`
(or specific opp_repl submodule) and keep `from inet.common.util import *` / `from inet.project.inet
import *` for INET names.

## Execution steps (commit after each) — ALL DONE

- [x] **Step 0** — Verified `import opp_repl` under inet's PYTHONPATH. `opp_repl` is **not** a
  released pip package — it is obtained from its GitHub repo and put on `PYTHONPATH` manually (like
  the other workspace repos), so it is intentionally **not** listed in `python/requirements.txt`.
- [x] **Step 1 (Bucket 1)** — `git rm`'d the Bucket 1 files (plus generic-bodied `simulation/task.py`
  and `simulation/optimize.py`); rewrote the `common`/`simulation`/`documentation`/`test`/`test.fingerprint`
  `__init__.py` facades onto `opp_repl`; repointed surviving files. `import inet` verified.
  Commit `5777e77`.
- [x] **Step 2 (Bucket 2)** — Trimmed `common/util.py` (613→108 lines, INET-only helpers);
  `common/github.py` kept as-is (already INET-specific). Added explicit stdlib imports
  (`os`, `re`, `sys`, `itertools`, `multiprocessing`, `builtins`, `glob`, `datetime`) that opp_repl
  no longer leaks through `*`. Repointed `scave/plot.py` to `opp_repl.common.util`; fixed
  `release.py` logger names to the `opp_repl.*` namespace. Commit `9d6cd3d`.
- [x] **Step 2b (project definitions → .opp)** — *Superseded approach.* First adapted
  `project/inet.py`/`project/omnetpp.py` to the opp_repl `SimulationProject` kwargs (commit `d931248`),
  then — per the project owner — **removed the `inet.project` package entirely** because opp_repl
  defines projects via `.opp` descriptor files and ships bundled `inet.opp`/`omnetpp.opp`.
  `repl.py`/`main.py` now `load_opp_file("@opp")`; `inet_project` references became
  `get_default_simulation_project()` (runtime) or `None`+lazy-resolve (default args). Commit `d02e726`.
- [x] **Step 3 (entry points)** — No changes needed: the ~26 `bin/inet_*` wrappers call
  `inet.main` / `inet.repl` / `inet.test.self`, all of which survive. Verified representative
  wrappers run `-h` and execute.
- [x] **Step 4 (CI)** — The `inet_run_*` wrappers are unchanged, so workflows that invoke them are
  unaffected. However, `opp_repl` is **not** installed via `requirements.txt` (it is not a released
  pip package), so CI must make `opp_repl` importable another way — clone its GitHub repo and add it
  to `PYTHONPATH` (or `pip install` from the git URL) as a separate setup step. See follow-up below.
- [x] **Step 5 (verify)** — `import inet` + all submodules OK; `py_compile` clean; all `bin/inet_*`
  `-h` OK; **end-to-end `inet_run_smoke_tests -w ethernet/lans` → 11 PASS** with self-bootstrapped
  inet/omnetpp projects from the bundled `.opp` files (no manual registration).

## Final state
`python/inet` now contains only INET-specific code: `cffi/`, `scave/`, `documentation/html.py`+`ned.py`,
`test/validation.py`, `test/self.py`, the INET-specific test types in `test/all.py`/`feature.py`/`release.py`/
`fingerprint/{old,task}.py`, `common/util.py` (INET path/type helpers) + `common/github.py`, `main.py`,
`repl.py`, `inetgdb/`, `lldb/`. Everything generic comes from `opp_repl`; projects come from `.opp` files.

## Open questions / follow-ups (not blocking)
- **opp_repl availability**: `opp_repl` is not a released pip package; it lives on GitHub and is put
  on `PYTHONPATH` manually (like omnetpp). Therefore it is deliberately absent from
  `python/requirements.txt`. Dev setup and CI both need a separate step to fetch opp_repl from GitHub
  and expose it on `PYTHONPATH` (e.g. a checkout + `export PYTHONPATH=.../opp_repl:$PYTHONPATH`, or
  `pip install git+https://github.com/omnetpp/opp_repl`). The required API includes
  `load_opp_file("@opp")`, `define_omnetpp_project`, the `workspace` module, and
  `update_fingerprint_test_results`, so a recent enough checkout is needed.
- **cffi `inprocess` runner**: opp_repl `simulation/task.py` hardcodes `import opp_repl.cffi`; INET's
  inprocess runner lives in `inet.cffi`. opp_repl needs a registration hook for a project-specific
  inprocess runner. Default subprocess/ide runners are unaffected.
- **inet-baseline.opp**: not bundled. `run_baseline_fingerprint_test` now lazily resolves
  `get_simulation_project("inet-baseline")`; baseline fingerprint comparison needs an
  `inet-baseline.opp` (e.g. in the inet-baseline checkout) before it works.
- **Pre-existing latent bugs** (predate this migration, left untouched): `SimulationRun` (undefined,
  should be `SimulationTask`) and `MultipleTestResults` (undefined) in `test/fingerprint/task.py`;
  `get_all_simulation_configs(project)` called as a free function (method only) — present in opp_repl too.
- **opp_repl bug**: `InifileContents` is referenced but not imported in `opp_repl/simulation/project.py`
  (~line 815) on a config-listing fallback path; and its own `test/release.py` calls
  `get_omnetpp_relative_path`, which isn't defined in opp_repl. Both are opp_repl-side issues.
