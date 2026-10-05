# Reads a unified diff of tests/fingerprint/*.csv on stdin and prints how the moved rows move.
#
# A row is one simulation run: "<directory>, <arguments>, <time limit>, <fingerprints>, ...".
# The fingerprint field holds one "<value>/<ingredients>" part for each ingredient set, separated
# by ';'. A moved row changes some of those parts. Rows that change different sets of parts
# usually have different causes, so a commit with more than one pattern asks whether it holds
# more than one behavior change (PR-SPLIT-ONE-CHANGE).
#
# Output: one line per pattern, "<rows>\t<changed ingredient sets>", most rows first, and in name
# order between equal counts, so that the same diff always gives the same output.
# A row that is only added or only removed is a first recording or a retirement, not a move.
import collections
import sys


def parse(row):
    fields = [f.strip() for f in row.split(",")]
    if len(fields) < 4 or fields[0].startswith("#"):
        return None
    parts = {}
    for part in fields[3].split(";"):
        value, _, ingredients = part.strip().partition("/")
        parts[ingredients or value] = value
    return (fields[0], fields[1]), parts


removed, added = {}, {}
path = ""
for line in sys.stdin:
    line = line.rstrip("\n")
    if line.startswith("+++ "):
        path = line[6:] if line.startswith("+++ b/") else line[4:]
    elif line.startswith("--- "):
        continue
    elif line.startswith("-") or line.startswith("+"):
        parsed = parse(line[1:])
        if parsed:
            key, parts = parsed
            (removed if line[0] == "-" else added)[(path, key)] = parts

patterns = collections.Counter()
for key in sorted(removed.keys() & added.keys()):
    old, new = removed[key], added[key]
    changed = sorted(k for k in old.keys() | new.keys() if old.get(k) != new.get(k))
    patterns[",".join(changed) or "(other fields)"] += 1

for pattern, rows in sorted(patterns.items(), key=lambda item: (-item[1], item[0])):
    print(f"{rows}\t{pattern}")
