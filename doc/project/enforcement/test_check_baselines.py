#!/usr/bin/env python3
"""Tests of the baseline gate (check-baselines.sh) and of the summary of baseline_summary.py.

Each gate test builds three temporary git repositories: the baseline repositories "statistics" and
"media", and an INET-like repository with an inet.opp that pins one commit of each baseline.
"""

from pathlib import Path
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
CHANGE = "Change: src | behavior.change | "
SCA_A = "examples/a/omnetpp.ini-General-#0.sca"
SCA_B = "examples/b/omnetpp.ini-General-#0.sca"
PNG_A = "showcases/a/doc/chart.png"
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
        self.write(self.media, "showcases/b/doc/chart.png", b"\x89PNG other")
        self.pins = {}
        for kind, repository in (("statistical", self.stat), ("chart", self.media)):
            self.git(repository, "add", "-A")
            self.git(repository, "commit", "-qm", "first baseline")
            self.pins[kind] = self.head(repository)
        self.first = dict(self.pins)
        self.write(self.inet, "src/inet/a/A.cc", "int a = 0;\n")
        self.write(self.inet, "inet.opp", opp_text(self.pins))
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

    def record(self, repository, changes, inet_subject=SUBJECT, attribution="commit", analysis=ANALYSIS,
               block=None, subject="Move the results"):
        """Makes a baseline commit with its record. A change of None removes the file. inet_subject and
        attribution of None leave the trailer out. A block of "none" leaves the block out, a function
        edits the computed lines. Returns the hash."""
        for path, content in changes.items():
            if content is None:
                self.git(repository, "rm", "-q", path)
            else:
                self.write(repository, path, content)
        self.git(repository, "add", "-A")
        lines = baseline_summary.block(baseline_summary.analyze(str(repository)))
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

    def inet_commit(self, subject=SUBJECT, source=0, trailer=CHANGE + "-", pins=None, opp=None):
        """Makes an INET commit. It appends the given number of lines to A.cc, and writes inet.opp."""
        self.pins.update(pins or {})
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
             media_record=None):
        """One INET commit that moves the pin of each baseline with changes: the baseline commit with
        its record, then the INET commit."""
        pins = {}
        for kind, repository, changes, options in (("statistical", self.stat, stat, stat_record),
                                                    ("chart", self.media, media, media_record)):
            if changes is not None:
                pins[kind] = self.record(repository, changes, **{"inet_subject": subject, **(options or {})})
        if trailer is None:
            trailer = CHANGE + " ".join(sorted(pins))
        self.inet_commit(subject, source, trailer, pins)

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
        block = baseline_summary.block(baseline_summary.analyze(str(self.stat)))
        self.assertIn("  rows of the changed tests: 1 changed, 0 added, 0 removed", block)
        self.assertNotIn("  - packets sent: 1 changed", block)
        self.git(self.stat, "reset", "-q", "--hard")
        self.move(stat=changes)
        result = self.assert_passes()
        self.assertIn("rows of the changed tests: 1 changed, 0 added, 0 removed", result.stdout)
        self.assertNotIn("- packets sent", result.stdout)


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
        self.assert_fails("moves the statistical pin over 2 baseline commits; one cause commit has one baseline commit")

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

    def test_no_moved_results_block_fails(self):
        self.move(stat={SCA_A: sca(sent="11")}, stat_record={"block": "none"})
        self.assert_fails("has no 'Moved results:' block")

    def test_a_block_with_a_changed_number_fails(self):
        def wrong(lines):
            return [line.replace("1 changed, 0 added", "2 changed, 0 added") for line in lines]
        self.move(stat={SCA_A: sca(sent="11")}, stat_record={"block": wrong})
        self.assert_fails("has a 'Moved results:' block that is not the computed block")

    def test_an_analysis_of_fewer_than_20_words_fails(self):
        self.move(stat={SCA_A: sca(sent="11")}, stat_record={"analysis": "It sends more packets now."})
        self.assert_fails("has no analysis: 5 words of prose")

    def test_a_line_of_the_analysis_that_starts_with_a_word_and_a_colon_counts(self):
        analysis = ("Note: each host now sends one more packet, so the number of sent packets rises by one in "
                    "this test and the other rows stay the same.")
        self.move(stat={SCA_A: sca(sent="11")}, stat_record={"analysis": analysis})
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
        self.assert_fails("changes attribution/other.patch; a baseline commit changes results")


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
        self.move(stat={SCA_A: sca(sent="11")}, media={"showcases/c/doc/chart.png": b"\x89PNG added"},
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
        self.assertIn("moves its tests in 2 ways", result.stdout)
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
        self.assertIn("rows of the changed tests: 1 changed, 0 added, 0 removed", result.stdout)


class BaselineSummaryTest(unittest.TestCase):
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

    def test_find_block_returns_the_lines_up_to_the_blank_line(self):
        message = "Subject\n\nAnalysis.\n\nMoved results:\n  tests: 1 changed\n  folders: a 1\n\nAttribution: commit\n"
        self.assertEqual(baseline_summary.find_block(message), ["Moved results:", "  tests: 1 changed", "  folders: a 1"])

    def test_find_block_returns_none_when_the_message_has_no_block(self):
        self.assertIsNone(baseline_summary.find_block("Subject\n\nAnalysis.\n\nAttribution: commit\n"))

    def test_block_sums_the_names_beyond_the_20th_in_an_other_names_line(self):
        changed = [(("Net.host", f"n{i:02d}"), 1.0, 2.0) for i in range(25)]
        lines = baseline_summary.block({"tests": [("M", "examples/a/x.sca", changed, [], [])], "charts": []})
        self.assertEqual(lines[0], "Moved results:")
        self.assertIn("  - 5 other names: 5 rows", lines)
        self.assertEqual(sum(1 for line in lines if line.startswith("  - n")), 20)
        self.assertIn("  - 1 test(s): more than 12 names each", lines)
        self.assertIn("  - n00: 1 changed, +100 % to +100 %", lines)

    def test_block_of_no_change_says_that_no_result_moves(self):
        self.assertEqual(baseline_summary.block({"tests": [], "charts": []}), ["Moved results:", "  no result moves"])


if __name__ == "__main__":
    unittest.main()
