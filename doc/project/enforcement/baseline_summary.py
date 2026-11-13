#!/usr/bin/env python3
"""Computes the "Moved results:" block of a baseline commit (TR-BASELINE-PROVENANCE).

The block is the summary of one baseline change: how many tests and rows move, in which folders,
in which groups of tests, and for which result names, with the range of the relative change. A
group holds the tests whose moved rows have the same names; tests that move in different ways
usually need different explanations. The author puts the block into the message of the baseline
commit. check-baselines.sh computes it again and compares, so the
summary in a message is always true.

A stored result file (.sca) is compared as the statistical test compares it: by its scalar rows and
its run attributes, with numbers as numbers. A different text for the same number is no change. A
chart is compared by its content.

Not each file of a baseline repository is a result. In the statistics repository, the results are
the .sca files. In the media repository, the results are the images that a chart of an analysis file
(.anf) of the INET tree exports; the other files are documentation media. A file that is not a result
is not in the block, and it needs no record.

Usage, from the INET repository root:
  baseline_summary.py FOLDER               the staged changes of the baseline checkout in FOLDER
  baseline_summary.py FOLDER OLD NEW       the change between two commits of that repository

FOLDER is the folder of a baseline entry of inet.opp, for example statistics or media.

Exit status 0 = done, 2 = invalid usage.
"""
import math
import os
import re
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
CHART_PROPERTY = re.compile(r'<property name="image_export_filename" value="([^"]+)"/>')


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


def chart_images(analysis_files, read_link, entry):
    """The paths, in the media repository, of the images that the charts of analysis files export.

    The chart test writes the image of a chart to <folder of the .anf file>/<read folder>/<name>.png,
    and the read folder (doc/media) is a link into the clone of the media repository. analysis_files
    is {path of an .anf file: its text}. read_link gives the target of a link, or None. entry is the
    baseline entry of the chart tests in inet.opp."""
    media_folder = (entry.get("folder") or ".").rstrip("/") + "/"
    read_folder = entry.get("read_folder") or entry.get("folder") or "."
    images = set()
    for path, text in analysis_files.items():
        names = CHART_PROPERTY.findall(text)
        if not names:
            continue
        link = os.path.normpath(os.path.join(os.path.dirname(path), read_folder))
        target = read_link(link)
        if target is None:
            continue
        resolved = os.path.normpath(os.path.join(os.path.dirname(link), target)) + "/"
        if resolved.startswith(media_folder):
            images.update(resolved[len(media_folder):] + name + ".png" for name in names)
    return images


def chart_images_at(commit, entry, cwd=None):
    """The chart images of the analysis files of an INET commit. It reads git objects only."""
    out = subprocess.run(["git", "grep", "-h", "--full-name", "--heading", "-e", "image_export_filename", commit, "--", "*.anf"],
                         cwd=cwd, capture_output=True, text=True).stdout
    analysis_files, path = {}, None
    for line in out.splitlines():
        if line.startswith(commit + ":") and "<property" not in line:
            path = line[len(commit) + 1:]
            analysis_files[path] = ""
        elif path is not None:
            analysis_files[path] += line + "\n"

    def read_link(link):
        entry_line = subprocess.run(["git", "ls-tree", commit, link], cwd=cwd, capture_output=True, text=True).stdout
        if not entry_line.startswith("120000 "):
            return None
        return subprocess.run(["git", "cat-file", "-p", entry_line.split()[2]], cwd=cwd, capture_output=True, text=True).stdout
    return chart_images(analysis_files, read_link, entry)


def chart_images_in_work_tree(entry, cwd=None):
    """The chart images of the analysis files of the INET work tree, with the files that are not committed."""
    root = cwd or "."
    paths = subprocess.run(["git", "ls-files", "-co", "--exclude-standard", "*.anf"], cwd=cwd, capture_output=True, text=True).stdout.split("\n")
    analysis_files = {}
    for path in paths:
        if path and os.path.exists(os.path.join(root, path)):
            with open(os.path.join(root, path), encoding="utf-8", errors="replace") as f:
                analysis_files[path] = f.read()

    def read_link(link):
        full = os.path.join(root, link)
        return os.readlink(full) if os.path.islink(full) else None
    return chart_images(analysis_files, read_link, entry)


def result_filter(kind, images=()):
    """The function that says if a file of a baseline repository is a result of a test kind."""
    if kind == "statistical":
        return is_test
    if kind == "chart":
        return lambda path: path in images
    return is_result


def is_test(path):
    return path.endswith(".sca")


def analyze(folder, old=None, new=None, is_result=is_result):
    """The moved results between two commits of a baseline repository, or of its staged changes.
    is_result says which files are results; the other files are not in the answer.
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


def result_filter_of_folder(folder):
    """The result filter for the baseline checkout in a folder of the INET work tree. Outside an
    INET work tree, and for a folder that inet.opp does not name, each file is a result."""
    from baseline_pins import OPP_FILE, PinError, parse_baselines
    if not os.path.exists(OPP_FILE):
        return is_result
    try:
        with open(OPP_FILE, encoding="utf-8") as f:
            baselines = parse_baselines(f.read())
    except PinError:
        return is_result
    for kind, entry in baselines.items():
        if os.path.normpath(entry.get("folder") or ".") == os.path.normpath(folder):
            images = chart_images_in_work_tree(entry) | chart_images_at("HEAD", entry) if kind == "chart" else ()
            return result_filter(kind, images)
    return is_result


def main(argv=None):
    argv = sys.argv[1:] if argv is None else argv
    if len(argv) not in (1, 3):
        print(__doc__, file=sys.stderr)
        return 2
    try:
        print("\n".join(block(analyze(*argv, is_result=result_filter_of_folder(argv[0])))))
    except RuntimeError as error:
        print(f"error: {error}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
