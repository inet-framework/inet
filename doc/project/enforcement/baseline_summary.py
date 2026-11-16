#!/usr/bin/env python3
"""Computes the "Baseline changes:" list of a baseline commit (TR-BASELINE-PROVENANCE).

The list has one line for each category of the change: test files or chart images that are added,
removed, renamed or changed. A changed test file goes to the line of its size class and of the group
of its moved row names, because tests that move in different ways usually have different reasons.
The author writes the reason of each line after " — ". check-baselines.sh computes the lines again,
compares them, and checks that each line has a reason.

Size classes of a changed test file, by the largest relative change of its rows:
  large    1 % or more, a change from or to zero, a change that is not finite, or a changed text
  minor    less than 1 %
  noise    at most 1e-9, the level of the differences of another compiler or C library
  rows     no value changes; rows are only added or removed
In a group of row names, "+name" is a row that the change adds and "-name" a row that it removes.

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

HEADING = "Baseline changes:"
SEPARATOR = " — "
RUN_ATTRIBUTES = ("experiment", "measurement", "replication")
ATTRIBUTION_FOLDER = "attribution/"
GROUP_NAME_LIMIT = 12
NAMES_SHOWN = 6
RENAMES_SHOWN = 5
FOLDER_LIMIT = 10
NOISE = 1e-9
MINOR = 1e-2
CLASSES = ("large", "minor", "noise", "rows", "no value")
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


def size_class(changed, added, removed):
    """The size class of a changed test file."""
    if not changed:
        return "rows" if added or removed else "no value"
    largest = 0.0
    for _, old_value, new_value in changed:
        change = relative_change(old_value, new_value)
        if change is None:
            return "large"
        largest = max(largest, abs(change))
    return "noise" if largest <= NOISE else "minor" if largest < MINOR else "large"


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
    return set(chart_image_folders(analysis_files, read_link, entry))


def chart_image_folders(analysis_files, read_link, entry):
    """{path of a chart image in the media repository: folder of its analysis file in the INET tree}.
    The chart test selects its charts by that folder."""
    media_folder = (entry.get("folder") or ".").rstrip("/") + "/"
    read_folder = entry.get("read_folder") or entry.get("folder") or "."
    images = {}
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
            images.update({resolved[len(media_folder):] + name + ".png": os.path.dirname(path) for name in names})
    return images


def chart_images_at(commit, entry, cwd=None):
    """The chart images of the analysis files of an INET commit. It reads git objects only."""
    return set(chart_image_folders_at(commit, entry, cwd))


def chart_image_folders_at(commit, entry, cwd=None):
    """{chart image: folder of its analysis file} for an INET commit. It reads git objects only."""
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
    return chart_image_folders(analysis_files, read_link, entry)


def chart_images_in_work_tree(entry, cwd=None):
    """The chart images of the analysis files of the INET work tree, with the files that are not committed."""
    root = cwd or "."
    out = subprocess.run(["git", "ls-files", "-co", "--exclude-standard", "*.anf"], cwd=cwd, capture_output=True, text=True)
    if out.returncode == 0:
        paths = out.stdout.split("\n")
    else:
        # git cannot read the work tree, for example a worktree in a container that has only its files
        paths = []
        for folder, folders, names in os.walk(root):
            folders[:] = [name for name in folders if name not in (".git", "out", entry.get("folder") or ".")]
            paths += [os.path.relpath(os.path.join(folder, name), root) for name in names if name.endswith(".anf")]
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


def blob_ids(folder, old=None, new=None):
    """{path: (old object name, new object name)} of the changed files: between two commits, or of
    the staged changes. A side where the file does not exist has None."""
    args = ["diff", "--no-renames", "--raw", "--no-abbrev", "-z"]
    out = git(folder, *args, old, new) if old else git(folder, *args, "--cached")
    fields, ids = out.split("\0"), {}
    for i in range(0, len(fields) - 1, 2):
        _, _, old_id, new_id, _ = fields[i].split()
        ids[fields[i + 1]] = tuple(None if set(name) == {"0"} else name for name in (old_id, new_id))
    return ids


def analyze(folder, old=None, new=None, is_result=is_result):
    """The changed results between two commits of a baseline repository, or of its staged changes.
    is_result says which files are results; the other files are not in the answer.
    Returns {"tests": [...], "charts": [...], "renamed tests": [...], "renamed charts": [...]}.
    A test is (status, path, changed, added, removed), a chart is (status, path), a rename is
    (old path, new path). A removed and an added test file with the same scalar rows are a rename
    (the run attributes hold the name of the config, so they do not count); a removed and an added
    image with the same bytes are a rename. A rename is not in "tests" or "charts"."""
    old_revision, new_revision = (old, new) if old else ("HEAD", "")
    tests, charts, rows = [], [], {}
    ids = blob_ids(folder, old, new)
    for status, path in changed_files(folder, old, new):
        if not is_result(path):
            continue
        if is_test(path):
            old_rows = read_scalars(read_file(folder, old_revision, path)) if status != "A" else {}
            new_rows = read_scalars(read_file(folder, new_revision, path)) if status != "D" else {}
            rows[path] = old_rows if status == "D" else new_rows
            tests.append((status, path, *compare_scalars(old_rows, new_rows)))
        else:
            charts.append((status, path))

    def scalars(table):
        return {key: value for key, value in table.items() if key[0]}

    def pair(items, content):
        """Pairs each removed item with an added item of the same content; returns the renames and the rest."""
        added = [item for item in items if item[0] == "A"]
        renames, used = [], set()
        for item in items:
            if item[0] != "D":
                continue
            for other in added:
                if other[1] not in used and content(item[1], "old") == content(other[1], "new") and content(item[1], "old") is not None:
                    renames.append((item[1], other[1]))
                    used |= {item[1], other[1]}
                    break
        return renames, [item for item in items if item[1] not in used]

    def test_content(path, side):
        table = scalars(rows[path])
        return tuple(sorted((key, repr(value)) for key, value in table.items())) if table else None
    renamed_tests, tests = pair(tests, test_content)
    renamed_charts, charts = pair(charts, lambda path, side: ids.get(path, (None, None))[0 if side == "old" else 1])
    return {"tests": sorted(tests, key=lambda test: test[1]), "charts": sorted(charts, key=lambda chart: chart[1]),
            "renamed tests": sorted(renamed_tests), "renamed charts": sorted(renamed_charts)}


def group_folder(path):
    return "/".join(path.split("/")[:2])


def most_first(table):
    """The items of {name: count}, the largest count first, and in name order between equal counts."""
    return sorted(table.items(), key=lambda item: (-item[1], item[0]))


def folders_text(paths):
    """"<count>; <folder> <count>, ..." of a list of paths."""
    folders = {}
    for path in paths:
        folders[group_folder(path)] = folders.get(group_folder(path), 0) + 1
    shown = most_first(folders)[:FOLDER_LIMIT]
    text = ", ".join(f"{name} {count}" for name, count in shown)
    if len(folders) > len(shown):
        text += f", {len(folders) - len(shown)} other folders"
    return f"{len(paths)}; {text}"


def names_text(names, limit=NAMES_SHOWN):
    shown = list(names[:limit])
    return ", ".join(shown) + (f" and {len(names) - limit} more" if len(names) > limit else "")


def renames_text(renames):
    shown = [f"{old} -> {new}" for old, new in renames[:RENAMES_SHOWN]]
    return ", ".join(shown) + (f" and {len(renames) - RENAMES_SHOWN} more" if len(renames) > RENAMES_SHOWN else "")


def test_group(changed, added, removed):
    """The names of the moved rows of a changed test, with a mark for an added or a removed row."""
    return tuple(sorted({key[1] for key, _, _ in changed} | {"+" + key[1] for key in added} | {"-" + key[1] for key in removed},
                        key=lambda name: (name.lstrip("+-"), name)))


def lines(moved):
    """The computed lines of the "Baseline changes:" list, without their reasons."""
    result = []
    tests, charts = moved["tests"], moved["charts"]
    for status, word in (("A", "added"), ("D", "removed")):
        paths = [test[1] for test in tests if test[0] == status]
        if paths:
            result.append(f"statistical tests {word} ({folders_text(paths)})")
    if moved.get("renamed tests"):
        result.append(f"statistical tests renamed ({len(moved['renamed tests'])}): {renames_text(moved['renamed tests'])}")
    groups = {}
    for status, path, changed, added, removed in tests:
        if status == "M":
            names = test_group(changed, added, removed)
            groups.setdefault((size_class(changed, added, removed), names if len(names) <= GROUP_NAME_LIMIT else None), []).append((path, names))
    for (cls, group), members in sorted(groups.items(), key=lambda item: (CLASSES.index(item[0][0]), -len(item[1]), item[0][1] is None, item[0][1] or ())):
        if group is not None:
            names = list(group)
        else:
            # a group of tests with many moved rows: the names that move in the most tests first
            frequency = {}
            for _, member_names in members:
                for name in member_names:
                    frequency[name] = frequency.get(name, 0) + 1
            names = [name for name, _ in most_first(frequency)]
        result.append(f"statistical tests changed, {cls} ({folders_text([path for path, _ in members])}): {names_text(names)}")
    for status, word in (("A", "added"), ("D", "removed"), ("M", "changed")):
        paths = [chart[1] for chart in charts if chart[0] == status]
        if paths:
            stems = sorted(os.path.splitext(os.path.basename(path))[0] for path in paths)
            result.append(f"chart images {word} ({folders_text(paths)}): {names_text(stems)}")
    if moved.get("renamed charts"):
        result.append(f"chart images renamed ({len(moved['renamed charts'])}): {renames_text(moved['renamed charts'])}")
    return result


def block(moved, reasons=None):
    """The "Baseline changes:" list as lines of a message: each computed line, the separator, and its
    reason from {line: reason}, empty when unknown."""
    reasons = reasons or {}
    return [HEADING] + [f"  {line}{SEPARATOR}{reasons.get(line, '')}" for line in lines(moved)]


def read_block(message):
    """[(computed text, reason)] of the "Baseline changes:" list of a commit message, or None when the
    message has no list. An item starts with two spaces; a line with more indentation continues the
    reason of the item before it."""
    message_lines = message.splitlines()
    for index, line in enumerate(message_lines):
        if line.strip() == HEADING:
            items = []
            for item in message_lines[index + 1:]:
                if not item.strip():
                    break
                if item.startswith("    ") and items:
                    text, reason = items[-1]
                    items[-1] = (text, (reason + " " + item.strip()).strip())
                    continue
                text, _, reason = item.strip().partition(SEPARATOR.strip())
                items.append((text.strip(), reason.strip()))
            return items
    return None


def check_block(message, expected):
    """The problems of the list of a message against the computed lines: [] when it is correct."""
    items = read_block(message)
    if items is None:
        return [f"no '{HEADING}' list"]
    if [text for text, _ in items] != expected:
        return [f"a '{HEADING}' list that is not the computed list"]
    return [f"no reason for: {text}" for text, reason in items if not reason]


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
