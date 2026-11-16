# Compares two versions of tests/fingerprint/store.json: how the moved runs move, and the categories
# of the change for the "Baseline changes:" list of a commit (TR-BASELINE-PROVENANCE).
#
# The store holds one entry for each run, sim-time-limit and ingredient set. A run is one
# simulation: its working directory, ini file, config and run number. A moved run changes the
# values of some of its ingredient sets. Runs that change different sets usually have different
# causes, so a commit with more than one pattern asks whether it holds more than one behavior
# change (PR-SPLIT-ONE-CHANGE).
#
# Usage: fingerprint_moves.py [--list] <old store> <new store>; an empty file is an empty store.
# With --list, the output is the "Baseline changes:" list of the change, with no reasons, for the
# message of the commit.
# Output: one line per pattern, "<runs>\t<changed ingredient sets>", most runs first, and in name
# order between equal counts, so that the same stores always give the same output.
# A run that is in one store only is a first recording or a retirement, not a move. A run whose
# values stay but whose limit or expected result changes counts as "(other fields)".
#
# As a module, categories() gives the categories of a change, one line each:
#   fingerprint runs added | removed (N; folders)
#   fingerprint runs renamed (N): old -> new      a removed and an added run with equal fingerprints
#   fingerprint runs changed in <sets> (N; folders)            one line for each pattern of sets
#   fingerprint ingredient sets added | removed (N; folders): <sets>
#   fingerprint runs with another limit or expected result (N; folders)
import collections
import json
import sys

try:
    from baseline_summary import HEADING, SEPARATOR, folders_text, renames_text
except ImportError:  # the command line use without --list needs no other module
    HEADING = SEPARATOR = folders_text = renames_text = None


def parse(text):
    """{run: {ingredients: (fingerprint, sim time limit, expected test result)}} of the text of a store."""
    runs = collections.defaultdict(dict)
    for entry in json.loads(text) if text.strip() else []:
        run = (entry["working_directory"], entry["ini_file"], entry["config"], json.dumps(entry["run_number"]))
        runs[run][entry["ingredients"]] = (entry["fingerprint"], entry["sim_time_limit"], entry["test_result"])
    return runs


def load(path):
    with open(path) as f:
        return parse(f.read())


def patterns(old, new):
    """{pattern of changed ingredient sets: number of runs}, as the command line prints it."""
    found = collections.Counter()
    for run in sorted(old.keys() & new.keys()):
        if old[run] == new[run]:
            continue
        changed = sorted(i for i in old[run].keys() | new[run].keys() if old[run].get(i, (None,))[0] != new[run].get(i, (None,))[0])
        found[",".join(changed) or "(other fields)"] += 1
    return found


def run_text(run):
    """The name of a run as opp_repl prints it: the folder, then -f, -c and -r when they are not the defaults."""
    folder, ini, config, number = run
    return (folder + (f" -f {ini}" if ini != "omnetpp.ini" else "") + (f" -c {config}" if config != "General" else "")
            + (f" -r {number}" if number != "0" else ""))


def categories(old, new):
    """[(kind, line)] of the change from the store old to the store new, in a fixed order. The kind is
    added, removed, renamed, changed, sets added, sets removed or other; only "changed" moves a value
    that the store held before."""
    result = []
    added, removed = sorted(new.keys() - old.keys()), sorted(old.keys() - new.keys())

    def fingerprints(sets):
        return tuple(sorted((ingredients, values[0]) for ingredients, values in sets.items()))
    renames, used = [], set()
    for run in removed:
        for other in added:
            if other not in used and fingerprints(old[run]) == fingerprints(new[other]):
                renames.append((run_text(run), run_text(other)))
                used |= {run, other}
                break
    added = [run for run in added if run not in used]
    removed = [run for run in removed if run not in used]
    for kind, runs in (("added", added), ("removed", removed)):
        if runs:
            result.append((kind, f"fingerprint runs {kind} ({folders_text([run[0] for run in runs])})"))
    if renames:
        result.append(("renamed", f"fingerprint runs renamed ({len(renames)}): {renames_text(sorted(renames))}"))
    changed, sets_added, sets_removed, other = {}, {}, {}, []
    for run in sorted(old.keys() & new.keys()):
        before, after = old[run], new[run]
        moved = tuple(sorted(i for i in before.keys() & after.keys() if before[i][0] != after[i][0]))
        if moved:
            changed.setdefault(moved, []).append(run)
        if after.keys() - before.keys():
            sets_added.setdefault(tuple(sorted(after.keys() - before.keys())), []).append(run)
        if before.keys() - after.keys():
            sets_removed.setdefault(tuple(sorted(before.keys() - after.keys())), []).append(run)
        if not moved and any(before[i][1:] != after[i][1:] for i in before.keys() & after.keys()):
            other.append(run)
    for sets, runs in sorted(changed.items(), key=lambda item: (-len(item[1]), item[0])):
        result.append(("changed", f"fingerprint runs changed in {','.join(sets)} ({folders_text([run[0] for run in runs])})"))
    for kind, table in (("sets added", sets_added), ("sets removed", sets_removed)):
        for sets, runs in sorted(table.items(), key=lambda item: (-len(item[1]), item[0])):
            result.append((kind, f"fingerprint ingredient sets {kind.split()[1]} ({folders_text([run[0] for run in runs])}): {','.join(sets)}"))
    if other:
        result.append(("other", f"fingerprint runs with another limit or expected result ({folders_text([run[0] for run in other])})"))
    return result


def main():
    arguments = sys.argv[1:]
    listed = arguments[:1] == ["--list"]
    old, new = load(arguments[-2]), load(arguments[-1])
    if listed:
        print("\n".join([HEADING] + [f"  {line}{SEPARATOR}" for _, line in categories(old, new)]))
        return
    for pattern, runs in sorted(patterns(old, new).items(), key=lambda item: (-item[1], item[0])):
        print(f"{runs}\t{pattern}")


if __name__ == "__main__":
    main()
