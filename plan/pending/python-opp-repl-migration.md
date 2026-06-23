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

## Execution steps (commit after each)

- [ ] **Step 0** — Add `opp_repl` to `python/requirements.txt`. Verify `import opp_repl` under
  inet's PYTHONPATH. Commit.
- [ ] **Step 1 (Bucket 1)** — `git rm` the Bucket 1 files. Rewrite the package `__init__.py`
  facades (`common`, `simulation`, `documentation`, `test`) to pull generic API from `opp_repl`.
  Verify `python3 -c "import inet"` succeeds. Commit: *"python: Remove generic modules now provided by opp_repl (bucket 1)"*.
- [ ] **Step 2 (Bucket 2)** — Trim `common/util.py`, `common/github.py`; delete `simulation/task.py`,
  `simulation/optimize.py`; repoint imports in `documentation/ned.py`, `test/all.py`,
  `test/feature.py`, `test/release.py`, `test/fingerprint/old.py`, `test/fingerprint/task.py`,
  `repl.py`, `main.py`. Fix G5/G6/G7 in `fingerprint/task.py`; add explicit `import multiprocessing`
  / `import itertools` (G10/G11). Verify import + a smoke test. Commit: *"python: Reduce to INET-specific code, source generics from opp_repl (bucket 2)"*.
- [ ] **Step 3 (rewire entry points)** — Repoint the ~26 `bin/inet_*` wrappers (they do
  `from inet.main import *`). Keep INET-only commands (validation/packet/...) working via the
  trimmed `inet.main`; for purely-generic ones, either keep the inet shim or point at `opp_repl`
  equivalents. Verify each `bin/inet_*` runs `-h`. Commit: *"bin: Rewire python wrappers onto migrated package"*.
- [ ] **Step 4 (CI)** — Update the ~8 GitHub workflows that invoke these modules, if their
  invocation paths changed. Commit.
- [ ] **Step 5 (verify)** — `import inet`, run `inet_run_smoke_tests` on a tiny config, run
  `inet/test/self.py` if feasible. Record results in this plan. Move plan to `plan/done/`.

## Open questions / follow-ups (not blocking)
- cffi `inprocess` runner: opp_repl needs a hook to use `inet.cffi.InprocessSimulationRunner`
  instead of hardcoded `opp_repl.cffi`. Track separately.
- Whether to upstream `get_omnetpp_relative_path` into opp_repl (its own `test/release.py` uses it).
- G6/G7 are latent bugs present in opp_repl too — fix there as well or just in inet.
