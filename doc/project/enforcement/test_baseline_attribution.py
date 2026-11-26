#!/usr/bin/env python3
"""Tests of the attribution run (check-baseline-attribution.sh and baseline_attribution.py).

The tests reuse the three temporary repositories of GateTestCase. The test commands are a fake shell
script that records each call and prints the summary line of a test command, so no build or
simulation runs. The control files in the folder "control" set the exit status and the number of
tests that the fake script reports: "<action>.status" and "<action>.count", or the same name with
".<hash of HEAD>" to set the value for the run in one tree. A file "<action>.nosummary" leaves the
summary line out.
"""

from pathlib import Path
import json
import os
import re
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_check_baselines import (GateTestCase, SCA_A, SCA_B, PNG_A, PNG_DOC, PATCH, FORWARD_PATCH,  # noqa: E402
                                  REVERSE_PATCH, BAD_PATCH, sca)
import baseline_attribution as ba  # noqa: E402
import test_check_baselines  # noqa: E402

WRAPPER = Path(__file__).with_name("check-baseline-attribution.sh").resolve()
SCA_C = "examples/c/omnetpp.ini-General-#0.sca"
NEW_PNG = b"\x89PNG new"
REPR_A = "working_directory=examples/a, ini_file=omnetpp.ini, config=General)"

FAKE = r"""action=$1; shift
head=$(git rev-parse HEAD)
refs=$(grep -o '[0-9a-f]\{40\}' inet.opp | tr '\n' ' ')
source=$(tr '\n' ';' < src/inet/a/A.cc)
store=none
[ -f tests/fingerprint/store.json ] && store=$(grep -o '"fingerprint": "[^"]*"' tests/fingerprint/store.json | cut -d'"' -f4 | tr '\n' ',')
echo "$action|$head|$refs|$source|$store|$*" >> "$RECORD"
value() {
  if [ -f "$CONTROL/$action.$1.$head" ]; then cat "$CONTROL/$action.$1.$head"
  elif [ -f "$CONTROL/$action.$1" ]; then cat "$CONTROL/$action.$1"
  else echo "$2"; fi
}
case $action in
  statistical|chart|fingerprint)
    if [ ! -f "$CONTROL/$action.nosummary" ]; then
      n=$(value count $DEFAULT)
      echo "Tested $action test ($n concurrent): $n PASS in 1.0"
    fi ;;
esac
exit $(value status 0)
"""


class AttributionTestCase(GateTestCase):
    def setUp(self):
        super().setUp()
        root = Path(self.tempdir.name)
        self.control = root / "control"
        self.control.mkdir()
        self.record_file = root / "record.txt"
        self.logs = root / "logs"
        self.script = root / "fake.sh"
        self.script.write_text(f'RECORD={self.record_file}\nCONTROL={self.control}\n'
                               'case $1 in statistical) DEFAULT=1;; *) DEFAULT=9;; esac\n' + FAKE)
        self.master = self.head(self.inet)

    def control_file(self, name, value="1"):
        (self.control / name).write_text(value)

    def tool(self, action, series="master..topic", *extra, directory=None):
        commands = []
        for option in ("build", "checkout", "statistical", "chart", "fingerprint"):
            commands += [f"--{option}", f"sh {self.script} {option}"]
        return subprocess.run(["bash", str(WRAPPER), action, series,
                               "--baseline-repository", f"statistical={self.stat}",
                               "--baseline-repository", f"chart={self.media}",
                               "--log-folder", str(self.logs), *commands, *extra],
                              cwd=directory or self.inet, capture_output=True, text=True)

    def calls(self):
        """The calls of the fake script: dictionaries with action, head, refs, source, store and args."""
        if not self.record_file.exists():
            return []
        keys = ("action", "head", "refs", "source", "store", "args")
        return [dict(zip(keys, line.split("|", 5))) for line in self.record_file.read_text().splitlines()]

    def commits(self):
        """The hashes of the INET commits of the series, oldest first."""
        return self.git(self.inet, "log", "--format=%H", "--reverse", "master..topic").stdout.split()

    def parent_of(self, commit):
        return self.git(self.inet, "rev-parse", commit + "^").stdout.strip()

    def refs_of(self, commit):
        return " ".join(re.findall(r"[0-9a-f]{40}", self.git(self.inet, "show", f"{commit}:inet.opp").stdout)) + " "

    def status(self):
        return self.git(self.inet, "status", "--porcelain", "--untracked-files=no").stdout

    def branch(self):
        return self.git(self.inet, "symbolic-ref", "-q", "--short", "HEAD").stdout.strip()

    def move_commit_form(self, **kwargs):
        self.move(stat={SCA_A: sca(sent="11")}, **kwargs)

    def move_reverse(self):
        self.move(stat={SCA_A: sca(sent="11"), PATCH: REVERSE_PATCH}, stat_record={"attribution": f"reverse {PATCH}"})

    def move_forward(self):
        self.move(stat={SCA_A: sca(sent="11"), PATCH: FORWARD_PATCH}, stat_record={"attribution": f"forward {PATCH}"})


class PlanTest(AttributionTestCase):
    def plan(self):
        result = self.tool("plan")
        self.assertEqual(result.returncode, 0, result.stderr)
        return result.stdout

    def test_series_without_a_pin_move_needs_no_run(self):
        self.inet_commit(source=1)
        self.assertIn("No run is necessary", self.plan())

    def test_commit_form_lists_the_parent_run_and_the_commit_run_with_one_test_each(self):
        self.move_commit_form()
        out = self.plan()
        self.assertIn("form commit; 1 statistical test(s), charts in 0 folder(s)", out)
        self.assertRegex(out, r"run parent: .*: 1 statistical test\(s\), charts of 0 folder\(s\)")
        self.assertRegex(out, r"run commit: .*: 1 statistical test\(s\), charts of 0 folder\(s\)")
        self.assertEqual(out.count("  run "), 2)

    def test_forward_form_lists_a_third_run_with_the_patch(self):
        self.move_forward()
        out = self.plan()
        self.assertEqual(out.count("  run "), 3)
        self.assertRegex(out, r"run forward reduction: the tree \w+ with the patch")

    def test_reverse_form_lists_a_third_run_with_the_patch(self):
        self.move_reverse()
        out = self.plan()
        self.assertEqual(out.count("  run "), 3)
        self.assertRegex(out, r"run reverse reduction: the tree \w+ with the patch")

    def test_first_record_lists_only_the_commit_run(self):
        self.move(stat={SCA_C: sca()}, stat_record={"attribution": "first-record"})
        out = self.plan()
        self.assertIn("run commit:", out)
        self.assertEqual(out.count("  run "), 1)

    def test_removal_lists_only_the_parent_run(self):
        self.move(stat={SCA_B: None}, stat_record={"attribution": "removal"})
        out = self.plan()
        self.assertIn("run parent:", out)
        self.assertEqual(out.count("  run "), 1)

    def test_environment_lists_only_the_commit_run(self):
        self.move(stat={SCA_A: sca(sent="11")}, source=0, stat_record={"attribution": "environment"})
        out = self.plan()
        self.assertIn("run commit:", out)
        self.assertEqual(out.count("  run "), 1)

    def test_a_cause_on_the_target_branch_lists_only_the_commit_run(self):
        self.move(stat={SCA_A: sca(sent="11")}, source=0,
                  stat_record={"inet_subject": None, "attribution": f"commit\nINET-Commit: {self.master}"})
        out = self.plan()
        self.assertIn("the cause is on the target branch", out)
        self.assertIn("run commit:", out)
        self.assertEqual(out.count("  run "), 1)

    def test_a_commit_that_moves_both_pins_lists_the_tests_and_the_chart_folder(self):
        self.move(stat={SCA_A: sca(sent="11")}, media={PNG_A: NEW_PNG})
        out = self.plan()
        self.assertIn("1 statistical test(s), charts in 1 folder(s)", out)
        self.assertRegex(out, r"run commit: .*: 1 statistical test\(s\), charts of 1 folder\(s\)")

    def test_a_chart_pin_move_over_a_documentation_commit_gives_no_step(self):
        doc = self.plain(self.media, {PNG_DOC: b"\x89PNG new documentation"})
        self.inet_commit(source=1, pins={"chart": doc})
        self.assertIn("No run is necessary", self.plan())

    def test_a_changed_and_an_added_test_give_one_test_in_the_parent_run_and_two_in_the_commit_run(self):
        self.move(stat={SCA_A: sca(sent="11"), SCA_C: sca()})
        out = self.plan()
        self.assertRegex(out, r"run parent: .*: 1 statistical test\(s\)")
        self.assertRegex(out, r"run commit: .*: 2 statistical test\(s\)")


class RunTest(AttributionTestCase):
    def run_ok(self, series="master..topic", *extra):
        result = self.tool("run", series, *extra)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        return result

    def test_commit_form_runs_the_parent_tree_and_then_the_commit_tree(self):
        self.move_commit_form()
        (commit,) = self.commits()
        parent = self.parent_of(commit)
        result = self.run_ok()
        self.assertIn("PASS run parent", result.stdout)
        self.assertIn("PASS run commit", result.stdout)
        self.assertEqual([(c["action"], c["head"]) for c in self.calls()],
                         [(a, h) for h in (parent, commit) for a in ("build", "checkout", "statistical")])
        self.assertEqual([c["refs"] for c in self.calls() if c["action"] == "build"],
                         [self.refs_of(parent), self.refs_of(commit)])

    def test_the_statistical_command_gets_a_filter_that_matches_the_moved_test(self):
        self.move_commit_form()
        self.run_ok()
        args = [c["args"] for c in self.calls() if c["action"] == "statistical"][0]
        self.assertTrue(args.startswith("-f "), args)
        self.assertTrue(re.search(args[3:], "working_directory=examples/a, ini_file=omnetpp.ini, config=General)"))

    def test_the_work_tree_is_back_on_the_branch_and_clean_after_the_run(self):
        self.move_commit_form()
        tip = self.head(self.inet)
        self.run_ok()
        self.assertEqual(self.branch(), "topic")
        self.assertEqual(self.head(self.inet), tip)
        self.assertEqual(self.status(), "")

    def test_the_reverse_reduction_run_uses_the_commit_with_the_patch_and_the_pins_of_the_parent(self):
        self.move_reverse()
        (commit,) = self.commits()
        parent = self.parent_of(commit)
        self.run_ok()
        third = [c for c in self.calls() if c["action"] == "statistical"][2]
        self.assertEqual(third["head"], commit)
        self.assertEqual(third["source"], "int a = 0;;")
        self.assertEqual(third["refs"], self.refs_of(parent))
        self.assertEqual(self.status(), "")
        self.assertEqual(self.git(self.inet, "diff", "topic").stdout, "")

    def test_the_forward_reduction_run_uses_the_parent_with_the_patch_and_the_pins_of_the_commit(self):
        self.move_forward()
        (commit,) = self.commits()
        parent = self.parent_of(commit)
        self.run_ok()
        calls = [c for c in self.calls() if c["action"] == "statistical"]
        self.assertEqual(calls[0]["source"], "int a = 0;;")
        third = calls[2]
        self.assertEqual(third["head"], parent)
        self.assertEqual(third["source"], "int a = 0;;int v1;;")
        self.assertEqual(third["refs"], self.refs_of(commit))
        self.assertEqual(self.status(), "")

    def test_a_reduction_that_does_not_apply_fails_and_leaves_the_work_tree_clean(self):
        self.move(stat={SCA_A: sca(sent="11"), PATCH: BAD_PATCH}, stat_record={"attribution": f"reverse {PATCH}"})
        result = self.tool("run")
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn("the reduction does not apply", result.stdout)
        self.assertEqual(self.status(), "")
        self.assertEqual(self.branch(), "topic")

    def test_both_pins_moved_run_the_chart_command_with_the_folder_of_the_chart(self):
        self.move(stat={SCA_A: sca(sent="11")}, media={PNG_A: NEW_PNG})
        self.run_ok()
        args = [c["args"] for c in self.calls() if c["action"] == "chart"]
        self.assertEqual(len(args), 2)
        for arg in args:
            self.assertTrue(arg.startswith("-w "), arg)
            self.assertTrue(re.search(arg[3:], "showcases/a"))
            self.assertFalse(re.search(arg[3:], "showcases/b"))

    def test_a_failing_test_command_fails_that_run_and_the_later_runs_still_happen(self):
        self.move_commit_form()
        (commit,) = self.commits()
        self.control_file(f"statistical.status.{self.parent_of(commit)}", "1")
        result = self.tool("run")
        self.assertEqual(result.returncode, 1)
        self.assertIn("FAIL run parent", result.stdout)
        self.assertIn("PASS run commit", result.stdout)
        self.assertEqual(self.branch(), "topic")

    def test_a_failing_build_fails_the_run(self):
        self.move_commit_form()
        self.control_file("build.status", "1")
        result = self.tool("run")
        self.assertEqual(result.returncode, 1)
        self.assertIn("the build fails", result.stdout)
        self.assertEqual([c["action"] for c in self.calls()], ["build", "build"])

    def test_a_failing_checkout_of_the_baselines_fails_the_run(self):
        self.move_commit_form()
        self.control_file("checkout.status", "1")
        result = self.tool("run")
        self.assertEqual(result.returncode, 1)
        self.assertIn("the checkout of the baselines fails", result.stdout)

    def test_a_statistical_run_of_zero_tests_fails(self):
        self.move_commit_form()
        self.control_file("statistical.count", "0")
        result = self.tool("run")
        self.assertEqual(result.returncode, 1)
        self.assertIn("selected 0 test(s)", result.stdout)

    def test_a_statistical_run_of_more_tests_than_moved_fails(self):
        self.move_commit_form()
        self.control_file("statistical.count", "2")
        result = self.tool("run")
        self.assertEqual(result.returncode, 1)
        self.assertIn("selected 2 test(s)", result.stdout)

    def test_a_statistical_log_without_a_summary_line_fails(self):
        self.move_commit_form()
        self.control_file("statistical.nosummary")
        result = self.tool("run")
        self.assertEqual(result.returncode, 1)
        self.assertIn("an unknown number of test(s)", result.stdout)

    def test_a_chart_run_that_ran_more_tests_than_the_moved_charts_passes(self):
        self.move(media={PNG_A: NEW_PNG})
        self.control_file("chart.count", "5")
        self.run_ok()

    def test_a_chart_run_that_ran_fewer_tests_than_the_moved_charts_fails(self):
        self.move(media={PNG_A: NEW_PNG})
        self.control_file("chart.count", "0")
        result = self.tool("run")
        self.assertEqual(result.returncode, 1)
        self.assertIn("the chart run selected 0 test(s)", result.stdout)

    def test_a_work_tree_with_a_change_in_a_tracked_file_exits_2_and_checks_out_nothing(self):
        self.move_commit_form()
        self.write(self.inet, "src/inet/a/A.cc", "int changed;\n")
        result = self.tool("run")
        self.assertEqual(result.returncode, 2)
        self.assertIn("the work tree has changes", result.stderr)
        self.assertEqual(self.calls(), [])
        self.assertEqual(self.branch(), "topic")
        self.assertEqual((self.inet / "src/inet/a/A.cc").read_text(), "int changed;\n")

    def test_the_log_folder_gets_one_log_for_each_command_of_each_run(self):
        self.move_commit_form()
        self.run_ok()
        logs = sorted(path.name for path in self.logs.glob("*.log"))
        self.assertEqual(len(logs), 6, logs)
        for kind in ("build", "checkout", "statistical"):
            self.assertEqual(len([name for name in logs if name.endswith(f"-{kind}.log")]), 2)
        self.assertIn("Tested statistical test (1 concurrent)", next(self.logs.glob("*-statistical.log")).read_text())


class TrailerAttributionTest(AttributionTestCase):
    """The trailer form: the step of a pin trailer starts on the base of its baseline branch, and the
    checkout of each run gets the commit whose expectations it uses."""

    def setUp(self):
        super().setUp()
        self.git(self.inet, "checkout", "-q", "master")
        self.write(self.inet, "inet.opp", test_check_baselines.trailer_opp_text())
        self.git(self.inet, "commit", "-qam", "switch to the trailer form")
        self.git(self.inet, "checkout", "-q", "-B", "topic", "master")
        for repository in (self.stat, self.media):
            self.git(repository, "checkout", "-q", "-b", "branch")

    def trailer_move(self, **kwargs):
        return test_check_baselines.TrailerTest.trailer_move(self, **kwargs)

    def test_the_plan_of_a_pin_trailer_lists_the_parent_run_and_the_commit_run(self):
        self.trailer_move(stat={SCA_A: sca(sent="11")})
        result = self.tool("plan")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("form commit; 1 statistical test(s), charts in 0 folder(s)", result.stdout)
        self.assertEqual(result.stdout.count("  run "), 2)

    def test_the_checkout_of_each_run_gets_the_commit_of_its_expectations(self):
        self.trailer_move(stat={SCA_A: sca(sent="11"), PATCH: FORWARD_PATCH}, stat_record={"attribution": f"forward {PATCH}"})
        (commit,) = self.commits()
        parent = self.parent_of(commit)
        result = self.tool("run")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        checkouts = [(c["head"], c["args"]) for c in self.calls() if c["action"] == "checkout"]
        # the forward reduction runs in the parent tree with the expectations of the commit
        self.assertEqual(checkouts, [(parent, f"--commit {parent}"), (commit, f"--commit {commit}"), (parent, f"--commit {commit}")])


class UsageTest(AttributionTestCase):
    def test_fewer_than_two_arguments_exit_2(self):
        for arguments in ([], ["plan"]):
            result = subprocess.run(["bash", str(WRAPPER), *arguments], cwd=self.inet, capture_output=True, text=True)
            self.assertEqual(result.returncode, 2)
            self.assertIn("usage:", result.stderr)

    def test_an_invalid_range_exits_2(self):
        for series in ("master..nothing", "master", "master...topic"):
            result = self.tool("plan", series)
            self.assertEqual(result.returncode, 2, series)
            self.assertIn("is not a valid commit range", result.stderr)


class StepTest(unittest.TestCase):
    @staticmethod
    def step(form, cause_on_target=False):
        step = ba.Step("c" * 40, "p" * 40)
        step.form, step.cause_on_target = form, cause_on_target
        return step

    def names(self, step):
        return [run[0] for run in step.runs()]

    def test_runs_of_the_commit_form_are_parent_and_commit(self):
        step = self.step("commit")
        self.assertEqual(self.names(step), ["parent", "commit"])
        self.assertEqual(step.runs()[0], ("parent", "p" * 40, "p" * 40, False, "MD"))
        self.assertEqual(step.runs()[1], ("commit", "c" * 40, "c" * 40, False, "MA"))

    def test_runs_of_the_forward_form_add_the_forward_reduction_on_the_parent_with_the_pins_of_the_commit(self):
        step = self.step("forward")
        self.assertEqual(self.names(step), ["parent", "commit", "forward reduction"])
        self.assertEqual(step.runs()[2], ("forward reduction", "p" * 40, "c" * 40, True, "M"))

    def test_runs_of_the_reverse_form_add_the_reverse_reduction_on_the_commit_with_the_pins_of_the_parent(self):
        step = self.step("reverse")
        self.assertEqual(self.names(step), ["parent", "commit", "reverse reduction"])
        self.assertEqual(step.runs()[2], ("reverse reduction", "c" * 40, "p" * 40, True, "M"))

    def test_runs_of_first_record_and_environment_and_of_a_cause_on_the_target_are_the_commit_run(self):
        for step in (self.step("first-record"), self.step("environment"), self.step("commit", cause_on_target=True),
                     self.step("forward", cause_on_target=True)):
            self.assertEqual(self.names(step), ["commit"])

    def test_runs_of_removal_are_the_parent_run(self):
        self.assertEqual(self.names(self.step("removal")), ["parent"])

    def test_statistical_filter_matches_the_given_tests_and_no_other(self):
        step = self.step("commit")
        step.tests = [("M", "examples/a", "omnetpp.ini", "General"), ("A", "showcases/x", "y.ini", "Some-Config")]
        regex = step.statistical_filter("MA")
        self.assertTrue(re.search(regex, "<T working_directory=examples/a, ini_file=omnetpp.ini, config=General) >"))
        self.assertTrue(re.search(regex, "working_directory=showcases/x, ini_file=y.ini, config=Some-Config)"))
        self.assertFalse(re.search(regex, "working_directory=examples/b, ini_file=omnetpp.ini, config=General)"))

    def test_statistical_filter_does_not_match_a_longer_config_name(self):
        step = self.step("commit")
        step.tests = [("M", "examples/a", "omnetpp.ini", "General")]
        regex = step.statistical_filter("M")
        self.assertFalse(re.search(regex, "working_directory=examples/a, ini_file=omnetpp.ini, config=General2)"))
        self.assertFalse(re.search(regex, "working_directory=examples/a, ini_file=omnetpp.ini, config=General-Long)"))

    def test_statistical_filter_uses_only_the_given_statuses_and_is_none_without_a_test(self):
        step = self.step("commit")
        step.tests = [("D", "examples/a", "omnetpp.ini", "General")]
        self.assertIsNone(step.statistical_filter("MA"))
        self.assertTrue(re.search(step.statistical_filter("MD"), REPR_A))

    def test_chart_filter_matches_the_whole_folder_name_only(self):
        step = self.step("commit")
        step.charts = [("M", "showcases/a"), ("M", "showcases/c"), ("A", "showcases/d")]
        regex = step.chart_filter("M")
        self.assertTrue(re.search(regex, "showcases/a"))
        self.assertTrue(re.search(regex, "showcases/c"))
        self.assertFalse(re.search(regex, "showcases/d"))
        self.assertFalse(re.search(regex, "showcases/ab"))
        self.assertFalse(re.search(regex, "xshowcases/a"))
        self.assertIsNone(step.chart_filter("D"))

    def test_count_counts_the_distinct_tests_and_all_charts(self):
        step = self.step("commit")
        step.tests = [("M", "a", "i.ini", "C"), ("M", "a", "i.ini", "C"), ("D", "b", "i.ini", "C")]
        step.charts = [("M", "f"), ("M", "f")]
        self.assertEqual(step.count("statistical", "M"), 1)
        self.assertEqual(step.count("statistical", "MD"), 2)
        self.assertEqual(step.count("chart", "M"), 2)


class HelpersTest(unittest.TestCase):
    def log(self, text):
        folder = tempfile.TemporaryDirectory()
        self.addCleanup(folder.cleanup)
        path = os.path.join(folder.name, "x.log")
        with open(path, "w", encoding="utf-8") as f:
            f.write(text)
        return path

    def test_tests_in_log_reads_the_summary_line_with_color_codes(self):
        path = self.log("noise\n\x1b[1;32mTested statistical test (141 concurrent)\x1b[0m: 141 PASS in 35.8\n")
        self.assertEqual(ba.tests_in_log(path), 141)

    def test_tests_in_log_takes_the_last_of_two_summary_lines(self):
        path = self.log("Tested chart test (3 concurrent): 3 PASS in 1.0\n"
                        "Tested chart test (7 concurrent): 7 PASS in 1.0\n")
        self.assertEqual(ba.tests_in_log(path), 7)

    def test_tests_in_log_without_a_summary_line_is_none(self):
        self.assertIsNone(ba.tests_in_log(self.log("nothing here\n")))

    def test_result_file_splits_a_result_path_of_an_example(self):
        match = ba.RESULT_FILE.match("examples/a/omnetpp.ini-General-#0.sca")
        self.assertEqual((match["folder"], match["ini"], match["config"], match["run"]),
                         ("examples/a", "omnetpp.ini", "General", "0"))

    def test_result_file_splits_a_result_path_of_a_showcase_with_a_dash_in_the_config(self):
        match = ba.RESULT_FILE.match("showcases/x/y/parameterstudy.ini-Some-Config-#0.sca")
        self.assertEqual((match["folder"], match["ini"], match["config"]),
                         ("showcases/x/y", "parameterstudy.ini", "Some-Config"))

    def test_result_file_rejects_a_path_that_is_no_result(self):
        self.assertIsNone(ba.RESULT_FILE.match("examples/a/omnetpp.ini"))


STORE_PATH = "tests/fingerprint/store.json"
A_CC = "src/inet/a/A.cc"
RUN_A0 = ("examples/a", "omnetpp.ini", "General", 0)
RUN_A1 = ("examples/a", "omnetpp.ini", "General", 1)
RUN_B0 = ("examples/b", "omnetpp.ini", "General", 0)
RUN_C0 = ("examples/c", "omnetpp.ini", "General", 0)
FP_MESSAGE = "src: change: one value\n\nThe body.\n\n"
# the cause commit appends the lines "" and "int v1;" to A.cc; the context line of the patch is empty
EMPTY_CONTEXT_REVERSE = "--- a/src/inet/a/A.cc\n+++ b/src/inet/a/A.cc\n@@ -1,3 +1,2 @@\n int a = 0;\n \n-int v1;\n"


def store_text(fingerprints):
    """The text of a fingerprint store with one ingredient set for each run: {run: fingerprint}."""
    return json.dumps([{"working_directory": folder, "ini_file": ini, "config": config, "run_number": number,
                        "ingredients": "tplx", "fingerprint": fingerprint, "sim_time_limit": "10s",
                        "test_result": "PASS"}
                       for (folder, ini, config, number), fingerprint in fingerprints.items()], indent=2) + "\n"


class FingerprintTestCase(AttributionTestCase):
    """A store with three runs on master; the commits of the series change it."""

    def setUp(self):
        super().setUp()
        self.fingerprints = {RUN_A0: "a0-old", RUN_A1: "a1-old", RUN_B0: "b0-old"}
        self.git(self.inet, "checkout", "-q", "master")
        self.write(self.inet, STORE_PATH, store_text(self.fingerprints))
        self.git(self.inet, "add", STORE_PATH)
        self.git(self.inet, "commit", "-qm", "store")
        self.git(self.inet, "checkout", "-q", "topic")
        self.git(self.inet, "reset", "-q", "--hard", "master")

    def fingerprint_commit(self, updates, trailers="", reduction=None, source=1, files=None):
        """Makes an INET commit that sets the fingerprints of runs ({run: fingerprint}, None removes a run)."""
        for run, fingerprint in updates.items():
            if fingerprint is None:
                del self.fingerprints[run]
            else:
                self.fingerprints[run] = fingerprint
        message = FP_MESSAGE + (f"Reduction:\n{reduction}\n" if reduction else "") + trailers
        self.inet_commit(files={STORE_PATH: store_text(self.fingerprints), **(files or {})}, source=source)
        self.git(self.inet, "commit", "-q", "--amend", "-m", message.rstrip("\n"))

    def fingerprint_calls(self):
        return [c for c in self.calls() if c["action"] == "fingerprint"]

    def plan(self):
        result = self.tool("plan")
        self.assertEqual(result.returncode, 0, result.stderr)
        return result.stdout

    def run_ok(self):
        result = self.tool("run")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        return result


class FingerprintPlanTest(FingerprintTestCase):
    def test_a_changed_fingerprint_gives_a_step_with_the_parent_run_and_the_commit_run(self):
        self.fingerprint_commit({RUN_A0: "a0-new"})
        out = self.plan()
        self.assertIn("form commit; 0 statistical test(s), charts in 0 folder(s), 1 fingerprint run(s)", out)
        self.assertRegex(out, r"run parent: .*the expectations of \w+: 0 statistical test\(s\), "
                              r"charts of 0 folder\(s\), 1 fingerprint run\(s\)")
        self.assertRegex(out, r"run commit: .*, 1 fingerprint run\(s\)")
        self.assertEqual(out.count("  run "), 2)

    def test_a_commit_that_only_adds_and_removes_runs_gives_no_step(self):
        self.fingerprint_commit({RUN_C0: "c0-new", RUN_B0: None})
        out = self.plan()
        self.assertIn("No commit of the series moves a pin over a result or a fingerprint value. No run is necessary.",
                      out)
        self.assertNotIn("  run ", out)

    def test_a_commit_that_only_adds_an_ingredient_set_gives_no_step(self):
        added = json.loads(store_text(self.fingerprints))
        added.append({**added[0], "ingredients": "tplx2", "fingerprint": "extra"})
        self.inet_commit(files={STORE_PATH: json.dumps(added, indent=2) + "\n"})
        self.assertIn("No run is necessary", self.plan())

    def test_two_changed_runs_of_one_config_count_as_two_runs(self):
        self.fingerprint_commit({RUN_A0: "a0-new", RUN_A1: "a1-new"})
        self.assertIn("2 fingerprint run(s)", self.plan())

    def test_the_reverse_form_with_a_reduction_patch_lists_a_third_run(self):
        self.fingerprint_commit({RUN_A0: "a0-new"}, trailers="Attribution: reverse", reduction=REVERSE_PATCH)
        out = self.plan()
        self.assertIn("form reverse", out)
        self.assertEqual(out.count("  run "), 3)
        self.assertRegex(out, r"run reverse reduction: the tree \w+ with the patch, the expectations of \w+: "
                              r"0 statistical test\(s\), charts of 0 folder\(s\), 1 fingerprint run\(s\)")

    def test_the_reverse_form_without_a_reduction_patch_exits_2_and_asks_for_the_gate(self):
        self.fingerprint_commit({RUN_A0: "a0-new"}, trailers="Attribution: reverse")
        result = self.tool("plan")
        self.assertEqual(result.returncode, 2, result.stdout)
        self.assertIn("run check-baselines.sh first", result.stderr)

    def test_an_unknown_form_exits_2(self):
        self.fingerprint_commit({RUN_A0: "a0-new"}, trailers="Attribution: first-record")
        result = self.tool("plan")
        self.assertEqual(result.returncode, 2, result.stdout)
        self.assertIn("no usable Attribution for its fingerprint change", result.stderr)

    def test_the_form_of_the_pinned_baseline_commit_is_the_form_of_the_fingerprint_change(self):
        self.move(stat={SCA_A: sca(sent="11"), PATCH: REVERSE_PATCH}, stat_record={"attribution": f"reverse {PATCH}"},
                  files={STORE_PATH: store_text({**self.fingerprints, RUN_A0: "a0-new"})})
        out = self.plan()
        self.assertIn("form reverse; 1 statistical test(s), charts in 0 folder(s), 1 fingerprint run(s)", out)
        self.assertEqual(out.count("  run "), 3)


class FingerprintRunTest(FingerprintTestCase):
    def test_the_commit_form_runs_the_fingerprint_command_on_each_tree_with_its_own_store(self):
        self.fingerprint_commit({RUN_A0: "a0-new"})
        (commit,) = self.commits()
        parent = self.parent_of(commit)
        result = self.run_ok()
        self.assertIn("PASS run parent", result.stdout)
        self.assertIn("PASS run commit", result.stdout)
        self.assertEqual([(c["action"], c["head"], c["store"]) for c in self.calls()],
                         [("build", parent, "a0-old,a1-old,b0-old,"), ("fingerprint", parent, "a0-old,a1-old,b0-old,"),
                          ("build", commit, "a0-new,a1-old,b0-old,"), ("fingerprint", commit, "a0-new,a1-old,b0-old,")])

    def test_the_commit_form_does_not_run_the_checkout_of_the_baselines(self):
        self.fingerprint_commit({RUN_A0: "a0-new"})
        self.run_ok()
        self.assertNotIn("checkout", [c["action"] for c in self.calls()])

    def test_the_fingerprint_filter_matches_the_config_of_the_changed_run_only(self):
        self.fingerprint_commit({RUN_A0: "a0-new"})
        self.run_ok()
        for call in self.fingerprint_calls():
            self.assertTrue(call["args"].startswith("-f "), call["args"])
            regex = call["args"][3:]
            self.assertTrue(re.search(regex, "working_directory=examples/a, ini_file=omnetpp.ini, config=General)"))
            self.assertFalse(re.search(regex, "working_directory=examples/b, ini_file=omnetpp.ini, config=General)"))
            self.assertFalse(re.search(regex, "working_directory=examples/a, ini_file=omnetpp.ini, config=General2)"))

    def test_two_changed_runs_of_one_config_give_a_filter_that_selects_the_config_once(self):
        self.fingerprint_commit({RUN_A0: "a0-new", RUN_A1: "a1-new"})
        self.run_ok()
        for call in self.fingerprint_calls():
            self.assertEqual(call["args"].count("working_directory"), 1, call["args"])
            self.assertNotIn("|", call["args"])

    def test_a_run_that_selects_fewer_tests_than_the_changed_runs_fails(self):
        self.fingerprint_commit({RUN_A0: "a0-new", RUN_A1: "a1-new"})
        self.control_file("fingerprint.count", "1")
        result = self.tool("run")
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn("the fingerprint run selected 1 test(s), and the step moves 2", result.stdout)

    def test_a_run_that_selects_as_many_tests_as_the_changed_runs_passes(self):
        self.fingerprint_commit({RUN_A0: "a0-new", RUN_A1: "a1-new"})
        self.control_file("fingerprint.count", "2")
        self.run_ok()

    def test_a_fingerprint_command_that_exits_1_fails_the_run(self):
        self.fingerprint_commit({RUN_A0: "a0-new"})
        self.control_file("fingerprint.status", "1")
        result = self.tool("run")
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn("FAIL run parent", result.stdout)
        self.assertIn("the fingerprint tests fail", result.stdout)

    def test_a_fingerprint_log_without_a_summary_line_fails(self):
        self.fingerprint_commit({RUN_A0: "a0-new"})
        self.control_file("fingerprint.nosummary")
        result = self.tool("run")
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn("the fingerprint run selected an unknown number of test(s)", result.stdout)

    def test_the_reverse_reduction_run_sees_the_commit_with_the_patch_and_the_store_of_the_parent(self):
        self.fingerprint_commit({RUN_A0: "a0-new"}, trailers="Attribution: reverse", reduction=REVERSE_PATCH)
        (commit,) = self.commits()
        tip = self.head(self.inet)
        result = self.run_ok()
        self.assertIn("the expectations of", result.stdout)
        third = self.fingerprint_calls()[2]
        self.assertEqual(third["head"], commit)
        self.assertEqual(third["source"], "int a = 0;;")
        self.assertEqual(third["store"], "a0-old,a1-old,b0-old,")
        self.assertEqual(self.status(), "")
        self.assertEqual(self.head(self.inet), tip)
        self.assertEqual(self.branch(), "topic")

    def test_the_forward_reduction_run_sees_the_parent_with_the_patch_and_the_store_of_the_commit(self):
        self.fingerprint_commit({RUN_A0: "a0-new"}, trailers="Attribution: forward", reduction=FORWARD_PATCH)
        (commit,) = self.commits()
        self.run_ok()
        third = self.fingerprint_calls()[2]
        self.assertEqual(third["head"], self.parent_of(commit))
        self.assertEqual(third["source"], "int a = 0;;int v1;;")
        self.assertEqual(third["store"], "a0-new,a1-old,b0-old,")
        self.assertEqual(self.status(), "")

    def test_a_patch_whose_empty_context_lines_lost_their_space_still_applies(self):
        self.fingerprint_commit({RUN_A0: "a0-new"}, trailers="Attribution: reverse", reduction=EMPTY_CONTEXT_REVERSE,
                                source=0, files={A_CC: "int a = 0;\n\nint v1;\n"})
        (commit,) = self.commits()
        self.assertNotIn(" \n", self.git(self.inet, "log", "-1", "--format=%B").stdout)
        self.run_ok()
        third = self.fingerprint_calls()[2]
        self.assertEqual(third["head"], commit)
        self.assertEqual(third["source"], "int a = 0;;;")
        self.assertEqual(self.status(), "")

    def test_a_reduction_that_does_not_apply_fails_and_leaves_the_work_tree_clean(self):
        self.fingerprint_commit({RUN_A0: "a0-new"}, trailers="Attribution: reverse", reduction=BAD_PATCH)
        result = self.tool("run")
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn("the reduction does not apply", result.stdout)
        self.assertEqual(self.status(), "")
        self.assertEqual(self.branch(), "topic")

    def test_a_step_with_a_pin_move_and_a_fingerprint_change_runs_both_commands_in_each_run(self):
        self.move_commit_form(files={STORE_PATH: store_text({**self.fingerprints, RUN_A0: "a0-new"})})
        result = self.run_ok()
        self.assertEqual(result.stdout.count("form commit"), 1)
        self.assertEqual([c["action"] for c in self.calls()],
                         ["build", "checkout", "statistical", "fingerprint"] * 2)
        self.assertEqual(self.status(), "")


class ReductionWithoutStoreTest(AttributionTestCase):
    """The tree of a reduction run holds no store, and the commit of the expectations holds one."""

    def direct_run(self, folder_exists):
        if folder_exists:
            self.inet_commit(files={"tests/fingerprint/README": "notes\n"})
        self.inet_commit(files={STORE_PATH: store_text({RUN_A0: "a0-new"})})
        expectations = self.head(self.inet)
        parent = self.parent_of(expectations)
        step = ba.Step(expectations, parent)
        step.form, step.reduction, step.fingerprints = "forward", FORWARD_PATCH, [RUN_A0[:3] + ("0",)]
        commands = {"build": "true", "checkout": "true",
                    "fingerprint": "grep -q a0-new tests/fingerprint/store.json && "
                                   "echo 'Tested fingerprint test (1 concurrent): 1 PASS in 1.0'"}
        logs = Path(self.tempdir.name) / "direct-logs"
        logs.mkdir()
        self.addCleanup(os.chdir, os.getcwd())
        os.chdir(self.inet)
        try:
            passed, text = ba.do_run(step, ("forward reduction", parent, expectations, True, "M"), commands, str(logs), 1)
            # the state of the tree after the run, before the checkout of the branch brings the store back
            return (passed, text, (self.inet / STORE_PATH).exists(), (self.inet / A_CC).read_text(), self.status(),
                    (self.inet / STORE_PATH).parent.exists())
        finally:
            self.git(self.inet, "checkout", "-q", "topic")

    def test_the_run_removes_the_store_that_it_wrote_and_restores_the_patched_file(self):
        passed, text, store_exists, source, status, _ = self.direct_run(folder_exists=True)
        self.assertTrue(passed, text)
        self.assertFalse(store_exists)
        self.assertEqual(source, "int a = 0;\n")
        self.assertEqual(status, "")

    def test_the_run_writes_the_store_into_a_tree_without_the_folder_of_the_store(self):
        passed, text, store_exists, source, status, folder_exists = self.direct_run(folder_exists=False)
        self.assertTrue(passed, text)
        self.assertFalse(store_exists)
        self.assertFalse(folder_exists)
        self.assertEqual(status, "")


if __name__ == "__main__":
    unittest.main()
