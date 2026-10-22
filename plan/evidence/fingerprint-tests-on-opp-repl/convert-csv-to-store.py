#!/usr/bin/env python3
"""
Converts the fingerprint CSV files (tests/fingerprint/*.csv) into tests/fingerprint/store.json, then checks the
result against an independent parse of the same CSV files.

Run it from an INET checkout whose environment is set up (the OMNeT++ and INET setenv scripts), with opp_repl on
PYTHONPATH, after a build: opp_repl's importer reads an empty CSV limit from the ini file, and the config
collection needs the INET binaries. Plan step I1 of plan/pending/fingerprint-tests-on-opp-repl.md.

    python3 convert-csv-to-store.py --timestamp <commit time of the CSV revision>

The timestamp of every entry is this fixed value, so the same CSV files always give the same store.
"""

import argparse
import csv
import glob
import importlib
import json
import os
import re
import shlex
import sys

from opp_repl.simulation.workspace import load_opp_file
from opp_repl.simulation.project import get_simulation_project

store_module = importlib.import_module("opp_repl.test.fingerprint.store")
old = importlib.import_module("opp_repl.test.fingerprint.old")

def convert(project, timestamp):
    store = store_module.get_correct_fingerprint_store(project)
    store.clear()
    old.update_correct_fingerprints_from_csvs(project, timestamp=timestamp)
    with open(store.file_name) as f:
        return f.read()

def get_ini_sim_time_limit(ini_path):
    # the CSV rows without a limit all use tests/fingerprint/*.ini, which set the limit in [General]
    with open(ini_path) as f:
        for line in f:
            match = re.match(r"\s*sim-time-limit\s*=\s*(\S+)", line)
            if match:
                return match.group(1)
    return None

def parse_csv_files(inet_root):
    expected = {}
    for csv_file in sorted(glob.glob(os.path.join(inet_root, "tests/fingerprint/*.csv"))):
        with open(csv_file, "rb") as f:
            lines = [re.sub(r" *#.*$", "", line.decode("utf-8")) for line in f]
        for fields in csv.reader(lines, delimiter=",", quotechar="'", skipinitialspace=True):
            if not fields:
                continue
            assert len(fields) == 6, f"{csv_file}: {fields}"
            working_directory = "tests/fingerprint" if fields[0] == "." else fields[0].strip("/")
            args = shlex.split(fields[1])
            options = {"-f": "omnetpp.ini", "-c": "General", "-r": "0"}
            for i, arg in enumerate(args):
                if arg in options:
                    options[arg] = args[i + 1]
            run = options["-r"].strip()
            run_number = int(run) if re.fullmatch(r"-?\d+", run) else run
            sim_time_limit = fields[2].strip() or get_ini_sim_time_limit(os.path.join(inet_root, working_directory, options["-f"]))
            for text in fields[3].split(";"):
                fingerprint, ingredients = text.strip().split("/", 1)
                key = (working_directory, options["-f"], options["-c"], json.dumps(run_number), sim_time_limit, ingredients)
                value = (fingerprint, fields[4])
                assert expected.get(key, value) == value, f"two values for {key}: {expected[key]} and {value}"
                expected[key] = value
    return expected

def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--timestamp", type=float, required=True, help="the timestamp of every entry")
    args = parser.parse_args()
    load_opp_file("@opp")
    project = get_simulation_project("inet")
    inet_root = project.get_full_path(".")
    first = convert(project, args.timestamp)
    store_module._correct_fingerprint_stores.clear()
    second = convert(project, args.timestamp)
    entries = json.loads(second)
    actual = {(e["working_directory"], e["ini_file"], e["config"], json.dumps(e["run_number"]), e["sim_time_limit"], e["ingredients"]): (e["fingerprint"], e["test_result"]) for e in entries}
    expected = parse_csv_files(inet_root)
    runs = {key[:5] for key in actual}
    print(f"entries: {len(entries)}, distinct keys: {len(actual)}, runs: {len(runs)}, "
          f"ERROR: {sum(e['test_result'] == 'ERROR' for e in entries)}, without a limit: {sum(e['sim_time_limit'] is None for e in entries)}")
    problems = []
    if first != second:
        problems.append("two imports gave different files")
    if len(entries) != len(actual):
        problems.append("the store has two entries with one key")
    for key in sorted(set(expected) - set(actual)):
        problems.append(f"not in the store: {key} {expected[key]}")
    for key in sorted(set(actual) - set(expected)):
        problems.append(f"not in the CSV files: {key} {actual[key]}")
    for key in sorted(set(actual) & set(expected)):
        if actual[key] != expected[key]:
            problems.append(f"different: {key} store {actual[key]} CSV {expected[key]}")
    for problem in problems[:50]:
        print(problem)
    print("OK: the store equals the CSV files" if not problems else f"{len(problems)} problems")
    sys.exit(1 if problems else 0)

if __name__ == "__main__":
    main()
