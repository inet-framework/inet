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

A commit that changes fingerprint values in tests/fingerprint/store.json is a step too. Each run
tests the runs whose fingerprints change, against the store of the commit that gives the
expectations: the parent, the commit, or for a reduction the other commit, as for the pins. The
form and the reduction come from the Attribution trailer and the patch after "Reduction:" in the
message of the commit, or else from its pinned baseline commits; with neither, the form is commit.

The tool changes the work tree: it checks out each tree in turn, and it builds and runs content of
the series. Use it in a checkout that is only for this purpose, with no local change: a CI job, or
a copy of the CI job. It puts the first HEAD back at the end. It is not a part of the trusted
pull-request audit. Run check-baselines.sh first; this tool expects a series that passes it.

Usage, from the INET repository root:
  check-baseline-attribution.sh plan BASE..HEAD [--baseline-repository KIND=PATH]...
  check-baseline-attribution.sh run BASE..HEAD [--baseline-repository KIND=PATH]... [--log-folder FOLDER]
        [--build COMMAND] [--checkout COMMAND] [--statistical COMMAND] [--chart COMMAND] [--fingerprint COMMAND]

plan prints the runs and reads git objects only. run does them. Each COMMAND is a shell command for
this checkout; the tool adds the filter of the moved tests to the three test commands, and the
option --commit with the commit that gives the expectations to the checkout command. The defaults
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
from baseline_pins import OPP_FILE, PinError, git, read_baselines, series_pin_moves  # noqa: E402
from check_baselines import (FINGERPRINT_FORMS, FORMS, REDUCTION_FORMS, STORE, BaselineRepository, message_of,  # noqa: E402
                             reduction_in_message, short, store_at, subject_of, trailers_of)

DEFAULTS = {
    "build": "make MODE=release -j$(nproc)",
    "checkout": "opp_checkout_test_baselines",
    "statistical": "opp_run_statistical_tests -m release --no-build",
    "chart": "opp_run_chart_tests -m release --no-build",
    "fingerprint": "opp_run_fingerprint_tests -m release --no-build",
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
        self.fingerprints = []      # (working directory, ini file, config, run number) of the runs whose fingerprints change

    def runs(self):
        """[(name, tree commit, commit whose expectations the run uses, with the patch, statuses of the tests)]
        The expectations are the pins in inet.opp and the fingerprint store."""
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
        if kind == "fingerprint":
            return len(self.fingerprints)
        items = self.tests if kind == "statistical" else self.charts
        return len({item for item in items if item[0] in statuses}) if kind == "statistical" else sum(1 for item in items if item[0] in statuses)

    def statistical_filter(self, statuses):
        """The regular expression that selects the statistical tests with one of the statuses, or None."""
        tests = sorted({test[1:] for test in self.tests if test[0] in statuses})
        return "|".join(re.escape(f"working_directory={folder}, ini_file={ini}, config={config})")
                        for folder, ini, config in tests) or None

    def fingerprint_filter(self):
        """The regular expression that selects the configs of the runs whose fingerprints change, or None.
        It selects each run of these configs in the store, so a run selects at least the moved runs."""
        configs = sorted({run[:3] for run in self.fingerprints})
        return "|".join(re.escape(f"working_directory={folder}, ini_file={ini}, config={config})")
                        for folder, ini, config in configs) or None

    def chart_filter(self, statuses):
        """The regular expression that selects the folders of the charts with one of the statuses, or None."""
        folders = sorted({folder for status, folder in self.charts if status in statuses})
        return "^(" + "|".join(re.escape(folder) for folder in folders) + ")$" if folders else None


def find_steps(commit_range, sources, cache_folder):
    """The steps of a series, in order. The series must pass check-baselines.sh."""
    base = git("rev-parse", commit_range.split("..")[0] or "HEAD").stdout.strip()
    commits = git("log", "--format=%H", "--reverse", "--first-parent", commit_range).stdout.split()
    repositories, steps = {}, []
    moves = series_pin_moves(base, commits)
    parent = base
    for commit in commits:
        current = read_baselines(commit)
        step = Step(commit, parent)
        for _, _, kind, old, new in (move for move in moves if move[0] == commit):
            if kind not in repositories:
                source = sources.get(kind) or current[kind]["repository"]
                if not os.path.isdir(source) and "://" not in source and not source.startswith("git@"):
                    source = f"https://github.com/{source}.git"
                repositories[kind] = BaselineRepository(kind, source, cache_folder)
            repository = repositories[kind]
            if old is None:
                # the trailer form: the first pin moves from the base of its baseline branch
                old = None if repository.get([new]) else repository.branch_base(new)
                if old is None:
                    raise AttributionError(f"{short(commit)} pins {kind} {short(new)}, which is not in the baseline repository or not on its master; run check-baselines.sh first")
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
        add_fingerprints(step)
        if step.form is not None:
            steps.append(step)
        parent = commit
    return steps


def add_fingerprints(step):
    """Adds the runs of a step whose fingerprints change, and the attribution of the fingerprint change:
    the Attribution trailer and the patch after "Reduction:" of the INET commit, or else the form of its
    pinned baseline commits, or else the form commit."""
    old, new = store_at(f"{step.commit}^"), store_at(step.commit)
    runs = sorted(run for run in old.keys() & new.keys() if any(new[run].get(i, (None,))[0] not in (None, values[0]) for i, values in old[run].items()))
    if not runs:
        return
    trailers, trailer_lines = trailers_of(step.commit)
    values = trailers.get("Attribution", [])
    if values:
        fields = values[0].split()
        if len(values) != 1 or len(fields) != 1 or fields[0] not in FINGERPRINT_FORMS or (step.form is not None and step.form != fields[0]):
            raise AttributionError(f"{short(step.commit)} has no usable Attribution for its fingerprint change; run check-baselines.sh first")
        step.form = fields[0]
    elif step.form is None:
        step.form = "commit"
    if step.form in REDUCTION_FORMS and step.reduction is None:
        step.reduction = reduction_in_message(message_of(step.commit), trailer_lines)
        if step.reduction is None:
            raise AttributionError(f"{short(step.commit)} has the Attribution {step.form} and no patch after a line 'Reduction:'; run check-baselines.sh first")
    step.fingerprints = runs


def print_plan(steps):
    for step in steps:
        print(f"{short(step.commit)} {subject_of(step.commit)}")
        print(f"  form {step.form}" + (", the cause is on the target branch" if step.cause_on_target else "")
              + f"; {len(step.tests)} statistical test(s), charts in {len({folder for _, folder in step.charts})} folder(s)"
              + f", {len(step.fingerprints)} fingerprint run(s)")
        for name, tree, expectations, patched, statuses in step.runs():
            statistical = sum(1 for test in step.tests if test[0] in statuses)
            folders = len({folder for status, folder in step.charts if status in statuses})
            print(f"  run {name}: the tree {short(tree)}" + (" with the patch" if patched else "")
                  + f", the expectations of {short(expectations)}: {statistical} statistical test(s), charts of {folders} folder(s)"
                  + f", {len(step.fingerprints)} fingerprint run(s)")
    if not steps:
        print("No commit of the series moves a pin over a result or a fingerprint value. No run is necessary.")


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
    # --recount: a patch from a commit message can lose an empty context line at its end
    numstat = subprocess.run(["git", "apply", "--numstat", "--recount", patch], capture_output=True, text=True, check=True).stdout
    summary = subprocess.run(["git", "apply", "--summary", "--recount", patch], capture_output=True, text=True, check=True).stdout
    made = {line.split()[-1] for line in summary.splitlines() if line.strip().startswith("create mode")}
    return [line.split("\t")[2] for line in numstat.splitlines() if line.count("\t") == 2], made


def do_run(step, run, commands, log_folder, number):
    """Does one run. Returns (passed, text for the report)."""
    name, tree, expectations, patched, statuses = run
    statistical, chart, fingerprint = step.statistical_filter(statuses), step.chart_filter(statuses), step.fingerprint_filter()
    if statistical is None and chart is None and fingerprint is None:
        return True, "no test"
    prefix = os.path.join(log_folder, f"{number:02d}-{short(step.commit)}-{name.replace(' ', '-')}")
    git("checkout", "-q", "--detach", tree)
    restore, made, folders = [], set(), []
    try:
        if patched:
            patch = prefix + ".patch"
            with open(patch, "w", encoding="utf-8") as f:
                f.write(step.reduction)
            changed, created = patched_paths(patch)
            restore = [path for path in changed if path not in created]
            made |= set(created)
            # the pins and the fingerprints of the other commit: the patched tree gives the results of that commit
            for path in (OPP_FILE, STORE):
                text = git("show", f"{expectations}:{path}", check=False)
                if text.returncode == 0:
                    (restore.append(path) if git("cat-file", "-e", f"{tree}:{path}", check=False).returncode == 0 else made.add(path))
                    if os.path.dirname(path) and not os.path.isdir(os.path.dirname(path)):
                        os.makedirs(os.path.dirname(path))
                        folders.append(os.path.dirname(path))
                    with open(path, "w", encoding="utf-8") as f:
                        f.write(text.stdout)
            if subprocess.run(["git", "apply", "--recount", patch]).returncode != 0:
                return False, "the reduction does not apply"
        if shell(commands["build"], prefix + "-build.log") != 0:
            return False, f"the build fails, see {prefix}-build.log"
        # the baselines of the commit that gives the expectations: for the trailer form, its history gives them
        if (statistical or chart) and shell(f"{commands['checkout']} --commit {expectations}", prefix + "-checkout.log") != 0:
            return False, f"the checkout of the baselines fails, see {prefix}-checkout.log"
        failed, passed = [], []
        for kind, value, option in (("statistical", statistical, "-f"), ("chart", chart, "-w"), ("fingerprint", fingerprint, "-f")):
            if value is None:
                continue
            log = f"{prefix}-{kind}.log"
            status = shell(f"{commands[kind]} {option} {shlex.quote(value)}", log)
            ran, moved = tests_in_log(log), step.count(kind, statuses)
            if status != 0:
                failed.append(f"the {kind} tests fail, see {log}")
            elif ran is None or ran < moved or (kind == "statistical" and ran != moved):
                # a run that selects no test, or not the moved tests, proves nothing
                failed.append(f"the {kind} run selected {ran if ran is not None else 'an unknown number of'} test(s), and the step moves {moved}, see {log}")
            else:
                passed.append(f"{ran} {kind} test(s) pass")
        return not failed, "; ".join(failed or passed)
    finally:
        for path in made:
            if os.path.exists(path):
                os.remove(path)
        for folder in folders:
            try:
                os.removedirs(folder)
            except OSError:
                pass
        # one path at a time: a path that the tree does not hold must not stop the others
        for path in restore:
            git("checkout", "-q", tree, "--", path, check=False)


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
                      + f", the expectations of {short(run[2])}: {text}", flush=True)
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
