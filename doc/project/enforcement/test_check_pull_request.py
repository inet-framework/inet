#!/usr/bin/env python3

from pathlib import Path
import subprocess
import tempfile
import unittest


CHECKER = Path(__file__).with_name("check-pull-request.sh").resolve()


class CheckPullRequestTest(unittest.TestCase):
    def setUp(self):
        self.tempdir = tempfile.TemporaryDirectory()
        self.root = Path(self.tempdir.name)
        self.git("init", "-q")
        self.git("config", "user.email", "test@example.invalid")
        self.git("config", "user.name", "Pull Request Test")
        self.write("src/inet/a/A.cc", "int a = 0;\n")
        self.git("add", ".")
        self.git("commit", "-qm", "fixture")
        self.git("branch", "-M", "master")
        self.git("checkout", "-qb", "topic")

    def tearDown(self):
        self.tempdir.cleanup()

    def write(self, relative_path, content):
        path = self.root / relative_path
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")

    def git(self, *args):
        return subprocess.run(["git", *args], cwd=self.root, check=True, capture_output=True, text=True)

    def commit(self, subject, trailer="Change: src | behavior.change | -", value=1):
        self.write("src/inet/a/A.cc", f"int a = {value};\n")
        self.git("add", ".")
        self.git("commit", "-q", "-m", f"{subject}\n\nThe value changes.\n\n{trailer}".rstrip())

    def check(self, *args):
        return subprocess.run(["bash", str(CHECKER), "--base", "master", *args], cwd=self.root,
                              capture_output=True, text=True)

    def test_a_clean_series_passes(self):
        self.commit("src: change: one value")
        result = self.check()
        self.assertEqual(result.returncode, 0, result.stdout)
        for gate in ("check-commits.sh", "check-classification.sh", "check-includes.sh", "check-ned-params.sh",
                     "check-whatsnew.sh"):
            self.assertRegex(result.stdout, rf"{gate} +pass")
        self.assertIn("check-pr-description.sh    not run", result.stdout)

    def test_a_violation_fails(self):
        self.commit("src: change: one value", trailer="")
        result = self.check()
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertRegex(result.stdout, r"check-classification.sh +FAIL +1 violation")

    def test_an_entry_in_whatsnew_fails(self):
        self.write("WHATSNEW", "INET-4.7.x (in development)\n---------------------------\n\n")
        self.git("add", ".")
        self.git("commit", "-qm", "fixture: WHATSNEW")
        self.git("branch", "-f", "master")
        self.write("WHATSNEW", "INET-4.7.x (in development)\n---------------------------\n\n"
                   "Notable bug fixes are the following:\n\n1. A repair\n\n   It is repaired.\n")
        self.commit("src: fix: one value", trailer="Change: src | behavior.change.fix | whatsnew")
        result = self.check()
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertRegex(result.stdout, r"check-whatsnew.sh +FAIL +1 violation")
        self.assertIn('adds the entry "A repair" to WHATSNEW; write it as doc/whatsnew/fixes/src-repair.txt',
                      result.stdout)

    def test_the_series_starts_at_the_merge_base(self):
        self.commit("src: change: one value")
        self.git("checkout", "-q", "master")
        self.write("src/inet/b/B.cc", "int b = 0;\n")
        self.git("add", ".")
        self.git("commit", "-qm", "master moves on, and has no trailer")
        self.git("checkout", "-q", "topic")
        result = self.check()
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertIn("  1 commit(s); the gates are from", result.stdout)

    def test_an_invalid_base_is_an_error(self):
        result = subprocess.run(["bash", str(CHECKER), "--base", "nothing"], cwd=self.root,
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 2, result.stdout)
        self.assertIn("'nothing' is not a commit", result.stderr)

    def test_an_empty_description_gives_a_note(self):
        self.commit("src: change: one value")
        self.write("body.md", "")
        result = self.check("--description-file", str(self.root / "body.md"))
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertRegex(result.stdout, r"check-pr-description.sh +pass +0 violation\(s\), 1 note")

    def test_the_markdown_report_escapes_commit_text(self):
        self.commit("</pre><script>x</script> one value", trailer="")
        report = self.root / "report.md"
        result = self.check("--markdown", str(report))
        self.assertEqual(result.returncode, 1, result.stdout)
        text = report.read_text(encoding="utf-8")
        self.assertIn("&lt;/pre&gt;&lt;script&gt;", text)
        self.assertNotIn("<script>", text)
        self.assertIn("<details open><summary><code>check-classification.sh</code>: FAIL</summary>", text)


if __name__ == "__main__":
    unittest.main()
