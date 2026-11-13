#!/usr/bin/env python3
"""Computes the "Moved results:" block of a baseline commit (TR-BASELINE-PROVENANCE).

The block is the summary of one baseline change: how many tests and rows move, in which folders,
in which groups of tests, and for which result names, with the range of the relative change. A
group holds the tests whose moved rows have the same names; tests that move in different ways
usually need different explanations. The author puts the block into the message of the baseline
commit. check-baselines.sh computes it again and compares, so the
summary in a message is always true.

A stored result file (.sca) is compared as the statistical test compares it: by its scalar rows and
its run attributes, with numbers as numbers. A different text for the same number is no change. Any
other file is a chart and is compared by its content.

Usage, from the INET repository root:
  baseline_summary.py FOLDER               the staged changes of the baseline checkout in FOLDER
  baseline_summary.py FOLDER OLD NEW       the change between two commits of that repository

Exit status 0 = done, 2 = invalid usage.
"""
import math
import shlex
import subprocess
import sys

HEADING = "Moved results:"
RUN_ATTRIBUTES = ("experiment", "measurement", "replication")
ATTRIBUTION_FOLDER = "attribution/"
NAME_LIMIT = 20
GROUP_LIMIT = 10
GROUP_NAME_LIMIT = 12
FOLDER_LIMIT = 10
CHART_LIMIT = 30


def git(folder, *args, check=True, text=True):
    out = subprocess.run(["git", *args], cwd=folder, capture_output=True, text=text)
    if check and out.returncode != 0:
        error = out.stderr if text else out.stderr.decode(errors="replace")
        raise RuntimeError(f"git {' '.join(args)} failed in {folder}:\n{error.strip()}")
    return out.stdout


def read_scalars(text):
    """{(module, name): value} of the text of a result file: its scalar rows, and the run attributes
    that the statistical test compares. A value is a float when the text is a number."""
    rows = {}
    for line in text.splitlines():
        if line.startswith("scalar "):
            fields = shlex.split(line) if '"' in line else line.split()
            if len(fields) == 4:
                rows[(fields[1], fields[2])] = to_number(fields[3])
        elif line.startswith("attr "):
            fields = shlex.split(line) if '"' in line else line.split()
            if len(fields) == 3 and fields[1] in RUN_ATTRIBUTES:
                rows[("", "run attribute " + fields[1])] = fields[2]
    return rows


def to_number(text):
    try:
        return float(text)
    except ValueError:
        return text


def same(a, b):
    return a == b or (isinstance(a, float) and isinstance(b, float) and math.isnan(a) and math.isnan(b))


def compare_scalars(old, new):
    """(changed, added, removed): changed is [(key, old value, new value)], the others are [key]."""
    changed = [(key, old[key], new[key]) for key in old if key in new and not same(old[key], new[key])]
    return changed, [key for key in new if key not in old], [key for key in old if key not in new]


def relative_change(old, new):
    """The relative change of a row, or None when it has none: text, zero, or a value that is not finite."""
    if isinstance(old, float) and isinstance(new, float) and math.isfinite(old) and math.isfinite(new) and old != 0:
        return (new - old) / abs(old)
    return None


def percent(value):
    return f"{100 * value:+.3g} %"


def counts(changed, added, removed):
    return f"{changed} changed, {added} added, {removed} removed"


def changed_files(folder, old=None, new=None):
    """[(status, path)] with status A, M or D: between two commits, or of the staged changes."""
    args = ["diff", "--no-renames", "--name-status", "-z"]
    out = git(folder, *args, old, new) if old else git(folder, *args, "--cached")
    fields = out.split("\0")
    return [(fields[i], fields[i + 1]) for i in range(0, len(fields) - 1, 2)]


def read_file(folder, revision, path):
    """The text of a file at a commit; an empty revision reads the index."""
    return git(folder, "show", f"{revision}:{path}", text=False).decode("utf-8", errors="replace")


def is_result(path):
    return not path.startswith(ATTRIBUTION_FOLDER)


def is_test(path):
    return path.endswith(".sca")


def analyze(folder, old=None, new=None):
    """The moved results between two commits of a baseline repository, or of its staged changes.
    Returns {"tests": [...], "charts": [...]}: a test is (status, path, changed, added, removed),
    a chart is (status, path)."""
    old_revision, new_revision = (old, new) if old else ("HEAD", "")
    tests, charts = [], []
    for status, path in changed_files(folder, old, new):
        if not is_result(path):
            continue
        if is_test(path):
            old_rows = read_scalars(read_file(folder, old_revision, path)) if status != "A" else {}
            new_rows = read_scalars(read_file(folder, new_revision, path)) if status != "D" else {}
            tests.append((status, path, *compare_scalars(old_rows, new_rows)))
        else:
            charts.append((status, path))
    return {"tests": sorted(tests, key=lambda test: test[1]), "charts": sorted(charts, key=lambda chart: chart[1])}


def count_status(items):
    return counts(*(sum(1 for item in items if item[0] == status) for status in "MAD"))


def group_folder(path):
    return "/".join(path.split("/")[:2])


def most_first(table):
    """The items of {name: count}, the largest count first, and in name order between equal counts."""
    return sorted(table.items(), key=lambda item: (-item[1], item[0]))


def count_groups(moved):
    """The number of ways in which the changed tests move: the groups of the block."""
    groups = set()
    for status, _, changed, added, removed in moved["tests"]:
        if status == "M":
            names = sorted({key[1] for key, _, _ in changed} | {key[1] for key in added} | {key[1] for key in removed})
            groups.add(tuple(names) if len(names) <= GROUP_NAME_LIMIT else None)
    return len(groups)


def block(moved):
    """The lines of the "Moved results:" block."""
    lines = [HEADING]
    tests, charts = moved["tests"], moved["charts"]
    if tests:
        lines.append(f"  tests: {count_status(tests)}")
        # the rows of a test that the commit adds or removes are all new or all gone; they say
        # nothing about a move, so the row counts and the names are of the changed tests only
        changed_tests = [test for test in tests if test[0] == "M"]
        lines.append("  rows of the changed tests: " + counts(*(sum(len(test[i]) for test in changed_tests) for i in (2, 3, 4))))
        folders = {}
        for test in tests:
            folders[group_folder(test[1])] = folders.get(group_folder(test[1]), 0) + 1
        shown = most_first(folders)[:FOLDER_LIMIT]
        lines.append("  folders: " + ", ".join(f"{name} {count}" for name, count in shown)
                     + (f", {len(folders) - len(shown)} other folders" if len(folders) > len(shown) else ""))
        names, groups = {}, {}
        for _, _, changed, added, removed in changed_tests:
            moved_names = sorted({key[1] for key, _, _ in changed} | {key[1] for key in added} | {key[1] for key in removed})
            group = tuple(moved_names) if len(moved_names) <= GROUP_NAME_LIMIT else None
            groups[group] = groups.get(group, 0) + 1
            for (_, name), old_value, new_value in changed:
                names.setdefault(name, [0, 0, 0, []])[0] += 1
                change = relative_change(old_value, new_value)
                if change is not None:
                    names[name][3].append(change)
            for _, name in added:
                names.setdefault(name, [0, 0, 0, []])[1] += 1
            for _, name in removed:
                names.setdefault(name, [0, 0, 0, []])[2] += 1
        if changed_tests:
            lines.append("  groups of the changed tests, by the names of their moved rows:")
            ordered_groups = sorted(groups.items(), key=lambda item: (-item[1], item[0] is None, item[0] or ()))
            for group, count in ordered_groups[:GROUP_LIMIT]:
                lines.append(f"  - {count} test(s): " + (", ".join(group) if group is not None else f"more than {GROUP_NAME_LIMIT} names each"))
            if len(ordered_groups) > GROUP_LIMIT:
                lines.append(f"  - {len(ordered_groups) - GROUP_LIMIT} other groups: {sum(count for _, count in ordered_groups[GROUP_LIMIT:])} test(s)")
            lines.append("  names of the moved rows:")
        ordered = most_first({name: sum(entry[:3]) for name, entry in names.items()})
        for name, _ in ordered[:NAME_LIMIT]:
            changed, added, removed, changes = names[name]
            parts = [f"{count} {word}" for count, word in ((changed, "changed"), (added, "added"), (removed, "removed")) if count]
            if changes:
                parts.append(f"{percent(min(changes))} to {percent(max(changes))}")
            lines.append(f"  - {name}: {', '.join(parts)}")
        if len(ordered) > NAME_LIMIT:
            lines.append(f"  - {len(ordered) - NAME_LIMIT} other names: {sum(count for _, count in ordered[NAME_LIMIT:])} rows")
    if charts:
        lines.append(f"  charts: {count_status(charts)}")
        words = {"M": "changed", "A": "added", "D": "removed"}
        for status, path in charts[:CHART_LIMIT]:
            lines.append(f"  - {path}: {words[status]}")
        if len(charts) > CHART_LIMIT:
            lines.append(f"  - {len(charts) - CHART_LIMIT} other charts")
    if not tests and not charts:
        lines.append("  no result moves")
    return lines


def find_block(message):
    """The lines of the "Moved results:" block of a commit message, or None."""
    lines = message.splitlines()
    for index, line in enumerate(lines):
        if line.strip() == HEADING:
            end = index + 1
            while end < len(lines) and lines[end].strip():
                end += 1
            return [HEADING] + [line.rstrip() for line in lines[index + 1:end]]
    return None


def main(argv=None):
    argv = sys.argv[1:] if argv is None else argv
    if len(argv) not in (1, 3):
        print(__doc__, file=sys.stderr)
        return 2
    try:
        print("\n".join(block(analyze(*argv))))
    except RuntimeError as error:
        print(f"error: {error}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
