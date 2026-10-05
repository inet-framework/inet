#!/usr/bin/env python3

from pathlib import Path
import subprocess
import tempfile
import unittest


HERE = Path(__file__).resolve().parent
CHECKER = HERE / "check-pr-description.sh"
TEMPLATE = HERE.parents[2] / ".github" / "pull_request_template.md"

PROBLEM = ("The node drops every frame after a restart, because the queue keeps a stale owner. "
           "Users of the lifecycle examples see no traffic after the first crash.")


def words(count):
    return " ".join("word" for _ in range(count))


class CheckPrDescriptionTest(unittest.TestCase):
    def check(self, text, *args, cwd=None):
        return subprocess.run(["bash", str(CHECKER), *args], input=text, cwd=cwd,
                              capture_output=True, text=True)

    def test_a_problem_first_description_passes(self):
        result = self.check(f"{PROBLEM}\n\n## Commits\n\n- a: fix: reset the owner\n")
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertIn("PASS: no note.", result.stdout)

    def test_the_bare_template_has_no_opening(self):
        result = self.check(TEMPLATE.read_text(encoding="utf-8"))
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertIn("no opening paragraph", result.stdout)

    def test_evidence_before_the_summary_gives_a_note(self):
        for opening in ("## Validation\n\n", "```sh\nmake\n```\n\n", "| Test | Result |\n| --- | --- |\n\n"):
            with self.subTest(opening=opening):
                result = self.check(f"{opening}{PROBLEM}\n")
                self.assertIn("opens with evidence", result.stdout)

    def test_a_heading_before_the_summary_is_quiet(self):
        result = self.check(f"## Summary\n\n{PROBLEM}\n")
        self.assertIn("PASS: no note.", result.stdout)

    def test_an_opening_of_actions_gives_a_note(self):
        result = self.check("Add the X module and wire it into the node.\n\n## Commits\n")
        self.assertIn('the opening starts with an action and names no problem: "Add the X module', result.stdout)

    def test_an_opening_list_of_changes_gives_a_note(self):
        result = self.check("## Summary\n\n- Model the X element.\n- Derive the Y rates.\n\n## Commits\n")
        self.assertIn("the opening is a list of changes that names no problem", result.stdout)
        result = self.check("## Summary\n\n- Stations fail to associate, because the AP sends no X element.\n")
        self.assertIn("PASS: no note.", result.stdout)

    def test_references_badges_and_rules_are_not_the_opening(self):
        result = self.check(f"Fixes #12.\n\n---\n\n<a href=\"x\"><img src=\"y\"></a>\n\n{PROBLEM}\n")
        self.assertIn("PASS: no note.", result.stdout)
        result = self.check("## Dependency\n\nDepends on #12.\n\nPR #12 is stacked on #11.\n\n---\n\n<p>badge</p>\n")
        self.assertIn("no opening paragraph", result.stdout)

    def test_a_long_description_without_a_problem_up_front_gives_a_note(self):
        result = self.check(f"This pull request has many parts.\n\n{words(1000)}\n")
        self.assertIn("1006 words, and the first 150 words name no problem", result.stdout)
        result = self.check(f"{PROBLEM}\n\n{words(1000)}\n")
        self.assertNotIn("names no problem", result.stdout)

    def test_a_large_series_without_a_problem_up_front_gives_a_note(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)

            def git(*args):
                subprocess.run(["git", *args], cwd=root, check=True, capture_output=True, text=True)

            git("init", "-q")
            git("config", "user.email", "test@example.invalid")
            git("config", "user.name", "Description Test")
            (root / "src").mkdir()
            for index in range(5):
                (root / "src" / "A.cc").write_text(f"int a = {index};\n", encoding="utf-8")
                git("add", ".")
                git("commit", "-qm", f"commit {index}")
            range_ = "HEAD~4..HEAD"
            plain = self.check("The series has five parts.\n\n## Commits\n\n- one\n", "--range", range_, cwd=root)
            self.assertIn("4 commits and 2 source lines, and the first 150 words name no problem", plain.stdout)
            short = self.check("Five parts.\n\n## Motivation\n\nThe queue keeps a stale owner.\n",
                               "--range", range_, cwd=root)
            self.assertIn("PASS: no note.", short.stdout)

    def test_no_description_is_invalid_usage(self):
        result = subprocess.run(["bash", str(CHECKER), "--file", "/nonexistent/description.md"],
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 2)


if __name__ == "__main__":
    unittest.main()
