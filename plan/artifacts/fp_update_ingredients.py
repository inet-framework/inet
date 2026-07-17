#!/usr/bin/env python3
"""Run opp_repl's update_fingerprint_test_results with explicit ingredients.

The opp_update_fingerprint_test_results CLI hardcodes ingredients="tplx"
(FingerprintUpdateTask.run_protected default); for wire-level parity baselines
we need ~tNl / ~tND entries. Must be run under the gated env (omnetpp setenv +
target INET worktree setenv) so @opp resolves the right project.

Usage: fp_update_ingredients.py <ingredients> <result_file> [working_dir_filter]
"""
import json
import sys

from opp_repl.common.util import initialize_logging
from opp_repl.simulation.workspace import determine_default_simulation_project, load_opp_file
from opp_repl.test import *

ingredients = sys.argv[1]
result_file = sys.argv[2]
wd_filter = sys.argv[3] if len(sys.argv) > 3 else None

initialize_logging("WARN", "WARN", None)
load_opp_file("@opp")
simulation_project = determine_default_simulation_project(name="inet")

kwargs = dict(simulation_project=simulation_project, mode="release", build=False,
              concurrent=True, user_interface="Cmdenv", run_number_filter="0",
              ingredients=ingredients, handle_exception=True)
if wd_filter:
    kwargs["working_directory_filter"] = wd_filter
result = update_fingerprint_test_results(**kwargs)

with open(result_file, "w") as f:
    json.dump(result.to_dict(), f, default=str)
sys.exit(0 if result.is_all_results_expected() else 1)
