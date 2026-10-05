#!/usr/bin/env python3

from pathlib import Path
import subprocess
import tempfile
import unittest


CHECKER = Path(__file__).with_name("check-commits.sh").resolve()

ROW = "/examples/{name}/, -f omnetpp.ini -c {name} -r 0, 10s, {tplx}/tplx;{tnl}/~tNl;1111-1111/~tND, PASS, wireless\n"


def rows(**fingerprints):
    """One fingerprint row per name; each value is (tplx, ~tNl)."""
    return "".join(ROW.format(name=name, tplx=tplx, tnl=tnl) for name, (tplx, tnl) in fingerprints.items())


class CheckCommitsTest(unittest.TestCase):
    def setUp(self):
        self.tempdir = tempfile.TemporaryDirectory()
        self.root = Path(self.tempdir.name)
        self.git("init", "-q")
        self.git("config", "user.email", "test@example.invalid")
        self.git("config", "user.name", "Commit Test")
        self.write("src/inet/a/A.cc", "int a = 0;\n")
        self.write("tests/fingerprint/examples.csv", rows(one=("aaaa-0001", "bbbb-0001"), two=("aaaa-0002", "bbbb-0002")))
        self.git("add", ".")
        self.git("commit", "-qm", "fixture")
        self.base = self.git("rev-parse", "HEAD").stdout.strip()

    def tearDown(self):
        self.tempdir.cleanup()

    def write(self, relative_path, content):
        path = self.root / relative_path
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")

    def git(self, *args):
        return subprocess.run(["git", *args], cwd=self.root, check=True, capture_output=True, text=True)

    def commit(self, message, files):
        for relative_path, content in files.items():
            self.write(relative_path, content)
        self.git("add", ".")
        self.git("commit", "-q", "-m", message)

    def check(self):
        return subprocess.run(["bash", str(CHECKER), f"{self.base}..HEAD"], cwd=self.root,
                              capture_output=True, text=True)

    def test_rows_that_move_in_two_ways_give_a_note(self):
        self.commit("a: change: move two rows\n\nThe rows move for two reasons.", {
            "src/inet/a/A.cc": "int a = 1;\n",
            "tests/fingerprint/examples.csv": rows(one=("cccc-0001", "bbbb-0001"), two=("cccc-0002", "dddd-0002")),
        })
        result = self.check()
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertIn("moves fingerprint rows in 2 ways", result.stdout)
        self.assertIn("1 row changes tplx; 1 row changes tplx,~tNl", result.stdout)

    def test_rows_that_move_in_one_way_are_quiet(self):
        self.commit("a: change: move two rows\n\nThe rows move for one reason.", {
            "src/inet/a/A.cc": "int a = 1;\n",
            "tests/fingerprint/examples.csv": rows(one=("cccc-0001", "bbbb-0001"), two=("cccc-0002", "bbbb-0002")),
        })
        result = self.check()
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertNotIn("moves fingerprint rows", result.stdout)

    def test_a_commit_above_400_source_lines_gives_a_size_note(self):
        self.commit("a: add: a large module\n\nThe module is large.", {
            "src/inet/a/Big.cc": "".join(f"int b{i} = {i};\n" for i in range(401)),
        })
        result = self.check()
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertIn("changes 401 source lines, above 400", result.stdout)

    def test_a_commit_at_the_limit_is_quiet(self):
        self.commit("a: add: a module at the limit\n\nThe module is at the limit.", {
            "src/inet/a/Big.cc": "".join(f"int b{i} = {i};\n" for i in range(400)),
        })
        result = self.check()
        self.assertNotIn("source lines, above 400", result.stdout)

    def test_mechanical_and_generated_lines_do_not_count(self):
        self.commit("a: name: rename a large module\n\nThe rename is mechanical.\n\n"
                    "Change: src | name | -", {
            "src/inet/a/Big.cc": "".join(f"int b{i} = {i};\n" for i in range(500)),
        })
        self.commit("a: add: a generated module\n\nThe module is generated.", {
            "src/inet/a/Big_m.cc": "".join(f"int g{i} = {i};\n" for i in range(500)),
        })
        result = self.check()
        self.assertNotIn("source lines, above 400", result.stdout)

    def words(self, count):
        return " ".join("word" for _ in range(count))

    def test_a_long_first_paragraph_gives_a_summary_note(self):
        self.commit(f"a: change: one value\n\n{self.words(121)}\n\nChange: src | behavior.change | -", {
            "src/inet/a/A.cc": "int a = 1;\n",
        })
        result = self.check()
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertIn("starts its body with 121 words, above 120", result.stdout)
        self.assertNotIn("has a body of", result.stdout)

    def test_a_long_body_gives_a_length_note(self):
        paragraphs = "\n\n".join(self.words(100) for _ in range(3))
        self.commit(f"a: change: one value\n\n{paragraphs}\n\n{self.words(1)}\n\n"
                    "Change: src | behavior.change | -", {
            "src/inet/a/A.cc": "int a = 1;\n",
        })
        result = self.check()
        self.assertIn("has a body of 301 words, above 300", result.stdout)
        self.assertNotIn("starts its body with", result.stdout)

    def test_a_short_summary_is_quiet(self):
        self.commit(f"a: change: one value\n\n{self.words(120)}\n\n{self.words(180)}\n\n"
                    "Plan: plan/pending/a.md\nChange: src | behavior.change | -", {
            "src/inet/a/A.cc": "int a = 1;\n",
        })
        result = self.check()
        self.assertNotIn("starts its body with", result.stdout)
        self.assertNotIn("has a body of", result.stdout)

    def test_revision_history_and_a_test_log_give_notes(self):
        self.commit("a: change: one value\n\nThe value was wrong.\n\n"
                    "Audit correction validation: the earlier revision failed.\n\n"
                    "Validation: debug and release builds pass.", {
            "src/inet/a/A.cc": "int a = 1;\n",
        })
        result = self.check()
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertIn("says 'Audit correction': describe the final change", result.stdout)
        self.assertIn("has a 'Validation:' paragraph", result.stdout)

    def test_a_message_about_the_final_change_is_quiet(self):
        self.commit("a: fix: one value\n\nThe value was wrong. The regression test fails against "
                    "the old code, and the validation of the input stays as it is.", {
            "src/inet/a/A.cc": "int a = 1;\n",
        })
        result = self.check()
        self.assertNotIn("describe the final change", result.stdout)
        self.assertNotIn("paragraph: test logs", result.stdout)

    def test_a_body_that_opens_with_actions_gives_a_note(self):
        self.commit("a: add: a module\n\nAdd the module and wire it into the node.\n\n"
                    "Change: src | behavior.add | -", {"src/inet/a/A.cc": "int a = 1;\n"})
        result = self.check()
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertIn('opens with an action and names no problem — which problem does it solve? '
                      '"Add the module and wire it into the node."', result.stdout)

    def test_an_action_with_its_reason_is_quiet(self):
        self.commit("a: add: a module\n\nAdd the module, because the node cannot route without it.\n\n"
                    "Change: src | behavior.add | -", {"src/inet/a/A.cc": "int a = 1;\n"})
        self.assertNotIn("opens with an action", self.check().stdout)

    def test_a_problem_first_body_is_quiet(self):
        self.commit("a: fix: one value\n\nThe node drops every frame after a restart. Reset the value.\n\n"
                    "Change: src | behavior.change.fix | -", {"src/inet/a/A.cc": "int a = 1;\n"})
        self.assertNotIn("opens with an action", self.check().stdout)

    def test_a_mechanical_body_is_quiet(self):
        self.commit("a: name: rename a value\n\nRename the value to its role.\n\n"
                    "Change: src | name | -", {"src/inet/a/A.cc": "int a = 1;\n"})
        self.assertNotIn("opens with an action", self.check().stdout)


if __name__ == "__main__":
    unittest.main()
