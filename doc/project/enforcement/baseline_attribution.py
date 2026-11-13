#!/usr/bin/env python3
"""The attribution run: the run that proves the Attribution of each baseline commit of a series
(TR-BASELINE-ATTRIBUTION), and that the moved tests pass on the parent commit (TR-BASELINE-PARENT).

check-baselines.sh checks the record of a baseline commit without a run. This tool does the runs
that the forms claim. For each commit of the series that moves a pin over a result, it takes the
moved tests from the baseline commit and runs them in up to three trees:

  parent      the parent of the cause commit, against the old pins: the tests must pass
  commit      the cause commit, against the new pins: the tests must pass
  reduction   forward: the parent with the patch, against the new pins: the tests must pass
              reverse: the cause commit with the patch, against the old pins: the tests must pass

A test that the cause commit adds runs only in the commit tree, and a test that it removes only in
the parent tree. The forms first-record and environment, and a cause that INET-Commit names, have
only the commit run; the form removal has only the parent run.

The tool changes the work tree: it checks out each tree in turn, and it builds and runs content of
the series. Use it in a checkout that is only for this purpose, with no local change: a CI job, or
a copy of the CI job. It puts the first HEAD back at the end. It is not a part of the trusted
pull-request audit. Run check-baselines.sh first; this tool expects a series that passes it.

Usage, from the INET repository root:
  check-baseline-attribution.sh plan BASE..HEAD [--baseline-repository KIND=PATH]...
  check-baseline-attribution.sh run BASE..HEAD [--baseline-repository KIND=PATH]... [--log-folder FOLDER]
        [--build COMMAND] [--checkout COMMAND] [--statistical COMMAND] [--chart COMMAND]

plan prints the runs and reads git objects only. run does them. Each COMMAND is a shell command for
this checkout; the tool adds the filter of the moved tests to the two test commands. The defaults
are the commands of the CI workflows.

Exit status 0 = each run passes (or plan is done), 1 = a run fails, 2 = invalid usage or the tool cannot run.
"""
import argparse
import os
import re
import shlex
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import baseline_summary  # noqa: E402
from baseline_pins import OPP_FILE, PinError, git, is_full_hash, read_baselines  # noqa: E402
from check_baselines import FORMS, REDUCTION_FORMS, BaselineRepository, short, subject_of  # noqa: E402

DEFAULTS = {
    "build": "make MODE=release -j$(nproc)",
    "checkout": "opp_checkout_test_baselines",
    "statistical": "opp_run_statistical_tests -m release --no-build",
    "chart": "opp_run_chart_tests -m release --no-build",
}
RESULT_FILE = re.compile(r"(?P<folder>.+)/(?P<ini>[^/]+?\.ini)-(?P<config>[^/]+)-#(?P<run>\d+)\.sca$")
# the last line of a test command of opp_repl, for example "Tested statistical test (141 concurrent): 141 PASS in 35.8"
TEST_SUMMARY = re.compile(r"Tested [a-z ]+ \((\d+) concurrent\)")
ANSI_CODE = re.compile(r"\x1b\[[0-9;]*m")


class AttributionError(Exception):
    """The tool cannot plan or do the runs."""


class Step:
    """One commit of the series that moves a pin over a result, with its moved tests."""

    def __init__(self, commit, parent):
        self.commit = commit
        self.parent = parent
        self.form = None
        self.reduction = None       # the text of the reduction patch
        self.cause_on_target = False
        self.tests = []             # (status, working directory, ini file, config) of the statistical tests
        self.charts = []            # (status, folder of the analysis file) of the charts

    def runs(self):
        """[(name, tree commit, commit whose pins the run uses, with the patch, statuses of the tests)]"""
        parent = ("parent", self.parent, self.parent, False, "MD")
        commit = ("commit", self.commit, self.commit, False, "MA")
        if self.form in ("first-record", "environment") or self.cause_on_target:
            return [commit]
        if self.form == "removal":
            return [parent]
        runs = [parent, commit]
        if self.form == "forward":
            runs.append(("forward reduction", self.parent, self.commit, True, "M"))
        if self.form == "reverse":
            runs.append(("reverse reduction", self.commit, self.parent, True, "M"))
        return runs

    def count(self, kind, statuses):
        """The number of moved tests of a kind with one of the statuses: the least number that a run must select."""
        items = self.tests if kind == "statistical" else self.charts
        return len({item for item in items if item[0] in statuses}) if kind == "statistical" else sum(1 for item in items if item[0] in statuses)

    def statistical_filter(self, statuses):
        """The regular expression that selects the statistical tests with one of the statuses, or None."""
        tests = sorted({test[1:] for test in self.tests if test[0] in statuses})
        return "|".join(re.escape(f"working_directory={folder}, ini_file={ini}, config={config})")
                        for folder, ini, config in tests) or None

    def chart_filter(self, statuses):
        """The regular expression that selects the folders of the charts with one of the statuses, or None."""
        folders = sorted({folder for status, folder in self.charts if status in statuses})
        return "^(" + "|".join(re.escape(folder) for folder in folders) + ")$" if folders else None


def find_steps(commit_range, sources, cache_folder):
    """The steps of a series, in order. The series must pass check-baselines.sh."""
    base = git("rev-parse", commit_range.split("..")[0] or "HEAD").stdout.strip()
    commits = git("log", "--format=%H", "--reverse", "--first-parent", commit_range).stdout.split()
    repositories, steps = {}, []
    previous, parent = read_baselines(base), base
    for commit in commits:
        current = read_baselines(commit)
        step = Step(commit, parent)
        for kind in sorted(current):
            old, new = (previous.get(kind) or {}).get("ref"), current[kind].get("ref")
            if not (is_full_hash(old) and is_full_hash(new)) or old == new:
                continue
            if kind not in repositories:
                source = sources.get(kind) or (previous.get(kind) or current[kind])["repository"]
                if not os.path.isdir(source) and "://" not in source and not source.startswith("git@"):
                    source = f"https://github.com/{source}.git"
                repositories[kind] = BaselineRepository(kind, source, cache_folder)
            repository = repositories[kind]
            if repository.get([old, new]) or not repository.is_ancestor(old, new):
                raise AttributionError(f"{short(commit)} moves the {kind} pin to a commit that is not in the baseline repository, or not forward; run check-baselines.sh first")
            folders = {}
            if kind == "chart":
                folders = {**baseline_summary.chart_image_folders_at(parent, current[kind]),
                           **baseline_summary.chart_image_folders_at(commit, current[kind])}
            is_result = baseline_summary.result_filter(kind, folders)
            with_results = [c for c in repository.chain(old, new) if any(is_result(path) for path in repository.changed_paths(c))]
            if len(with_results) > 1:
                raise AttributionError(f"{short(commit)} moves the {kind} pin over {len(with_results)} commits that change results; run check-baselines.sh first")
            if not with_results:
                continue
            baseline_commit = with_results[0]
            trailers = repository.trailers(baseline_commit)
            fields = (trailers.get("Attribution") or [""])[0].split()
            if not fields or fields[0] not in FORMS or (step.form is not None and step.form != fields[0]):
                raise AttributionError(f"{short(commit)} -> {kind} {short(baseline_commit)} has no usable Attribution; run check-baselines.sh first")
            step.form = fields[0]
            step.cause_on_target = step.cause_on_target or bool(trailers.get("INET-Commit"))
            if step.form in REDUCTION_FORMS:
                if len(fields) != 2 or repository.blob(baseline_commit, fields[1]) is None:
                    raise AttributionError(f"{short(commit)} -> {kind} {short(baseline_commit)} names no reduction file that exists; run check-baselines.sh first")
                step.reduction = baseline_summary.read_file(repository.folder, baseline_commit, fields[1])
            for status, path in baseline_summary.changed_files(repository.folder, repository.parents(baseline_commit)[0], baseline_commit):
                if not is_result(path):
                    continue
                if kind == "chart":
                    step.charts.append((status, folders[path]))
                else:
                    match = RESULT_FILE.match(path)
                    if match is None:
                        raise AttributionError(f"{path} of {kind} {short(baseline_commit)} does not have the name of a stored result")
                    step.tests.append((status, match["folder"], match["ini"], match["config"]))
        if step.form is not None:
            steps.append(step)
        previous, parent = current, commit
    return steps


def print_plan(steps):
    for step in steps:
        print(f"{short(step.commit)} {subject_of(step.commit)}")
        print(f"  form {step.form}" + (", the cause is on the target branch" if step.cause_on_target else "")
              + f"; {len(step.tests)} statistical test(s), charts in {len({folder for _, folder in step.charts})} folder(s)")
        for name, tree, pins, patched, statuses in step.runs():
            statistical = sum(1 for test in step.tests if test[0] in statuses)
            folders = len({folder for status, folder in step.charts if status in statuses})
            print(f"  run {name}: the tree {short(tree)}" + (" with the patch" if patched else "")
                  + f", the pins of {short(pins)}: {statistical} statistical test(s), charts of {folders} folder(s)")
    if not steps:
        print("No commit of the series moves a pin over a result. No run is necessary.")


def shell(command, log=None):
    """Runs a shell command in the work tree. Returns the exit status; the output goes to the log file."""
    if log is None:
        return subprocess.run(command, shell=True).returncode
    with open(log, "w") as f:
        return subprocess.run(command, shell=True, stdout=f, stderr=subprocess.STDOUT).returncode


def tests_in_log(log):
    """The number of tests that a test command ran, from the summary line of its log; None when the log has no summary."""
    with open(log, encoding="utf-8", errors="replace") as f:
        found = TEST_SUMMARY.findall(ANSI_CODE.sub("", f.read()))
    return int(found[-1]) if found else None


def patched_paths(patch):
    """The paths that a patch changes, and the paths that it makes."""
    numstat = subprocess.run(["git", "apply", "--numstat", patch], capture_output=True, text=True, check=True).stdout
    summary = subprocess.run(["git", "apply", "--summary", patch], capture_output=True, text=True, check=True).stdout
    made = {line.split()[-1] for line in summary.splitlines() if line.strip().startswith("create mode")}
    return [line.split("\t")[2] for line in numstat.splitlines() if line.count("\t") == 2], made


def do_run(step, run, commands, log_folder, number):
    """Does one run. Returns (passed, text for the report)."""
    name, tree, pins, patched, statuses = run
    statistical, chart = step.statistical_filter(statuses), step.chart_filter(statuses)
    if statistical is None and chart is None:
        return True, "no test"
    prefix = os.path.join(log_folder, f"{number:02d}-{short(step.commit)}-{name.replace(' ', '-')}")
    git("checkout", "-q", "--detach", tree)
    restore = []
    try:
        if patched:
            patch = prefix + ".patch"
            with open(patch, "w", encoding="utf-8") as f:
                f.write(step.reduction)
            changed, made = patched_paths(patch)
            restore = [OPP_FILE] + [path for path in changed if path not in made]
            # the pins of the other commit: the patched tree gives the results of that commit
            with open(OPP_FILE, "w", encoding="utf-8") as f:
                f.write(git("show", f"{pins}:{OPP_FILE}").stdout)
            if subprocess.run(["git", "apply", patch]).returncode != 0:
                return False, "the reduction does not apply"
        if shell(commands["build"], prefix + "-build.log") != 0:
            return False, f"the build fails, see {prefix}-build.log"
        if shell(commands["checkout"], prefix + "-checkout.log") != 0:
            return False, f"the checkout of the baselines fails, see {prefix}-checkout.log"
        failed, passed = [], []
        for kind, value, option in (("statistical", statistical, "-f"), ("chart", chart, "-w")):
            if value is None:
                continue
            log = f"{prefix}-{kind}.log"
            status = shell(f"{commands[kind]} {option} {shlex.quote(value)}", log)
            ran, moved = tests_in_log(log), step.count(kind, statuses)
            if status != 0:
                failed.append(f"the {kind} tests fail, see {log}")
            elif ran is None or ran < moved or (kind == "statistical" and ran != moved):
                # a run that selects no test, or not the moved tests, proves nothing
                failed.append(f"the {kind} run selected {ran if ran is not None else 'an unknown number of'} test(s), and the baseline commit moves {moved}, see {log}")
            else:
                passed.append(f"{ran} {kind} test(s) pass")
        return not failed, "; ".join(failed or passed)
    finally:
        if patched:
            for path in made if restore else ():
                if os.path.exists(path):
                    os.remove(path)
            if restore:
                git("checkout", "-q", tree, "--", *restore, check=False)


def do_runs(steps, commands, log_folder):
    if git("status", "--porcelain", "--untracked-files=no").stdout.strip():
        raise AttributionError("the work tree has changes; the attribution run needs a checkout with no local change")
    first = git("symbolic-ref", "-q", "--short", "HEAD", check=False).stdout.strip() or git("rev-parse", "HEAD").stdout.strip()
    os.makedirs(log_folder, exist_ok=True)
    failures, number = 0, 0
    try:
        for step in steps:
            print(f"{short(step.commit)} {subject_of(step.commit)}: form {step.form}")
            for run in step.runs():
                number += 1
                passed, text = do_run(step, run, commands, log_folder, number)
                print(f"  {'PASS' if passed else 'FAIL'} run {run[0]}: the tree {short(run[1])}" + (" with the patch" if run[3] else "")
                      + f", the pins of {short(run[2])}: {text}", flush=True)
                failures += not passed
    finally:
        git("checkout", "-q", first, check=False)
    return failures


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("action", choices=["plan", "run"])
    parser.add_argument("range", metavar="BASE..HEAD")
    parser.add_argument("--baseline-repository", action="append", default=[], metavar="KIND=PATH",
                        help="read the baseline commits of a test kind from a local clone, or from another URL")
    parser.add_argument("--log-folder", help="the folder for the logs of the runs (default: a new temporary folder)")
    for name, command in DEFAULTS.items():
        parser.add_argument(f"--{name}", default=command, metavar="COMMAND", help=f"default: {command}")
    args = parser.parse_args(argv)
    if ".." not in args.range or "..." in args.range or git("rev-parse", args.range, check=False).returncode != 0:
        print(f"error: '{args.range}' is not a valid commit range", file=sys.stderr)
        return 2
    sources = dict(value.split("=", 1) for value in args.baseline_repository if "=" in value)
    if len(sources) != len(args.baseline_repository):
        print("error: --baseline-repository needs KIND=PATH", file=sys.stderr)
        return 2
    with tempfile.TemporaryDirectory() as cache_folder:
        try:
            steps = find_steps(args.range, sources, cache_folder)
            if args.action == "plan" or not steps:
                print_plan(steps)
                return 0
            log_folder = args.log_folder or tempfile.mkdtemp(prefix="baseline-attribution-")
            failures = do_runs(steps, {name: getattr(args, name) for name in DEFAULTS}, log_folder)
        except (AttributionError, PinError, RuntimeError, subprocess.CalledProcessError) as error:
            print(f"error: {error}", file=sys.stderr)
            return 2
    print()
    if failures:
        print(f"FAIL: {failures} run(s) do not prove the attribution. The logs are in {log_folder}.")
        return 1
    print(f"PASS: each run proves its attribution. The logs are in {log_folder}.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
