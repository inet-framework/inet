#!/usr/bin/env python3
"""
Records the absent run-0 fingerprints into tests/fingerprint/store.json and checks them with the debug build. Plan
step I3b of plan/pending/fingerprint-tests-on-opp-repl.md.

Run it like convert-csv-to-store.py: from an INET checkout with the OMNeT++ and INET environment, opp_repl on
PYTHONPATH, and both the release and the debug build.

    python3 record-run-0.py --exclude <file> --report <file>

A config gets new entries if run 0 lacks one of the three CI ingredients. A run with entries keeps its stored limit,
so the same simulation must reproduce the stored values; a run without entries uses the limit of its ini file.

Pass 1 records tplx, ~tNl and ~tND in release mode and checks them in debug mode. ~tND computes the checksums, CRCs
and FCSs, which needs a serializer for each packet, so it fails for some protocols. A config fails pass 1 if its
update gives ERROR or its debug check gives an unexpected result. Pass 2 removes the run-0 entries that pass 1 added
for a failed config (pass 1 recorded them with the ~tND arguments), records tplx and ~tNl, and checks them in debug
mode. A config that asks for user input cannot run, and the report only names it.

The exclude file names configs to skip, one "<working directory> <ini file> <config>" line each. The report holds one
JSON line for each recorded run.
"""

import argparse
import importlib
import json

from opp_repl.simulation.workspace import load_opp_file
from opp_repl.simulation.project import get_simulation_project

store_module = importlib.import_module("opp_repl.test.fingerprint.store")
fptask = importlib.import_module("opp_repl.test.fingerprint.task")

CI_INGREDIENTS = ["tplx", "~tNl", "~tND"]
WITHOUT_TND = ["tplx", "~tNl"]

def flat(results):
    return sum((flat(r) for r in results.results), []) if hasattr(results, "results") else [results]

def config_key(simulation_config):
    return (simulation_config.working_directory, simulation_config.ini_file, simulation_config.config)

def entry_key(e):
    return (e["working_directory"], e["ini_file"], e["config"], json.dumps(e["run_number"]), e["sim_time_limit"], e["ingredients"])

def record_and_check(project, configs, ingredients_list):
    update_results = fptask.update_fingerprint_test_results(simulation_project=project, simulation_configs=configs, run_number=0, mode="release", build=False,
                                                            stored_only=False, ingredients_list=ingredients_list)
    store_module._correct_fingerprint_stores.clear()
    test_results = fptask.run_fingerprint_tests(simulation_project=project, simulation_configs=configs, run_number=0, mode="debug", build=False,
                                                stored_only=True, ingredients_list=ingredients_list)
    debug = {}
    for r in flat(test_results):
        debug.setdefault(config_key(r.task.simulation_task.simulation_config), []).append((r.result, r.expected, r.reason))
    return update_results, debug

def report_line(name, r, debug):
    t = r.task.simulation_task
    simulation_task_result = getattr(r, "simulation_task_result", None)
    return json.dumps({"pass": name, "config": " ".join(config_key(t.simulation_config)), "sim_time_limit": t.sim_time_limit,
                       "release": r.result, "debug": [d[0] for d in debug.get(config_key(t.simulation_config), [])],
                       "error": simulation_task_result.get_error_message() if simulation_task_result and r.result == "ERROR" else r.reason if r.result == "ERROR" else None,
                       "changes": [(i, e["fingerprint"] if e else None, str(c) if c else None) for i, e, c in getattr(r, "ingredient_updates", [])]})

def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exclude", help="a file with configs to skip")
    parser.add_argument("--report", required=True, help="the report file")
    args = parser.parse_args()
    excluded = set()
    if args.exclude:
        with open(args.exclude) as f:
            excluded = {tuple(line.split()) for line in f if line.strip() and not line.startswith("#")}
    load_opp_file("@opp")
    project = get_simulation_project("inet")
    store = store_module.get_correct_fingerprint_store(project)
    before = {entry_key(e) for e in store.get_entries()}
    run_0 = {}
    for e in store.get_entries():
        if e["run_number"] == 0:
            run_0.setdefault((e["working_directory"], e["ini_file"], e["config"]), set()).add(e["ingredients"])
    configs = [c for c in project.get_simulation_configs(build=False, mode="release")
               if config_key(c) not in excluded and not set(CI_INGREDIENTS) <= run_0.get(config_key(c), set())]
    print(f"pass 1: {len(configs)} configs lack a run-0 CI ingredient, {len(excluded)} excluded")
    update_results_1, debug_1 = record_and_check(project, configs, CI_INGREDIENTS)
    interactive, failed = set(), set()
    for r in update_results_1.results:
        key = config_key(r.task.simulation_task.simulation_config)
        error = getattr(r, "simulation_task_result", None) and r.simulation_task_result.get_error_message() or ""
        if r.result == "ERROR" and "prompt for user input" in error:
            interactive.add(key)
        elif (r.result == "ERROR" and not r.expected) or any(not expected for _, expected, _ in debug_1.get(key, [])):
            failed.add(key)
    store = store_module.get_correct_fingerprint_store(project)
    added = [e for e in store.get_entries() if e["run_number"] == 0 and (e["working_directory"], e["ini_file"], e["config"]) in failed and entry_key(e) not in before]
    for e in added:
        store.remove_entry(e)
    store.write()
    print(f"pass 2: {len(failed)} configs failed pass 1, {len(added)} of their pass-1 entries removed, {len(interactive)} configs ask for user input")
    failed_configs = [c for c in configs if config_key(c) in failed]
    update_results_2, debug_2 = record_and_check(project, failed_configs, WITHOUT_TND)
    with open(args.report, "w") as f:
        for r in update_results_1.results:
            f.write(report_line("1", r, debug_1) + "\n")
        for r in update_results_2.results:
            f.write(report_line("2", r, debug_2) + "\n")
    print("pass 1:", update_results_1)
    print("pass 2:", update_results_2)

if __name__ == "__main__":
    main()
