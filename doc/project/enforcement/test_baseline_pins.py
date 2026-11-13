#!/usr/bin/env python3
"""Tests of baseline_pins.py: the show and moved commands, the parser, and the rebase command.

Each test builds three temporary git repositories: the baseline repositories "statistics" and "media",
and an INET-like repository with an inet.opp that pins one commit of each baseline. The tool runs as a
child process with the INET repository as the working directory, as in real use.
"""

from pathlib import Path
import os
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))
import baseline_pins  # noqa: E402

TOOL = Path(__file__).with_name("baseline_pins.py").resolve()

OPP = """SimulationProject(
    name=%r,
    test_parameters=%r,
)
"""


def opp_text(pins, name="inet"):
    """The text of inet.opp with the given pins, in the form of the real file."""
    parameters = {}
    for kind, repository, folder in (("statistical", "inet-framework/statistics", "statistics"),
                                     ("chart", "inet-framework/media", "media")):
        baseline = {"repository": repository, "folder": folder}
        if kind in pins:
            baseline["ref"] = pins[kind]
        parameters[kind] = {"baseline": baseline}
    return OPP % (name, parameters)


class PinTestCase(unittest.TestCase):
    """The three repositories, and the helpers that build commits in them and run the tool."""

    def setUp(self):
        self.tempdir = tempfile.TemporaryDirectory()
        root = Path(self.tempdir.name)
        self.stat, self.media, self.inet = root / "statistics", root / "media", root / "inet"
        for repository in (self.stat, self.media, self.inet):
            repository.mkdir()
            self.git(repository, "init", "-q", "-b", "master")
            self.git(repository, "config", "user.email", "test@example.invalid")
            self.git(repository, "config", "user.name", "Pin Test")
            self.git(repository, "config", "commit.gpgsign", "false")
        self.pins = {"statistical": self.baseline_commit(self.stat, "a"), "chart": self.baseline_commit(self.media, "m1")}
        self.commit(self.inet, "fixture", {"inet.opp": opp_text(self.pins), "src/a.c": "int a = 0;\n", "src/b.c": "int b = 0;\n"})
        self.git(self.inet, "checkout", "-qb", "topic")

    def tearDown(self):
        self.tempdir.cleanup()

    @staticmethod
    def git(repository, *args, check=True):
        return subprocess.run(["git", *args], cwd=repository, check=check, capture_output=True, text=True)

    def head(self, repository, revision="HEAD"):
        return self.git(repository, "rev-parse", revision).stdout.strip()

    @staticmethod
    def write(repository, relative_path, content):
        path = Path(repository) / relative_path
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")

    def commit(self, repository, message, files, env=None):
        """Writes the files, commits them with the message and returns the hash."""
        for path, content in files.items():
            self.write(repository, path, content)
        self.git(repository, "add", "--", *files)
        out = subprocess.run(["git", "commit", "-q", "-m", message], cwd=repository, env=dict(os.environ, **(env or {})),
                             capture_output=True, text=True)
        self.assertEqual(out.returncode, 0, out.stderr)
        return self.head(repository)

    def baseline_commit(self, repository, name):
        """Makes a commit in a baseline repository and returns the hash."""
        return self.commit(repository, f"baseline {name}", {f"{name}.txt": f"{name}\n"})

    def move(self, message, pins, files=None, env=None, name="inet"):
        """Makes an INET commit that has the given pins and files."""
        return self.commit(self.inet, message, dict(files or {}, **{"inet.opp": opp_text(pins, name)}), env)

    def run_tool(self, *args, cwd=None):
        return subprocess.run([sys.executable, str(TOOL), *args], cwd=cwd or self.inet, capture_output=True, text=True)

    def rebase(self, upstream="master", *extra):
        """Runs the rebase command with the two baseline clones given by path."""
        return self.run_tool("rebase", upstream, "--baseline-repository", f"statistical={self.stat}",
                             "--baseline-repository", f"chart={self.media}", *extra)

    def describe(self, revision):
        return self.git(self.inet, "log", "-1", "--format=%an|%ae|%aI|%B", revision).stdout.rstrip("\n")

    def raw_message(self, revision):
        return self.git(self.inet, "cat-file", "commit", revision).stdout.split("\n\n", 1)[1]

    def series(self, upstream="master"):
        return self.git(self.inet, "rev-list", "--reverse", f"{upstream}..HEAD").stdout.split()

    def pins_of(self, revision):
        return baseline_pins.read_pins(revision, cwd=self.inet)

    def assert_refused(self, result, old_head):
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertTrue(result.stderr.startswith("error:"), result.stderr)
        self.assertEqual(self.head(self.inet), old_head)


class ShowAndMovedTest(PinTestCase):
    def test_show_prints_kind_repository_ref_and_folder(self):
        result = self.run_tool("show")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout.splitlines(), [
            f"statistical inet-framework/statistics {self.pins['statistical']} statistics",
            f"chart inet-framework/media {self.pins['chart']} media"])

    def test_show_with_commit_reads_that_commit(self):
        b = self.baseline_commit(self.stat, "b")
        self.move("move", dict(self.pins, statistical=b))
        result = self.run_tool("show", "master")
        self.assertIn(self.pins["statistical"], result.stdout)
        self.assertNotIn(b, result.stdout)
        self.assertIn(b, self.run_tool("show").stdout)

    def test_moved_prints_kind_of_changed_pin(self):
        b = self.baseline_commit(self.stat, "b")
        self.move("move", dict(self.pins, statistical=b))
        result = self.run_tool("moved", "HEAD")
        self.assertEqual((result.returncode, result.stdout), (0, "statistical\n"))

    def test_moved_prints_all_changed_kinds(self):
        pins = {"statistical": self.baseline_commit(self.stat, "b"), "chart": self.baseline_commit(self.media, "m2")}
        self.move("move", pins)
        self.assertEqual(self.run_tool("moved", "HEAD").stdout, "chart statistical\n")

    def test_moved_prints_empty_line_for_commit_that_adds_first_pin(self):
        self.move("drop pins", {})
        self.move("add pins", self.pins)
        self.assertEqual(baseline_pins.moved_pins("HEAD", cwd=self.inet),
                         [("chart", None, self.pins["chart"]), ("statistical", None, self.pins["statistical"])])
        result = self.run_tool("moved", "HEAD")
        self.assertEqual((result.returncode, result.stdout), (0, "\n"))

    def test_moved_prints_empty_line_for_commit_without_inet_opp_change(self):
        self.commit(self.inet, "source", {"src/a.c": "int a = 1;\n"})
        result = self.run_tool("moved", "HEAD")
        self.assertEqual((result.returncode, result.stdout), (0, "\n"))

    def test_moved_prints_empty_line_for_root_commit(self):
        result = self.run_tool("moved", "master")
        self.assertEqual((result.returncode, result.stdout), (0, "\n"))


class ParserTest(PinTestCase):
    def test_parse_baselines_reads_entries_of_kinds_with_baseline(self):
        text = opp_text(self.pins)
        self.assertEqual(baseline_pins.parse_baselines(text)["chart"]["ref"], self.pins["chart"])
        self.assertEqual(baseline_pins.parse_baselines('SimulationProject(test_parameters={"x": {}})'), {})
        self.assertEqual(baseline_pins.parse_baselines('SimulationProject(name="inet")'), {})

    def test_parse_baselines_rejects_text_that_is_not_an_expression(self):
        with self.assertRaises(baseline_pins.PinError):
            baseline_pins.parse_baselines("x = 1\ny = 2\n")
        with self.assertRaises(baseline_pins.PinError):
            baseline_pins.parse_baselines("SimulationProject(")

    def test_parse_baselines_rejects_text_that_is_not_one_call(self):
        for text in ("42", "[1, 2]", "name"):
            with self.assertRaises(baseline_pins.PinError, msg=text):
                baseline_pins.parse_baselines(text)

    def test_parse_baselines_rejects_test_parameters_that_is_not_a_literal(self):
        with self.assertRaises(baseline_pins.PinError):
            baseline_pins.parse_baselines('SimulationProject(test_parameters={"statistical": dict(baseline=1)})')

    def test_parse_baselines_rejects_test_parameters_that_is_not_a_dict(self):
        with self.assertRaises(baseline_pins.PinError):
            baseline_pins.parse_baselines("SimulationProject(test_parameters=[1])")

    def test_parse_baselines_rejects_baseline_that_is_not_a_dict(self):
        with self.assertRaises(baseline_pins.PinError):
            baseline_pins.parse_baselines('SimulationProject(test_parameters={"statistical": {"baseline": "x"}})')

    def test_read_baselines_gives_empty_dict_for_commit_without_inet_opp(self):
        self.git(self.inet, "rm", "-q", "inet.opp")
        self.git(self.inet, "commit", "-qm", "remove inet.opp")
        self.assertEqual(baseline_pins.read_baselines("HEAD", cwd=self.inet), {})
        self.assertEqual(baseline_pins.read_pins("HEAD", cwd=self.inet), {})
        self.assertEqual(self.run_tool("show").stdout, "")

    def test_inet_opp_is_never_run(self):
        text = 'SimulationProject(name="inet", test_parameters={"statistical": __import__("os").system("touch X")})\n'
        self.commit(self.inet, "hostile", {"inet.opp": text})
        result = self.run_tool("show")
        self.assertEqual(result.returncode, 2, result.stdout)
        self.assertTrue(result.stderr.startswith("error:"), result.stderr)
        self.assertFalse((self.inet / "X").exists())
        self.assertFalse((Path(self.tempdir.name) / "X").exists())

    def test_show_exits_2_for_inet_opp_that_is_not_python(self):
        self.commit(self.inet, "broken", {"inet.opp": "SimulationProject(\n"})
        result = self.run_tool("show")
        self.assertEqual(result.returncode, 2)
        self.assertTrue(result.stderr.startswith("error:"), result.stderr)


class RebaseTest(PinTestCase):
    def test_amended_baseline_commit_gives_series_commit_the_new_pin(self):
        b = self.baseline_commit(self.stat, "b")
        env = {"GIT_AUTHOR_NAME": "Some Author", "GIT_AUTHOR_EMAIL": "author@example.invalid",
               "GIT_AUTHOR_DATE": "2020-01-02T03:04:05+0100"}
        self.move("src: change a value\n\nA body line.", dict(self.pins, statistical=b), env=env)
        old_head, old_description = self.head(self.inet), self.describe("HEAD")
        self.git(self.stat, "commit", "-q", "--amend", "-m", "baseline b, new message")
        b2 = self.head(self.stat)
        self.assertNotEqual(b, b2)
        result = self.rebase()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertNotEqual(self.head(self.inet), old_head)
        self.assertEqual(self.pins_of("HEAD"), dict(self.pins, statistical=b2))
        self.assertEqual(self.describe("HEAD"), old_description)
        self.assertIn("Some Author|author@example.invalid|2020-01-02T03:04:05+01:00|", self.describe("HEAD"))
        self.assertEqual(self.git(self.inet, "diff", "--name-only", old_head, "HEAD").stdout.split(), ["inet.opp"])
        self.assertIn(b2, (self.inet / "inet.opp").read_text())
        self.assertNotIn(b, (self.inet / "inet.opp").read_text())
        self.assertEqual(self.git(self.inet, "status", "--porcelain").stdout, "")

    def test_rebase_keeps_the_raw_message_of_the_commit(self):
        b = self.baseline_commit(self.stat, "b")
        self.move("subject\n\nBody.", dict(self.pins, statistical=b))
        old = self.raw_message("HEAD")
        self.git(self.stat, "commit", "-q", "--amend", "-m", "b again")
        self.assertEqual(self.rebase().returncode, 0)
        self.assertEqual(self.raw_message("HEAD"), old)

    def test_rebase_prints_commit_and_pin_lines(self):
        b = self.baseline_commit(self.stat, "b")
        self.move("move", dict(self.pins, statistical=b))
        result = self.rebase()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("    statistical pin", result.stdout)
        self.assertIn("Nothing is pushed.", result.stdout)

    def test_base_that_moved_the_pin_gives_series_a_pin_on_top_of_it(self):
        b = self.baseline_commit(self.stat, "b")
        self.git(self.stat, "checkout", "-qb", "other", self.pins["statistical"])
        a2 = self.baseline_commit(self.stat, "a2")
        self.git(self.stat, "checkout", "-q", "master")
        self.git(self.stat, "rebase", "-q", "other")
        b_new = self.head(self.stat)
        self.assertNotEqual(b, b_new)
        self.move("topic: move", dict(self.pins, statistical=b), {"src/a.c": "int a = 1;\n"})
        self.git(self.inet, "checkout", "-q", "master")
        self.move("master: move", dict(self.pins, statistical=a2), {"src/b.c": "int b = 2;\n"})
        self.git(self.inet, "checkout", "-q", "topic")
        # the premise: git alone cannot combine the two commits
        merge = self.git(self.inet, "merge-tree", "--write-tree", "master", "topic", check=False)
        self.assertEqual(merge.returncode, 1)
        result = self.rebase()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(self.series(), [self.head(self.inet)])
        self.assertEqual(self.head(self.inet, "HEAD~"), self.head(self.inet, "master"))
        self.assertEqual(self.pins_of("HEAD"), dict(self.pins, statistical=b_new))
        self.assertEqual((self.inet / "src/a.c").read_text(), "int a = 1;\n")
        self.assertEqual((self.inet / "src/b.c").read_text(), "int b = 2;\n")
        self.assertEqual(self.git(self.stat, "rev-parse", f"{b_new}~").stdout.strip(), a2)

    def test_two_moves_of_one_kind_get_two_baseline_commits_in_order(self):
        b, c = self.baseline_commit(self.stat, "b"), self.baseline_commit(self.stat, "c")
        self.move("first move", dict(self.pins, statistical=b))
        self.commit(self.inet, "no move", {"src/a.c": "int a = 1;\n"})
        self.move("second move", dict(self.pins, statistical=c))
        self.commit(self.inet, "no move again", {"src/b.c": "int b = 1;\n"})
        old = self.series()
        old_descriptions = [self.describe(commit) for commit in old]
        self.git(self.stat, "checkout", "-q", "-b", "new", self.pins["statistical"])
        b2, c2 = self.baseline_commit(self.stat, "b2"), self.baseline_commit(self.stat, "c2")
        self.git(self.stat, "checkout", "-q", "master")
        result = self.rebase("master", "--baseline", "statistical=new")
        self.assertEqual(result.returncode, 0, result.stderr)
        new = self.series()
        self.assertEqual(len(new), 4)
        self.assertEqual([self.describe(commit) for commit in new], old_descriptions)
        self.assertEqual([self.pins_of(commit)["statistical"] for commit in new], [b2, b2, c2, c2])
        self.assertEqual(self.pins_of("HEAD")["chart"], self.pins["chart"])
        self.assertEqual(self.git(self.inet, "diff", "--name-only", old[1], new[1]).stdout.split(), ["inet.opp"])
        self.assertEqual(self.git(self.inet, "diff", "--name-only", new[0], new[1]).stdout.split(), ["src/a.c"])

    def test_both_kinds_moving_in_one_commit_get_their_own_baseline_commits(self):
        b, m2 = self.baseline_commit(self.stat, "b"), self.baseline_commit(self.media, "m2")
        self.move("move both", {"statistical": b, "chart": m2})
        self.git(self.stat, "commit", "-q", "--amend", "-m", "b again")
        self.git(self.media, "commit", "-q", "--amend", "-m", "m2 again")
        result = self.rebase()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(self.pins_of("HEAD"), {"statistical": self.head(self.stat), "chart": self.head(self.media)})
        self.assertNotIn(b, (self.inet / "inet.opp").read_text())
        self.assertNotIn(m2, (self.inet / "inet.opp").read_text())

    def test_baseline_option_selects_branch_that_is_not_the_head_of_the_clone(self):
        b = self.baseline_commit(self.stat, "b")
        self.move("move", dict(self.pins, statistical=b))
        self.git(self.stat, "checkout", "-q", "-b", "wanted", self.pins["statistical"])
        wanted = self.baseline_commit(self.stat, "wanted")
        self.git(self.stat, "checkout", "-q", "master")
        self.assertEqual(self.head(self.stat), b)
        result = self.rebase("master", "--baseline", "statistical=wanted")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(self.pins_of("HEAD")["statistical"], wanted)
        self.assertEqual(self.head(self.stat), b)

    def test_other_change_of_inet_opp_stays_with_the_new_pin(self):
        b = self.baseline_commit(self.stat, "b")
        self.move("rename and move", dict(self.pins, statistical=b), name="inet-renamed")
        self.git(self.stat, "commit", "-q", "--amend", "-m", "b again")
        result = self.rebase()
        self.assertEqual(result.returncode, 0, result.stderr)
        text = (self.inet / "inet.opp").read_text()
        self.assertIn("name='inet-renamed'", text)
        self.assertEqual(self.pins_of("HEAD")["statistical"], self.head(self.stat))
        self.assertEqual(baseline_pins.parse_baselines(text)["statistical"]["ref"], self.head(self.stat))

    def test_untracked_files_stay_and_default_folder_is_the_clone(self):
        b = self.baseline_commit(self.stat, "b")
        self.move("move", dict(self.pins, statistical=b))
        self.git(self.stat, "commit", "-q", "--amend", "-m", "b again")
        self.write(self.inet, "notes.txt", "mine\n")
        self.git(self.inet, "clone", "-q", str(self.stat), "statistics")
        result = self.run_tool("rebase", "master")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual((self.inet / "notes.txt").read_text(), "mine\n")
        self.assertTrue((self.inet / "statistics/.git").exists())
        self.assertEqual(self.pins_of("HEAD")["statistical"], self.head(self.stat))


class RebaseErrorTest(PinTestCase):
    def setUp(self):
        super().setUp()
        self.b = self.baseline_commit(self.stat, "b")

    def test_baseline_branch_that_does_not_stand_on_pin_of_new_base(self):
        self.git(self.stat, "checkout", "-q", "-b", "other", self.pins["statistical"])
        a2 = self.baseline_commit(self.stat, "a2")
        self.git(self.stat, "checkout", "-q", "master")
        self.move("topic: move", dict(self.pins, statistical=self.b))
        self.git(self.inet, "checkout", "-q", "master")
        self.move("master: move", dict(self.pins, statistical=a2))
        self.git(self.inet, "checkout", "-q", "topic")
        old_head = self.head(self.inet)
        result = self.rebase()
        self.assert_refused(result, old_head)
        self.assertIn("does not stand on the pin", result.stderr)

    def test_one_move_and_two_baseline_commits(self):
        self.baseline_commit(self.stat, "c")
        self.move("move", dict(self.pins, statistical=self.b))
        old_head = self.head(self.inet)
        result = self.rebase()
        self.assert_refused(result, old_head)
        self.assertIn("1 time(s)", result.stderr)

    def test_two_moves_and_one_baseline_commit(self):
        c = self.baseline_commit(self.stat, "c")
        self.move("first", dict(self.pins, statistical=self.b))
        self.move("second", dict(self.pins, statistical=c))
        self.git(self.stat, "reset", "-q", "--hard", self.b)
        old_head = self.head(self.inet)
        result = self.rebase()
        self.assert_refused(result, old_head)
        self.assertIn("2 time(s)", result.stderr)

    def test_change_in_tracked_file_of_work_tree(self):
        self.move("move", dict(self.pins, statistical=self.b))
        old_head = self.head(self.inet)
        self.write(self.inet, "src/a.c", "int a = 9;\n")
        self.assert_refused(self.rebase(), old_head)
        self.assertEqual((self.inet / "src/a.c").read_text(), "int a = 9;\n")

    def test_series_commit_that_conflicts_with_new_base_in_source_file(self):
        self.git(self.stat, "reset", "-q", "--hard", self.pins["statistical"])
        self.commit(self.inet, "topic: edit", {"src/a.c": "int a = 1;\n"})
        self.git(self.inet, "checkout", "-q", "master")
        self.commit(self.inet, "master: edit", {"src/a.c": "int a = 2;\n"})
        self.git(self.inet, "checkout", "-q", "topic")
        old_head = self.head(self.inet)
        result = self.rebase()
        self.assert_refused(result, old_head)
        self.assertIn("without a conflict", result.stderr)

    def test_series_with_merge_commit(self):
        self.git(self.inet, "checkout", "-q", "-b", "side", "master")
        self.commit(self.inet, "side", {"src/b.c": "int b = 1;\n"})
        self.git(self.inet, "checkout", "-q", "topic")
        self.commit(self.inet, "topic", {"src/a.c": "int a = 1;\n"})
        self.git(self.inet, "merge", "-q", "--no-ff", "-m", "merge", "side")
        old_head = self.head(self.inet)
        result = self.rebase()
        self.assert_refused(result, old_head)
        self.assertIn("merge commit", result.stderr)

    def test_empty_series(self):
        self.git(self.inet, "checkout", "-q", "master")
        old_head = self.head(self.inet)
        result = self.rebase()
        self.assert_refused(result, old_head)
        self.assertIn("no commit", result.stderr)

    def test_missing_baseline_clone_for_moved_pin(self):
        self.move("move", dict(self.pins, statistical=self.b))
        old_head = self.head(self.inet)
        result = self.run_tool("rebase", "master")
        self.assert_refused(result, old_head)
        self.assertIn("no clone", result.stderr)

    def test_missing_baseline_clone_for_unmoved_pin_is_fine(self):
        self.commit(self.inet, "source", {"src/a.c": "int a = 1;\n"})
        result = self.run_tool("rebase", "master")
        self.assertEqual(result.returncode, 0, result.stderr)


class UsageTest(PinTestCase):
    def test_rebase_without_upstream_exits_2(self):
        result = self.run_tool("rebase")
        self.assertEqual(result.returncode, 2)
        self.assertIn("usage:", result.stderr)

    def test_baseline_without_equals_sign_exits_2(self):
        old_head = self.head(self.inet)
        result = self.run_tool("rebase", "master", "--baseline", "statistical")
        self.assertEqual(result.returncode, 2)
        self.assertIn("KIND=REVISION", result.stderr)
        self.assertEqual(self.head(self.inet), old_head)

    def test_baseline_repository_without_equals_sign_exits_2(self):
        result = self.run_tool("rebase", "master", "--baseline-repository", "statistics")
        self.assertEqual(result.returncode, 2)

    def test_unknown_command_exits_2(self):
        self.assertEqual(self.run_tool("frobnicate").returncode, 2)


if __name__ == "__main__":
    unittest.main()
