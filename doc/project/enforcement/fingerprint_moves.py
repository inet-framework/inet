# Compares two versions of tests/fingerprint/store.json and prints how the moved runs move.
#
# The store holds one entry for each run, sim-time-limit and ingredient set. A run is one
# simulation: its working directory, ini file, config and run number. A moved run changes the
# values of some of its ingredient sets. Runs that change different sets usually have different
# causes, so a commit with more than one pattern asks whether it holds more than one behavior
# change (PR-SPLIT-ONE-CHANGE).
#
# Usage: fingerprint_moves.py <old store> <new store>; an empty file is an empty store.
# Output: one line per pattern, "<runs>\t<changed ingredient sets>", most runs first, and in name
# order between equal counts, so that the same stores always give the same output.
# A run that is in one store only is a first recording or a retirement, not a move. A run whose
# values stay but whose limit or expected result changes counts as "(other fields)".
import collections
import json
import sys


def load(path):
    with open(path) as f:
        text = f.read()
    runs = collections.defaultdict(dict)
    for entry in json.loads(text) if text.strip() else []:
        run = (entry["working_directory"], entry["ini_file"], entry["config"], json.dumps(entry["run_number"]))
        runs[run][entry["ingredients"]] = (entry["fingerprint"], entry["sim_time_limit"], entry["test_result"])
    return runs


old, new = load(sys.argv[1]), load(sys.argv[2])
patterns = collections.Counter()
for run in sorted(old.keys() & new.keys()):
    if old[run] == new[run]:
        continue
    changed = sorted(i for i in old[run].keys() | new[run].keys() if old[run].get(i, (None,))[0] != new[run].get(i, (None,))[0])
    patterns[",".join(changed) or "(other fields)"] += 1

for pattern, runs in sorted(patterns.items(), key=lambda item: (-item[1], item[0])):
    print(f"{runs}\t{pattern}")
