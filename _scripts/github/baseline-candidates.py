#!/usr/bin/env python3
"""Makes the candidate baseline files after a run of the statistical or the chart tests that failed.

The CI job runs the tests with --result-file and, when they fail, calls this script. The script
selects the tests with an unexpected FAIL, and the tests with an ERROR because no stored result
exists. It runs the update command for these tests only, and copies each result file that the update
adds or changes to the output folder, at its path in the baseline repository. The output folder also
gets the "Moved results:" block of the change (moved-results.txt), the files that the change removes
(removed.txt), and the versions of the environment (environment.txt).

The candidates are evidence, not a baseline (TR-BASELINE-DELIBERATE in doc/project/rule/testing.md).
The author examines them, leaves out the files of the tests that fail at the parent commit too
(TR-BASELINE-PARENT), and commits the rest to the baseline repository with the record.

Usage, from the INET repository root, after the tests:
  baseline-candidates.py statistical|chart RESULT_FILE OUTPUT_FOLDER --update COMMAND

COMMAND is the shell command of the update, for example "opp_update_statistical_test_results --load
. -p inet -m release --no-build"; the script appends the filter of the tests.

Exit status 0 = done (also when no test needs a candidate), 2 = invalid usage or the script cannot run.
"""
import argparse
import json
import os
import re
import shlex
import shutil
import subprocess
import sys

ENFORCEMENT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "doc", "project", "enforcement")
sys.path.insert(0, os.path.normpath(ENFORCEMENT))
import baseline_summary  # noqa: E402
from baseline_pins import OPP_FILE, parse_baselines  # noqa: E402

# the parameters of a statistical test: "<working directory>[ -f <ini>][ -c <config>][ -r <run>][ for <limit>]"
STATISTICAL_TEST = re.compile(r"^(?P<folder>\S+)(?: -f (?P<ini>\S+))?(?: -c (?P<config>\S+))?(?: -r (?P<run>\d+))?(?: for .*)?$")
# the parameters of a chart test: "<analysis file>: <chart name>"
CHART_TEST = re.compile(r"^(?P<analysis>\S+\.anf): ")
NO_STORED_RESULT = ("Stored statistical results are not found", "Baseline chart not found")
WORK_FILES = re.compile(r"-(old|new|diff)\.png$")


def git(folder, *args):
    return subprocess.run(["git", *args], cwd=folder, capture_output=True, text=True, check=True).stdout


def selected_tests(results):
    """The parameter texts of the tests that need a candidate."""
    selected = []
    for result in results:
        if result.get("expected"):
            continue
        reason = result.get("reason") or ""
        if result.get("result") == "FAIL" or (result.get("result") == "ERROR" and reason.startswith(NO_STORED_RESULT)):
            selected.append(result.get("parameters") or "")
    return selected


def test_filter(kind, tests):
    """The option and the value that select the tests in the update command."""
    if kind == "statistical":
        parts = []
        for test in tests:
            match = STATISTICAL_TEST.match(test)
            if match is None or (match["run"] not in (None, "0")):
                raise ValueError(f"cannot read the statistical test '{test}'")
            parts.append(re.escape(f"working_directory={match['folder']}, ini_file={match['ini'] or 'omnetpp.ini'}, "
                                   f"config={match['config'] or 'General'})"))
        return "-f", "|".join(sorted(set(parts)))
    folders = set()
    for test in tests:
        match = CHART_TEST.match(test)
        if match is None:
            raise ValueError(f"cannot read the chart test '{test}'")
        folders.add(os.path.dirname(match["analysis"]))
    return "-w", "^(" + "|".join(re.escape(folder) for folder in sorted(folders)) + ")$"


OPP_REPL_VERSION = """import opp_repl
try:
    from opp_repl._version import version
except ImportError:
    version = "?"
print(version, opp_repl.__file__)"""


def environment(kind, folder):
    """The versions that recorded the candidates."""
    def run(command):
        out = subprocess.run(command, shell=True, capture_output=True, text=True)
        return (out.stdout.strip().splitlines() or ["?"])[0]
    commands = [
        ("inet", "git rev-parse HEAD"),
        (f"{kind} baseline checkout", "git -C " + shlex.quote(folder) + " rev-parse HEAD"),
        ("omnetpp", 'git -C "$__omnetpp_root_dir" rev-parse HEAD'),
        ("omnetpp version", "opp_run -v 2>&1 | grep -m1 Version"),
        ("opp_repl", "python3 -c " + shlex.quote(OPP_REPL_VERSION)),
        ("compiler", "${CXX:-clang++} --version"),
        ("python packages", "python3 -m pip freeze 2>/dev/null | grep -E '^(matplotlib|numpy|pandas|scipy)==' | tr '\\n' ' '"),
        ("system", '. /etc/os-release && echo "$PRETTY_NAME"'),
    ]
    return "".join(f"{name}: {run(command)}\n" for name, command in commands)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("kind", choices=["statistical", "chart"])
    parser.add_argument("result_file")
    parser.add_argument("output_folder")
    parser.add_argument("--update", required=True, metavar="COMMAND")
    args = parser.parse_args(argv)
    try:
        with open(OPP_FILE, encoding="utf-8") as f:
            entry = parse_baselines(f.read())[args.kind]
        folder = entry.get("folder") or "."
        with open(args.result_file, encoding="utf-8") as f:
            tests = selected_tests(json.load(f).get("results", []))
        if not tests:
            print(f"No {args.kind} test needs a candidate.")
            return 0
        option, value = test_filter(args.kind, tests)
        print(f"{len(tests)} {args.kind} test(s) need a candidate; the update runs for them.", flush=True)
        # the update of opp_repl ends with the status 1 when it changes a file, because it expects no change
        status = subprocess.run(f"{args.update} {option} {shlex.quote(value)}", shell=True).returncode
        if status not in (0, 1):
            print(f"warning: the update command stopped with the exit status {status}")
        # the work files of the chart test and of the update are not candidates
        for path in git(folder, "ls-files", "--others", "--exclude-standard", "-z").split("\0"):
            if path and WORK_FILES.search(path):
                os.remove(os.path.join(folder, path))
        git(folder, "add", "-A")
        is_result = baseline_summary.result_filter_of_folder(folder)
        # only result files are candidates; a work file of a test is not, also when the repository does not ignore it
        changes = [(letter, path) for letter, path in baseline_summary.changed_files(folder) if is_result(path)]
        os.makedirs(args.output_folder, exist_ok=True)
        removed = []
        for status_letter, path in changes:
            if status_letter == "D":
                removed.append(path)
                continue
            target = os.path.join(args.output_folder, path)
            os.makedirs(os.path.dirname(target), exist_ok=True)
            shutil.copyfile(os.path.join(folder, path), target)
        block = "\n".join(baseline_summary.block(baseline_summary.analyze(folder, is_result=is_result))) + "\n"
        with open(os.path.join(args.output_folder, "moved-results.txt"), "w") as f:
            f.write(block)
        with open(os.path.join(args.output_folder, "removed.txt"), "w") as f:
            f.write("".join(path + "\n" for path in removed))
        with open(os.path.join(args.output_folder, "environment.txt"), "w") as f:
            f.write(environment(args.kind, folder))
    except (OSError, KeyError, ValueError, subprocess.CalledProcessError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 2
    print(f"{len(changes) - len(removed)} candidate file(s) and {len(removed)} removed file(s) are in {args.output_folder}.")
    if args.kind == "statistical" and len(changes) < len(tests):
        # each selected statistical test changes or adds its one result file
        print(f"warning: {len(tests)} test(s) need a candidate, and the update gave {len(changes)} file(s)")
    print(block, end="")
    return 0


if __name__ == "__main__":
    sys.exit(main())
