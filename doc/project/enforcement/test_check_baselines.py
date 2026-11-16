#!/usr/bin/env python3
"""Tests of the baseline gate (check-baselines.sh) and of the summary of baseline_summary.py.

Each gate test builds three temporary git repositories: the baseline repositories "statistics" and
"media", and an INET-like repository with an inet.opp that pins one commit of each baseline.
"""

from pathlib import Path
import os
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))
import baseline_summary  # noqa: E402

CHECKER = Path(__file__).with_name("check-baselines.sh").resolve()

SUBJECT = "src: change: one value"
ANALYSIS = ("The change adds a counter to the host. Each host now sends one more packet, so the number of sent "
            "packets and the number of received packets both rise by one, and the other rows stay the same.")
REASON = "the host sends one more packet in this test"
CHANGE = "Change: src | behavior.change | "
SCA_A = "examples/a/omnetpp.ini-General-#0.sca"
SCA_B = "examples/b/omnetpp.ini-General-#0.sca"
PNG_A = "showcases/a/doc/chart.png"
PNG_A2 = "showcases/a/doc/chart2.png"
PNG_DOC = "showcases/b/doc/chart.png"
ANF = "showcases/a/A.anf"
ANF_TEXT = ('<?xml version="1.0"?>\n<analysis>\n<property name="image_export_filename" value="chart"/>\n'
            '<property name="image_export_filename" value="chart2"/>\n</analysis>\n')
ANF_TEXT_WITHOUT_CHART = ANF_TEXT.replace('<property name="image_export_filename" value="chart"/>\n', "")
MEDIA_LINK = "../../../media/showcases/a/doc"
ENTRY = {"folder": "media", "read_folder": "doc/media"}
PATCH = "attribution/x.patch"

# the cause commit appends the line "int v1;" to src/inet/a/A.cc
REVERSE_PATCH = "--- a/src/inet/a/A.cc\n+++ b/src/inet/a/A.cc\n@@ -1,2 +1 @@\n int a = 0;\n-int v1;\n"
FORWARD_PATCH = "--- a/src/inet/a/A.cc\n+++ b/src/inet/a/A.cc\n@@ -1 +1,2 @@\n int a = 0;\n+int v1;\n"
BAD_PATCH = "--- a/src/inet/a/A.cc\n+++ b/src/inet/a/A.cc\n@@ -1,2 +1 @@\n int a = 0;\n-int other;\n"
LONG_PATCH = ("--- a/src/inet/a/A.cc\n+++ b/src/inet/a/A.cc\n@@ -1 +1,102 @@\n int a = 0;\n"
              + "".join(f"+int w{i};\n" for i in range(101)))

OPP = """SimulationProject(
    name="inet",
    test_parameters=%r,
)
"""


def sca(sent="10", rcvd="7"):
    """The text of a minimal result file."""
    return ('version 3\nrun General-0-x\nattr experiment General\nattr measurement ""\nattr replication #0\n'
            f'scalar Net.host "packets sent" {sent}\nscalar Net.host rcvdPk:count {rcvd}\n')


def opp_text(pins, omit=(), statistical_repository="inet-framework/statistics"):
    """The text of inet.opp with the given pins, in the form of the real file."""
    parameters = {}
    for kind, repository, folder in (("statistical", statistical_repository, "statistics"),
                                     ("chart", "inet-framework/media", "media")):
        if kind not in omit:
            baseline = {"repository": repository, "folder": folder}
            if kind == "chart":
                baseline["read_folder"] = "doc/media"
            if kind in pins:
                baseline["ref"] = pins[kind]
            parameters[kind] = {"baseline": baseline}
    return OPP % (parameters,)


class GateTestCase(unittest.TestCase):
    """The three repositories, and the helpers that build commits in them."""

    def setUp(self):
        self.tempdir = tempfile.TemporaryDirectory()
        root = Path(self.tempdir.name)
        self.stat, self.media, self.inet = root / "statistics", root / "media", root / "inet"
        for repository in (self.stat, self.media, self.inet):
            repository.mkdir()
            self.git(repository, "init", "-q", "-b", "master")
            self.git(repository, "config", "user.email", "test@example.invalid")
            self.git(repository, "config", "user.name", "Baseline Test")
            self.git(repository, "config", "commit.gpgsign", "false")
        self.write(self.stat, SCA_A, sca())
        self.write(self.stat, SCA_B, sca())
        self.write(self.media, PNG_A, b"\x89PNG old")
        self.write(self.media, PNG_DOC, b"\x89PNG other")
        self.pins = {}
        for kind, repository in (("statistical", self.stat), ("chart", self.media)):
            self.git(repository, "add", "-A")
            self.git(repository, "commit", "-qm", "first baseline")
            self.pins[kind] = self.head(repository)
        self.first = dict(self.pins)
        self.write(self.inet, "src/inet/a/A.cc", "int a = 0;\n")
        self.write(self.inet, "inet.opp", opp_text(self.pins))
        # the chart of the analysis file exports to the folder doc/media, a link into the media folder
        self.write(self.inet, ANF, ANF_TEXT)
        (self.inet / "showcases/a/doc").mkdir(parents=True)
        os.symlink(MEDIA_LINK, self.inet / "showcases/a/doc/media")
        self.git(self.inet, "add", "-A")
        self.git(self.inet, "commit", "-qm", "fixture")
        self.git(self.inet, "checkout", "-qb", "topic")

    def tearDown(self):
        self.tempdir.cleanup()

    @staticmethod
    def git(repository, *args):
        return subprocess.run(["git", *args], cwd=repository, check=True, capture_output=True, text=True)

    def head(self, repository):
        return self.git(repository, "rev-parse", "HEAD").stdout.strip()

    @staticmethod
    def write(repository, relative_path, content):
        path = Path(repository) / relative_path
        path.parent.mkdir(parents=True, exist_ok=True)
        if isinstance(content, bytes):
            path.write_bytes(content)
        else:
            path.write_text(content, encoding="utf-8")

    def result_filter(self, repository):
        """The filter of the results of a baseline repository, as the gate computes it for the parent commit."""
        if repository == self.stat:
            return baseline_summary.result_filter("statistical")
        return baseline_summary.result_filter("chart", baseline_summary.chart_images_at("HEAD", ENTRY, cwd=self.inet))

    def plain(self, repository, changes, subject="Change files that are no result"):
        """Makes a baseline commit with no record. Returns the hash."""
        for path, content in changes.items():
            self.write(repository, path, content)
        self.git(repository, "add", "-A")
        self.git(repository, "commit", "-q", "-m", subject)
        return self.head(repository)

    def record(self, repository, changes, inet_subject=SUBJECT, attribution="commit", analysis=ANALYSIS,
               block=None, subject="Move the results"):
        """Makes a baseline commit with its record. A change of None removes the file. inet_subject and
        attribution of None leave the trailer out. Each computed line gets a reason. A block of "none" leaves
        the list out, a function edits the lines of the list. Returns the hash."""
        for path, content in changes.items():
            if content is None:
                self.git(repository, "rm", "-q", path)
            else:
                self.write(repository, path, content)
        self.git(repository, "add", "-A")
        moved = baseline_summary.analyze(str(repository), is_result=self.result_filter(repository))
        lines = baseline_summary.block(moved, {line: REASON for line in baseline_summary.lines(moved)})
        if block == "none":
            lines = None
        elif block is not None:
            lines = block(lines)
        trailers = []
        if inet_subject is not None:
            trailers.append(f"INET-Subject: {inet_subject}")
        if attribution is not None:
            trailers.append(f"Attribution: {attribution}")
        parts = [subject, analysis, "\n".join(lines) if lines else "", "\n".join(trailers)]
        self.git(repository, "commit", "-q", "-m", "\n\n".join(part for part in parts if part))
        return self.head(repository)

    def inet_commit(self, subject=SUBJECT, source=0, trailer=CHANGE + "-", pins=None, opp=None, files=None):
        """Makes an INET commit. It appends the given number of lines to A.cc, writes inet.opp and the files."""
        self.pins.update(pins or {})
        for path, content in (files or {}).items():
            self.write(self.inet, path, content)
        self.write(self.inet, "inet.opp", opp if opp is not None else opp_text(self.pins))
        if source:
            path = self.inet / "src/inet/a/A.cc"
            text = path.read_text(encoding="utf-8")
            start = len(text.splitlines())
            path.write_text(text + "".join(f"int v{start + i};\n" for i in range(source)), encoding="utf-8")
        self.git(self.inet, "add", "-A")
        message = f"{subject}\n\nThe body.\n\n{trailer}" if trailer else f"{subject}\n\nThe body."
        self.git(self.inet, "commit", "-q", "--allow-empty", "-m", message)

    def move(self, stat=None, media=None, subject=SUBJECT, source=1, trailer=None, stat_record=None,
             media_record=None, files=None):
        """One INET commit that moves the pin of each baseline with changes: the baseline commit with
        its record, then the INET commit."""
        pins = {}
        for kind, repository, changes, options in (("statistical", self.stat, stat, stat_record),
                                                    ("chart", self.media, media, media_record)):
            if changes is not None:
                pins[kind] = self.record(repository, changes, **{"inet_subject": subject, **(options or {})})
        if trailer is None:
            trailer = CHANGE + " ".join(sorted(pins))
        self.inet_commit(subject, source, trailer, pins, files=files)

    def gate(self, *args, series="master..topic", repositories=True):
        extra = []
        if repositories:
            extra = ["--baseline-repository", f"statistical={self.stat}",
                     "--baseline-repository", f"chart={self.media}"]
        return subprocess.run(["bash", str(CHECKER), series, *extra, *args], cwd=self.inet,
                              capture_output=True, text=True)

    def assert_fails(self, text, result=None):
        result = result or self.gate()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn(text, result.stdout)
        return result

    def assert_passes(self, result=None):
        result = result or self.gate()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("PASS", result.stdout)
        return result


class PassTest(GateTestCase):
    def test_a_series_with_no_pin_move_passes(self):
        self.inet_commit(source=1)
        result = self.assert_passes()
        self.assertIn("0 pin move(s)", result.stdout)

    def test_a_correct_pin_move_with_attribution_commit_passes(self):
        self.move(stat={SCA_A: sca(sent="11")})
        result = self.assert_passes()
        self.assertIn("1 pin move(s)", result.stdout)

    def test_a_pin_move_of_both_baselines_passes(self):
        self.move(stat={SCA_A: sca(sent="11")}, media={PNG_A: b"\x89PNG new"})
        result = self.assert_passes()
        self.assertIn("2 pin move(s)", result.stdout)

    def test_first_record_with_a_file_that_is_added_passes(self):
        self.move(stat={"examples/c/omnetpp.ini-General-#0.sca": sca()}, stat_record={"attribution": "first-record"})
        self.assert_passes()

    def test_removal_with_a_file_that_is_removed_passes(self):
        self.move(stat={SCA_B: None}, stat_record={"attribution": "removal"})
        self.assert_passes()

    def test_environment_passes_when_the_inet_commit_changes_only_inet_opp(self):
        self.move(stat={SCA_A: sca(sent="11")}, source=0, stat_record={"attribution": "environment"})
        self.assert_passes()

    def test_a_reverse_reduction_that_applies_to_the_cause_commit_passes(self):
        self.move(stat={SCA_A: sca(sent="11"), PATCH: REVERSE_PATCH}, stat_record={"attribution": f"reverse {PATCH}"})
        self.assert_passes()

    def test_a_forward_reduction_that_applies_to_the_parent_of_the_cause_commit_passes(self):
        self.move(stat={SCA_A: sca(sent="11"), PATCH: FORWARD_PATCH}, stat_record={"attribution": f"forward {PATCH}"})
        self.assert_passes()

    def test_a_number_with_another_text_and_the_same_value_is_no_change(self):
        changes = {SCA_A: sca(sent="10.0", rcvd="8")}
        self.write(self.stat, SCA_A, changes[SCA_A])
        self.git(self.stat, "add", "-A")
        lines = baseline_summary.lines(baseline_summary.analyze(str(self.stat)))
        self.assertEqual(lines, ["statistical tests changed, large (1; examples/a 1): rcvdPk:count"])
        self.git(self.stat, "reset", "-q", "--hard")
        self.move(stat=changes)
        self.assert_passes()


class PinViolationTest(GateTestCase):
    def test_a_pin_that_is_a_branch_name_fails(self):
        self.inet_commit(pins={"statistical": "master"})
        self.assert_fails("the pin of the statistical baseline is 'master', not a full commit hash")

    def test_a_commit_that_removes_a_baseline_entry_fails(self):
        self.inet_commit(opp=opp_text(self.pins, omit=("chart",)))
        self.assert_fails("removes the baseline entry of the chart tests")

    def test_a_commit_that_changes_the_repository_of_a_baseline_fails(self):
        self.inet_commit(opp=opp_text(self.pins, statistical_repository="other/statistics"))
        self.assert_fails("changes the repository or the folder of the statistical baseline")

    def test_a_pinned_commit_that_is_not_in_the_baseline_repository_fails(self):
        self.inet_commit(pins={"statistical": "0123456789abcdef0123456789abcdef01234567"})
        self.assert_fails("which is not in the baseline repository")

    def test_a_pin_that_moves_over_two_baseline_commits_fails(self):
        self.record(self.stat, {SCA_A: sca(sent="11")})
        pin = self.record(self.stat, {SCA_A: sca(sent="12")})
        self.inet_commit(source=1, trailer=CHANGE + "statistical", pins={"statistical": pin})
        self.assert_fails("moves the statistical pin over 2 baseline commits that change results; "
                          "one cause commit has one baseline commit")

    def test_a_pin_that_moves_to_a_commit_that_does_not_descend_from_the_old_pin_fails(self):
        self.git(self.stat, "checkout", "-q", "--orphan", "other")
        self.git(self.stat, "commit", "-qm", "unrelated")
        self.inet_commit(source=1, trailer=CHANGE + "statistical", pins={"statistical": self.head(self.stat)})
        self.assert_fails("which does not stand on the pin of the parent")


class RecordViolationTest(GateTestCase):
    def test_no_attribution_trailer_fails(self):
        self.move(stat={SCA_A: sca(sent="11")}, stat_record={"attribution": None})
        self.assert_fails("needs one Attribution trailer")

    def test_an_unknown_attribution_form_fails(self):
        self.move(stat={SCA_A: sca(sent="11")}, stat_record={"attribution": "bogus"})
        self.assert_fails("Attribution is 'bogus'; the forms are")

    def test_forward_with_no_file_fails(self):
        self.move(stat={SCA_A: sca(sent="11")}, stat_record={"attribution": "forward"})
        self.assert_fails("Attribution is 'forward'; the forms are")

    def test_a_named_reduction_file_that_is_not_in_the_commit_fails(self):
        self.move(stat={SCA_A: sca(sent="11")}, stat_record={"attribution": "reverse attribution/missing.patch"})
        self.assert_fails("names the reduction attribution/missing.patch, which is not a file in attribution/")

    def test_no_inet_subject_fails(self):
        self.move(stat={SCA_A: sca(sent="11")}, stat_record={"inet_subject": None})
        self.assert_fails("needs one trailer that names the cause")

    def test_an_inet_subject_that_is_not_the_subject_of_the_inet_commit_fails(self):
        self.move(stat={SCA_A: sca(sent="11")}, stat_record={"inet_subject": "src: change: another value"})
        self.assert_fails("INET-Subject is not the subject of")

    def test_no_baseline_changes_list_fails(self):
        self.move(stat={SCA_A: sca(sent="11")}, stat_record={"block": "none"})
        self.assert_fails("has no 'Baseline changes:' list")

    def test_a_list_with_a_changed_number_fails(self):
        def wrong(lines):
            return [line.replace("(1; examples/a 1)", "(2; examples/a 2)") for line in lines]
        self.move(stat={SCA_A: sca(sent="11")}, stat_record={"block": wrong})
        self.assert_fails("has a 'Baseline changes:' list that is not the computed list")

    def test_the_failure_of_a_missing_list_prints_the_expected_list(self):
        self.move(stat={SCA_A: sca(sent="11")}, stat_record={"block": "none"})
        result = self.assert_fails("has no 'Baseline changes:' list")
        self.assertIn("  statistical tests changed, large (1; examples/a 1): packets sent — ", result.stdout)

    def test_a_list_with_the_right_lines_in_another_order_fails(self):
        def reversed_lines(lines):
            return lines[:1] + lines[:0:-1]
        self.move(stat={SCA_A: sca(sent="11"), SCA_B: sca(rcvd="8")}, stat_record={"block": reversed_lines})
        self.assert_fails("has a 'Baseline changes:' list that is not the computed list")

    def test_a_line_with_no_reason_gives_exactly_one_violation_that_names_the_line(self):
        def without_reason(lines):
            return [line.split(" — ")[0] + " — " if "rcvdPk:count" in line else line for line in lines]
        self.move(stat={SCA_A: sca(sent="11"), SCA_B: sca(rcvd="8")}, stat_record={"block": without_reason})
        result = self.assert_fails("no reason for: statistical tests changed, large (1; examples/b 1): rcvdPk:count")
        self.assertEqual(result.stdout.count("VIOLATION"), 1, result.stdout)
        self.assertNotIn("no reason for: statistical tests changed, large (1; examples/a 1)", result.stdout)

    def test_a_line_with_no_separator_has_no_reason(self):
        def without_separator(lines):
            return [line.split(" — ")[0] for line in lines]
        self.move(stat={SCA_A: sca(sent="11")}, stat_record={"block": without_separator})
        self.assert_fails("no reason for: statistical tests changed, large (1; examples/a 1): packets sent")

    def test_a_reason_may_continue_on_an_indented_line(self):
        def continued(lines):
            return lines + ["      and the other rows stay the same."]
        self.move(stat={SCA_A: sca(sent="11")}, stat_record={"block": continued})
        self.assert_passes()

    def test_an_analysis_in_the_message_is_optional(self):
        self.move(stat={SCA_A: sca(sent="11")}, stat_record={"analysis": ""})
        self.assert_passes()

    def test_a_result_file_that_changes_only_in_text_fails(self):
        self.move(stat={SCA_A: sca(sent="10.0")})
        self.assert_fails("and no value in it; a baseline changes only for a test that moves (TR-BASELINE-PARENT)")

    def test_first_record_with_a_changed_file_fails(self):
        self.move(stat={SCA_A: sca(sent="11")}, stat_record={"attribution": "first-record"})
        self.assert_fails("has the form first-record and changes or removes 1 file(s) that exist")

    def test_removal_with_an_added_file_fails(self):
        self.move(stat={"examples/c/omnetpp.ini-General-#0.sca": sca()}, stat_record={"attribution": "removal"})
        self.assert_fails("has the form removal and adds or changes 1 file(s)")

    def test_a_baseline_commit_that_changes_an_unnamed_file_under_attribution_fails(self):
        self.move(stat={SCA_A: sca(sent="11"), "attribution/other.patch": FORWARD_PATCH})
        self.assert_fails("changes attribution/other.patch; a baseline commit adds only the reduction")


class AttributionViolationTest(GateTestCase):
    def test_attribution_commit_with_more_than_100_source_lines_fails(self):
        self.move(stat={SCA_A: sca(sent="11")}, source=101)
        self.assert_fails("changes 101 source lines, above 100")

    def test_attribution_commit_with_no_source_fails(self):
        self.move(stat={SCA_A: sca(sent="11")}, source=0)
        self.assert_fails("changes no source")

    def test_attribution_environment_with_source_fails(self):
        self.move(stat={SCA_A: sca(sent="11")}, stat_record={"attribution": "environment"})
        self.assert_fails("has the form environment, and")

    def test_a_reverse_reduction_that_does_not_apply_fails(self):
        self.move(stat={SCA_A: sca(sent="11"), PATCH: BAD_PATCH}, stat_record={"attribution": f"reverse {PATCH}"})
        self.assert_fails("has a reverse reduction that does not apply to the cause commit")

    def test_a_forward_reduction_that_applies_to_the_cause_commit_only_fails(self):
        self.move(stat={SCA_A: sca(sent="11"), PATCH: REVERSE_PATCH}, stat_record={"attribution": f"forward {PATCH}"})
        self.assert_fails("has a forward reduction that does not apply to the parent of the cause commit")

    def test_a_reduction_above_100_changed_lines_fails(self):
        self.move(stat={SCA_A: sca(sent="11"), PATCH: LONG_PATCH}, stat_record={"attribution": f"forward {PATCH}"})
        result = self.assert_fails("has a reduction of 101 changed lines, above 100")
        self.assertNotIn("does not apply", result.stdout)

    def test_a_change_trailer_that_does_not_name_the_baseline_fails(self):
        self.move(stat={SCA_A: sca(sent="11")}, trailer=CHANGE + "chart")
        self.assert_fails("its Change: trailer does not name 'statistical' (CR-OBL-BASELINE)")

    def test_a_question_mark_in_the_obligations_field_is_no_violation(self):
        self.move(stat={SCA_A: sca(sent="11")}, trailer=CHANGE + "?")
        self.assert_passes()

    def test_two_baseline_commits_with_different_attributions_fail(self):
        self.move(stat={SCA_A: sca(sent="11")}, media={PNG_A2: b"\x89PNG added"},
                  media_record={"attribution": "first-record"})
        result = self.assert_fails("moves 2 pins, and their baseline commits do not have the same attribution")
        self.assertEqual(result.stdout.count("VIOLATION"), 1, result.stdout)


class NoteTest(GateTestCase):
    def test_attribution_commit_with_31_source_lines_gives_a_note(self):
        self.move(stat={SCA_A: sca(sent="11")}, source=31)
        result = self.assert_passes()
        self.assertIn("note:", result.stdout)
        self.assertIn("changes 31 source lines, above 30", result.stdout)

    def test_attribution_commit_with_100_source_lines_gives_a_note(self):
        self.move(stat={SCA_A: sca(sent="11")}, source=100)
        result = self.assert_passes()
        self.assertIn("changes 100 source lines, above 30", result.stdout)

    def test_tests_that_move_different_rows_give_the_split_note(self):
        self.move(stat={SCA_A: sca(sent="11"), SCA_B: sca(rcvd="8")})
        result = self.assert_passes()
        self.assertIn("note:", result.stdout)
        self.assertIn("has 2 lines of changed entries", result.stdout)
        self.assertIn("(PR-SPLIT-ONE-CHANGE)", result.stdout)

    def test_the_commit_that_adds_the_first_pins_gives_a_note(self):
        self.git(self.inet, "checkout", "-q", "master")
        self.write(self.inet, "inet.opp", opp_text({}))
        self.git(self.inet, "commit", "-q", "-a", "--amend", "-m", "fixture without pins")
        self.git(self.inet, "checkout", "-q", "-B", "topic")
        self.inet_commit(pins=self.first)
        result = self.assert_passes()
        self.assertIn("adds the first pin of the chart baseline", result.stdout)
        self.assertIn("adds the first pin of the statistical baseline", result.stdout)
        self.assertIn("0 pin move(s)", result.stdout)


class UsageTest(GateTestCase):
    def test_no_range_argument_is_a_usage_error(self):
        result = subprocess.run(["bash", str(CHECKER)], cwd=self.inet, capture_output=True, text=True)
        self.assertEqual(result.returncode, 2, result.stdout)
        self.assertIn("usage:", result.stderr)

    def test_a_range_that_is_not_valid_is_a_usage_error(self):
        for series in ("nothing..topic", "master", "master...topic"):
            result = self.gate(series=series)
            self.assertEqual(result.returncode, 2, series + result.stdout)
            self.assertIn("is not a valid commit range", result.stderr)

    def test_a_baseline_repository_argument_without_a_path_is_a_usage_error(self):
        result = self.gate("--baseline-repository", "statistical", repositories=False)
        self.assertEqual(result.returncode, 2, result.stdout)
        self.assertIn("--baseline-repository needs KIND=PATH", result.stderr)


class UrlTest(GateTestCase):
    def test_a_correct_pin_move_passes_with_a_baseline_repository_that_is_a_url(self):
        self.git(self.stat, "config", "uploadpack.allowFilter", "true")
        self.git(self.stat, "config", "uploadpack.allowAnySHA1InWant", "true")
        self.move(stat={SCA_A: sca(sent="11")})
        result = subprocess.run(["bash", str(CHECKER), "master..topic", "--baseline-repository",
                                 f"statistical=file://{self.stat}"], cwd=self.inet, capture_output=True, text=True)
        self.assert_passes(result)
        self.assertNotIn("read from the local clone", result.stdout)


class ResultRuleTest(GateTestCase):
    """Not each file of a baseline repository is a result; a commit with no result needs no record."""

    DOC_TRAILER = "Change: doc | behavior.change | -"

    def test_a_media_commit_that_changes_only_a_documentation_image_needs_no_record(self):
        doc = self.plain(self.media, {PNG_DOC: b"\x89PNG new documentation"})
        self.inet_commit("doc: change: an image", 0, self.DOC_TRAILER, {"chart": doc})
        result = self.assert_passes()
        self.assertIn("1 commit(s) that change no result", result.stdout)

    def test_a_statistics_commit_that_only_adds_a_readme_needs_no_record(self):
        readme = self.plain(self.stat, {"README.md": "About the results.\n"})
        self.inet_commit("doc: change: a readme", 0, self.DOC_TRAILER, {"statistical": readme})
        result = self.assert_passes()
        self.assertIn("change no result", result.stdout)

    def test_a_pin_move_over_a_documentation_commit_and_then_a_result_commit_passes(self):
        self.plain(self.media, {PNG_DOC: b"\x89PNG new documentation"})
        self.move(media={PNG_A: b"\x89PNG new"})
        result = self.assert_passes()
        self.assertIn("moves the chart pin over 2 commits;", result.stdout)
        self.assertIn("changes results and has the record", result.stdout)

    def test_a_pin_move_over_a_result_commit_and_then_a_documentation_commit_passes(self):
        self.record(self.media, {PNG_A: b"\x89PNG new"})
        doc = self.plain(self.media, {PNG_DOC: b"\x89PNG new documentation"})
        self.inet_commit(SUBJECT, 1, CHANGE + "chart", {"chart": doc})
        result = self.assert_passes()
        self.assertIn("moves the chart pin over 2 commits;", result.stdout)

    def test_a_pin_move_over_two_commits_that_both_change_results_fails(self):
        self.record(self.media, {PNG_A: b"\x89PNG new"})
        pin = self.record(self.media, {PNG_A: b"\x89PNG newer"})
        self.inet_commit(SUBJECT, 1, CHANGE + "chart", {"chart": pin})
        self.assert_fails("moves the chart pin over 2 baseline commits that change results; "
                          "one cause commit has one baseline commit")

    def test_a_result_commit_that_also_changes_a_documentation_image_lists_only_the_chart(self):
        self.move(media={PNG_A: b"\x89PNG new", PNG_DOC: b"\x89PNG new documentation"})
        self.assert_passes()
        message = self.git(self.media, "log", "-1", "--format=%B").stdout
        self.assertEqual([text for text, _ in baseline_summary.read_block(message)],
                         ["chart images changed (1; showcases/a 1): chart"])

    def test_a_list_that_names_a_documentation_image_fails(self):
        def with_documentation(lines):
            return lines + [f"  chart images changed (1; showcases/b 1): chart{baseline_summary.SEPARATOR}{REASON}"]
        self.move(media={PNG_A: b"\x89PNG new", PNG_DOC: b"\x89PNG new documentation"},
                  media_record={"block": with_documentation})
        self.assert_fails("has a 'Baseline changes:' list that is not the computed list")

    def test_a_behavior_free_change_that_moves_the_statistical_pin_over_a_result_fails(self):
        self.move(stat={SCA_A: sca(sent="11")}, trailer="Change: src | refactor | statistical")
        self.assert_fails("claims 'refactor' and moves the pin of the statistical baseline over a result (CR-OBL-INERT)")

    def test_a_comment_change_that_moves_the_chart_pin_over_a_documentation_commit_is_no_violation(self):
        doc = self.plain(self.media, {PNG_DOC: b"\x89PNG new documentation"})
        self.inet_commit("doc: comment: an image", 0, "Change: doc | comment | -", {"chart": doc})
        result = self.assert_passes()
        self.assertNotIn("CR-OBL-INERT", result.stdout)

    def test_an_image_that_only_the_parent_of_the_cause_commit_names_is_a_result_when_it_goes(self):
        # the cause commit removes the chart property from the .anf file, and the media commit removes the image
        self.move(media={PNG_A: None}, media_record={"attribution": "removal"}, files={ANF: ANF_TEXT_WITHOUT_CHART})
        self.assert_passes()

    def test_the_removal_of_an_image_that_only_the_parent_names_needs_the_record(self):
        self.move(media={PNG_A: None}, media_record={"attribution": "removal", "block": "none"},
                  files={ANF: ANF_TEXT_WITHOUT_CHART})
        self.assert_fails("has no 'Baseline changes:' list")

    def test_a_pin_that_moves_over_a_merge_commit_of_the_baseline_repository_fails(self):
        self.git(self.media, "checkout", "-q", "-b", "side")
        self.plain(self.media, {"side.txt": "side\n"})
        self.git(self.media, "checkout", "-q", "master")
        self.plain(self.media, {"main.txt": "main\n"})
        self.git(self.media, "merge", "-q", "--no-ff", "-m", "merge side", "side")
        self.inet_commit(SUBJECT, 1, CHANGE + "chart", {"chart": self.head(self.media)})
        self.assert_fails("moves the chart pin over a merge commit")

    def test_chart_images_at_reads_the_git_objects_like_the_work_tree(self):
        images = baseline_summary.chart_images_at("HEAD", ENTRY, cwd=self.inet)
        self.assertEqual(images, {PNG_A, PNG_A2})
        self.assertEqual(baseline_summary.chart_images_in_work_tree(ENTRY, cwd=self.inet), images)


K = ("Net.host", "packets sent")


def changed_test(path, *names, old=100.0, new=200.0):
    """A changed test of the analysis, with the given names of changed rows."""
    return ("M", path, [(("Net.host", name), old, new) for name in names], [], [])


class BaselineSummaryTest(unittest.TestCase):
    def test_the_chart_images_of_a_work_tree_that_git_cannot_read(self):
        # a worktree in a container has a .git file that names a repository outside the container
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            (root / ".git").write_text("gitdir: /nonexistent/repository/.git/worktrees/inet\n")
            (root / "showcases/a/doc").mkdir(parents=True)
            (root / "showcases/a/A.anf").write_text('<property name="image_export_filename" value="chart"/>\n')
            os.symlink("../../../media/showcases/a/doc", root / "showcases/a/doc/media")
            entry = {"folder": "media", "read_folder": "doc/media"}
            self.assertEqual(baseline_summary.chart_images_in_work_tree(entry, cwd=folder), {"showcases/a/doc/chart.png"})

    def test_read_scalars_reads_a_quoted_name_and_the_run_attributes(self):
        rows = baseline_summary.read_scalars(sca())
        self.assertEqual(rows[("Net.host", "packets sent")], 10.0)
        self.assertEqual(rows[("Net.host", "rcvdPk:count")], 7.0)
        self.assertEqual(rows[("", "run attribute experiment")], "General")
        self.assertEqual(rows[("", "run attribute measurement")], "")
        self.assertEqual(rows[("", "run attribute replication")], "#0")
        self.assertEqual(len(rows), 5)

    def test_read_scalars_keeps_text_that_is_not_a_number(self):
        rows = baseline_summary.read_scalars('scalar Net.host "state" idle\n')
        self.assertEqual(rows, {("Net.host", "state"): "idle"})

    def test_compare_scalars_sees_nan_on_both_sides_as_no_change(self):
        old = baseline_summary.read_scalars("scalar Net.host x nan\n")
        new = baseline_summary.read_scalars("scalar Net.host x nan\n")
        self.assertEqual(baseline_summary.compare_scalars(old, new), ([], [], []))

    def test_compare_scalars_sees_nan_against_a_number_as_a_change(self):
        old = baseline_summary.read_scalars("scalar Net.host x nan\n")
        new = baseline_summary.read_scalars("scalar Net.host x 1\n")
        changed, added, removed = baseline_summary.compare_scalars(old, new)
        self.assertEqual([key for key, _, _ in changed], [("Net.host", "x")])
        self.assertEqual((added, removed), ([], []))

    def test_compare_scalars_sees_another_text_for_the_same_number_as_no_change(self):
        old = baseline_summary.read_scalars("scalar Net.host x 10\n")
        new = baseline_summary.read_scalars("scalar Net.host x 10.0\n")
        self.assertEqual(baseline_summary.compare_scalars(old, new), ([], [], []))

    def test_lines_of_no_change_are_empty(self):
        self.assertEqual(baseline_summary.lines({"tests": [], "charts": []}), [])

    def test_lines_name_the_added_and_removed_tests_and_the_changed_charts(self):
        moved = {"tests": [("A", "examples/a/x.sca", [], [], []), ("A", "examples/a/y.sca", [], [], []),
                           ("D", "examples/b/z.sca", [], [], [])],
                 "charts": [("A", "showcases/a/doc/c1.png"), ("D", "showcases/a/doc/c2.png"), ("M", "showcases/b/doc/c3.png")]}
        self.assertEqual(baseline_summary.lines(moved), [
            "statistical tests added (2; examples/a 2)", "statistical tests removed (1; examples/b 1)",
            "chart images added (1; showcases/a 1): c1", "chart images removed (1; showcases/a 1): c2",
            "chart images changed (1; showcases/b 1): c3"])

    def test_block_has_the_heading_and_a_line_with_the_reason_of_each_computed_line(self):
        moved = {"tests": [changed_test("examples/a/x.sca", "n")], "charts": [("M", "showcases/a/doc/c.png")]}
        text = "statistical tests changed, large (1; examples/a 1): n"
        block = baseline_summary.block(moved, {text: "the reason"})
        self.assertEqual(block, ["Baseline changes:", f"  {text} — the reason", "  chart images changed (1; showcases/a 1): c — "])

    def test_block_of_no_change_is_the_heading_only(self):
        self.assertEqual(baseline_summary.block({"tests": [], "charts": []}), ["Baseline changes:"])

    def test_read_block_returns_the_texts_and_reasons_up_to_the_empty_line(self):
        message = "Subject\n\nAnalysis.\n\nBaseline changes:\n  text one — reason one\n  text two — reason two\n\nAttribution: commit\n"
        self.assertEqual(baseline_summary.read_block(message), [("text one", "reason one"), ("text two", "reason two")])

    def test_read_block_returns_none_when_the_message_has_no_list(self):
        self.assertIsNone(baseline_summary.read_block("Subject\n\nAnalysis.\n\nAttribution: commit\n"))

    def test_read_block_joins_a_line_with_four_spaces_to_the_reason_before_it(self):
        message = "S\n\nBaseline changes:\n  text one — first part\n      second part\n  text two — reason two\n"
        self.assertEqual(baseline_summary.read_block(message), [("text one", "first part second part"), ("text two", "reason two")])

    def test_read_block_gives_an_empty_reason_for_a_line_with_no_separator(self):
        self.assertEqual(baseline_summary.read_block("S\n\nBaseline changes:\n  text one\n"), [("text one", "")])

    def test_check_block_accepts_the_computed_list_with_reasons(self):
        message = "S\n\n" + "\n".join(["Baseline changes:", "  a — r1", "  b — r2"]) + "\n"
        self.assertEqual(baseline_summary.check_block(message, ["a", "b"]), [])

    def test_check_block_reports_a_missing_list(self):
        self.assertEqual(baseline_summary.check_block("S\n\nbody\n", ["a"]), ["no 'Baseline changes:' list"])

    def test_check_block_reports_a_list_that_is_not_the_computed_list(self):
        message = "S\n\nBaseline changes:\n  a — r1\n  other — r2\n"
        self.assertEqual(baseline_summary.check_block(message, ["a", "b"]), ["a 'Baseline changes:' list that is not the computed list"])

    def test_check_block_reports_the_right_lines_in_another_order(self):
        message = "S\n\nBaseline changes:\n  b — r2\n  a — r1\n"
        self.assertEqual(baseline_summary.check_block(message, ["a", "b"]), ["a 'Baseline changes:' list that is not the computed list"])

    def test_check_block_reports_one_problem_for_each_line_without_a_reason(self):
        message = "S\n\nBaseline changes:\n  a — \n  b — r2\n  c\n"
        self.assertEqual(baseline_summary.check_block(message, ["a", "b", "c"]), ["no reason for: a", "no reason for: c"])

    def test_size_class_large_for_a_change_of_one_percent_or_more(self):
        self.assertEqual(baseline_summary.size_class([(K, 100.0, 101.0)], [], []), "large")
        self.assertEqual(baseline_summary.size_class([(K, 100.0, 50.0)], [], []), "large")

    def test_size_class_large_for_a_change_from_zero(self):
        self.assertEqual(baseline_summary.size_class([(K, 0.0, 1e-12)], [], []), "large")

    def test_size_class_large_for_a_text_value(self):
        self.assertEqual(baseline_summary.size_class([(K, "idle", "busy")], [], []), "large")
        self.assertEqual(baseline_summary.size_class([(K, 1.0, "busy")], [], []), "large")

    def test_size_class_large_for_a_value_that_is_not_finite(self):
        nan, inf = float("nan"), float("inf")
        self.assertEqual(baseline_summary.size_class([(K, nan, 1.0)], [], []), "large")
        self.assertEqual(baseline_summary.size_class([(K, 1.0, inf)], [], []), "large")

    def test_size_class_minor_for_a_change_below_one_percent(self):
        self.assertEqual(baseline_summary.size_class([(K, 100.0, 100.5)], [], []), "minor")
        self.assertEqual(baseline_summary.size_class([(K, 1.0, 1.0 + 1e-8)], [], []), "minor")

    def test_size_class_noise_for_a_change_of_at_most_1e_9(self):
        self.assertEqual(baseline_summary.size_class([(K, 1.0, 1.0 + 1e-10)], [], []), "noise")

    def test_size_class_takes_the_largest_change_of_the_rows(self):
        changed = [(K, 1.0, 1.0 + 1e-10), (("Net.host", "other"), 100.0, 100.5)]
        self.assertEqual(baseline_summary.size_class(changed, [], []), "minor")

    def test_size_class_rows_when_only_rows_are_added_or_removed(self):
        self.assertEqual(baseline_summary.size_class([], [K], []), "rows")
        self.assertEqual(baseline_summary.size_class([], [], [K]), "rows")

    def test_size_class_no_value_when_nothing_changes(self):
        self.assertEqual(baseline_summary.size_class([], [], []), "no value")

    def test_lines_of_changed_tests_are_ordered_by_class_and_then_by_the_number_of_tests(self):
        moved = {"tests": [
            ("M", "examples/a/t1.sca", [], [], []),
            ("M", "examples/a/t2.sca", [], [K], []),
            changed_test("examples/a/t3.sca", "noise", old=1.0, new=1.0 + 1e-10),
            changed_test("examples/a/t4.sca", "minor", old=100.0, new=100.5),
            changed_test("examples/a/t5.sca", "minor", old=100.0, new=100.5),
            changed_test("examples/a/t6.sca", "few"),
            changed_test("examples/a/t7.sca", "many"),
            changed_test("examples/a/t8.sca", "many"),
        ], "charts": []}
        classes = [line.split(" (")[0] for line in baseline_summary.lines(moved)]
        self.assertEqual(classes, ["statistical tests changed, large"] * 2 + ["statistical tests changed, minor",
                         "statistical tests changed, noise", "statistical tests changed, rows", "statistical tests changed, no value"])
        self.assertEqual([line.rsplit(": ", 1)[-1] for line in baseline_summary.lines(moved)[:2]], ["many", "few"])

    def test_a_group_marks_an_added_row_with_a_plus_and_a_removed_row_with_a_minus(self):
        test = ("M", "examples/a/x.sca", [(("Net.host", "x"), 100.0, 200.0)], [("Net.host", "extra")], [("Net.host", "gone")])
        self.assertEqual(baseline_summary.lines({"tests": [test], "charts": []}),
                         ["statistical tests changed, large (1; examples/a 1): +extra, -gone, x"])

    def test_a_group_of_more_than_12_names_orders_its_names_by_the_number_of_tests_that_move_them(self):
        first = changed_test("examples/a/t1.sca", *[f"n{i:02d}" for i in range(13)])
        second = changed_test("examples/a/t2.sca", "n12", "n11", *[f"m{i:02d}" for i in range(11)])
        lines = baseline_summary.lines({"tests": [first, second], "charts": []})
        self.assertEqual(lines, ["statistical tests changed, large (2; examples/a 2): n11, n12, m00, m01, m02, m03 and 18 more"])

    def test_a_group_of_12_names_stays_a_group_of_its_own(self):
        first = changed_test("examples/a/t1.sca", *[f"n{i:02d}" for i in range(12)])
        second = changed_test("examples/a/t2.sca", *[f"n{i:02d}" for i in range(1, 13)])
        self.assertEqual(len(baseline_summary.lines({"tests": [first, second], "charts": []})), 2)

    def test_more_than_10_folders_give_a_count_of_the_other_folders(self):
        tests = [("A", f"examples/f{i:02d}/x.sca", [], [], []) for i in range(12)]
        shown = ", ".join(f"examples/f{i:02d} 1" for i in range(10))
        self.assertEqual(baseline_summary.lines({"tests": tests, "charts": []}),
                         [f"statistical tests added (12; {shown}, 2 other folders)"])

    def test_exactly_10_folders_give_no_count_of_other_folders(self):
        tests = [("A", f"examples/f{i:02d}/x.sca", [], [], []) for i in range(10)]
        self.assertNotIn("other folders", baseline_summary.lines({"tests": tests, "charts": []})[0])

    def test_more_than_6_names_give_a_count_of_more_names(self):
        test = changed_test("examples/a/x.sca", *[f"n{i}" for i in range(8)])
        self.assertEqual(baseline_summary.lines({"tests": [test], "charts": []}),
                         ["statistical tests changed, large (1; examples/a 1): n0, n1, n2, n3, n4, n5 and 2 more"])

    def test_exactly_6_names_give_no_count_of_more_names(self):
        test = changed_test("examples/a/x.sca", *[f"n{i}" for i in range(6)])
        self.assertNotIn("more", baseline_summary.lines({"tests": [test], "charts": []})[0])

    def test_more_than_5_renames_give_a_count_of_more_renames(self):
        renames = [(f"examples/a/o{i}.sca", f"examples/a/n{i}.sca") for i in range(7)]
        text = ", ".join(f"{old} -> {new}" for old, new in renames[:5])
        self.assertEqual(baseline_summary.lines({"tests": [], "charts": [], "renamed tests": renames}),
                         [f"statistical tests renamed (7): {text} and 2 more"])

    def test_lines_name_the_renamed_charts_after_the_other_chart_lines(self):
        moved = {"tests": [], "charts": [("M", "showcases/a/doc/c.png")], "renamed charts": [("a/old.png", "a/new.png")]}
        self.assertEqual(baseline_summary.lines(moved),
                         ["chart images changed (1; showcases/a 1): c", "chart images renamed (1): a/old.png -> a/new.png"])

    def test_chart_images_names_the_images_of_the_charts_of_each_analysis_file(self):
        texts = {
            "showcases/a/A.anf": ANF_TEXT,
            "showcases/b/B.anf": '<property name="image_export_filename" value="x"/>\n',
            "showcases/c/C.anf": "<analysis/>\n",
            "showcases/d/D.anf": '<property name="image_export_filename" value="outside"/>\n',
            "showcases/e/E.anf": '<property name="image_export_filename" value="nolink"/>\n',
        }
        links = {
            "showcases/a/doc/media": "../../../media/showcases/a/doc",
            "showcases/b/doc/media": "../../../media/showcases/b/doc",
            "showcases/c/doc/media": "../../../media/showcases/c/doc",
            "showcases/d/doc/media": "../../../other/showcases/d/doc",
        }
        images = baseline_summary.chart_images(texts, links.get, ENTRY)
        self.assertEqual(images, {PNG_A, PNG_A2, "showcases/b/doc/x.png"})

    def test_result_filter_of_statistical_accepts_only_sca_files(self):
        is_result = baseline_summary.result_filter("statistical")
        self.assertTrue(is_result(SCA_A))
        self.assertFalse(is_result("README.md"))
        self.assertFalse(is_result(PNG_A))

    def test_result_filter_of_chart_accepts_only_the_given_images(self):
        is_result = baseline_summary.result_filter("chart", {PNG_A})
        self.assertTrue(is_result(PNG_A))
        self.assertFalse(is_result(PNG_DOC))

    def test_result_filter_of_another_kind_accepts_each_file_outside_the_attribution_folder(self):
        is_result = baseline_summary.result_filter("other")
        self.assertTrue(is_result("x/y.txt"))
        self.assertFalse(is_result(PATCH))

    def test_analyze_leaves_out_a_file_that_is_not_a_result(self):
        with tempfile.TemporaryDirectory() as folder:
            def git(*args):
                return subprocess.run(["git", *args], cwd=folder, check=True, capture_output=True, text=True).stdout.strip()
            git("init", "-q", "-b", "master")
            for name, value in (("user.email", "test@example.invalid"), ("user.name", "Test"), ("commit.gpgsign", "false")):
                git("config", name, value)
            Path(folder, "a.png").write_bytes(b"old")
            Path(folder, "b.png").write_bytes(b"old")
            git("add", "-A")
            git("commit", "-qm", "first")
            old = git("rev-parse", "HEAD")
            Path(folder, "a.png").write_bytes(b"new")
            Path(folder, "b.png").write_bytes(b"new")
            git("commit", "-qam", "second")
            new = git("rev-parse", "HEAD")
            everything = baseline_summary.analyze(folder, old, new)
            only_a = baseline_summary.analyze(folder, old, new, is_result=lambda path: path == "a.png")
        self.assertEqual(everything["charts"], [("M", "a.png"), ("M", "b.png")])
        self.assertEqual(only_a, {"tests": [], "charts": [("M", "a.png")], "renamed tests": [], "renamed charts": []})


class RenameTest(unittest.TestCase):
    """analyze() with two commits of a small baseline repository."""

    def setUp(self):
        self.tempdir = tempfile.TemporaryDirectory()
        self.folder = self.tempdir.name
        self.git("init", "-q", "-b", "master")
        for name, value in (("user.email", "test@example.invalid"), ("user.name", "Test"), ("commit.gpgsign", "false")):
            self.git("config", name, value)

    def tearDown(self):
        self.tempdir.cleanup()

    def git(self, *args):
        return subprocess.run(["git", *args], cwd=self.folder, check=True, capture_output=True, text=True).stdout.strip()

    def commit(self, files):
        """Commits the files; a content of None removes the file. Returns the hash."""
        for path, content in files.items():
            if content is None:
                self.git("rm", "-q", path)
            else:
                GateTestCase.write(self.folder, path, content)
        self.git("add", "-A")
        self.git("commit", "-q", "-m", "commit")
        return self.git("rev-parse", "HEAD")

    def analyze_between(self, before, after):
        old = self.commit(before)
        new = self.commit(after)
        return baseline_summary.analyze(self.folder, old, new)

    def test_a_test_file_that_moves_to_another_name_with_the_same_rows_is_a_rename(self):
        result = self.analyze_between({"examples/a/x-General-#0.sca": sca()}, {"examples/a/x-General-#0.sca": None, "examples/a/y-General-#0.sca": sca()})
        self.assertEqual(result["renamed tests"], [("examples/a/x-General-#0.sca", "examples/a/y-General-#0.sca")])
        self.assertEqual(result["tests"], [])

    def test_a_test_file_that_differs_only_in_the_run_attribute_experiment_is_a_rename(self):
        renamed = sca().replace("attr experiment General", "attr experiment Wifi")
        result = self.analyze_between({"examples/a/x-General-#0.sca": sca()},
                                      {"examples/a/x-General-#0.sca": None, "examples/a/x-Wifi-#0.sca": renamed})
        self.assertEqual(result["renamed tests"], [("examples/a/x-General-#0.sca", "examples/a/x-Wifi-#0.sca")])
        self.assertEqual(result["tests"], [])
        self.assertEqual(baseline_summary.lines(result),
                         ["statistical tests renamed (1): examples/a/x-General-#0.sca -> examples/a/x-Wifi-#0.sca"])

    def test_an_added_and_a_removed_test_file_with_different_rows_are_no_rename(self):
        result = self.analyze_between({"examples/a/x-General-#0.sca": sca()},
                                      {"examples/a/x-General-#0.sca": None, "examples/a/y-General-#0.sca": sca(sent="11")})
        self.assertEqual(result["renamed tests"], [])
        self.assertEqual([(test[0], test[1]) for test in result["tests"]],
                         [("D", "examples/a/x-General-#0.sca"), ("A", "examples/a/y-General-#0.sca")])

    def test_an_added_and_a_removed_test_file_without_scalar_rows_are_no_rename(self):
        empty = "version 3\nrun General-0-x\nattr experiment General\n"
        result = self.analyze_between({"examples/a/x.sca": empty}, {"examples/a/x.sca": None, "examples/a/y.sca": empty})
        self.assertEqual(result["renamed tests"], [])
        self.assertEqual(len(result["tests"]), 2)

    def test_a_chart_image_that_moves_to_another_name_with_the_same_bytes_is_a_rename(self):
        result = self.analyze_between({"showcases/a/doc/old.png": b"\x89PNG same"},
                                      {"showcases/a/doc/old.png": None, "showcases/a/doc/new.png": b"\x89PNG same"})
        self.assertEqual(result["renamed charts"], [("showcases/a/doc/old.png", "showcases/a/doc/new.png")])
        self.assertEqual(result["charts"], [])
        self.assertEqual(baseline_summary.lines(result), ["chart images renamed (1): showcases/a/doc/old.png -> showcases/a/doc/new.png"])

    def test_an_added_and_a_removed_chart_image_with_different_bytes_are_no_rename(self):
        result = self.analyze_between({"showcases/a/doc/old.png": b"\x89PNG old"},
                                      {"showcases/a/doc/old.png": None, "showcases/a/doc/new.png": b"\x89PNG new"})
        self.assertEqual(result["renamed charts"], [])
        self.assertEqual(result["charts"], [("A", "showcases/a/doc/new.png"), ("D", "showcases/a/doc/old.png")])


if __name__ == "__main__":
    unittest.main()
