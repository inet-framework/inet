#!/usr/bin/env python3
"""Tests of the check of the description of a pull request (PR-REQ-BASELINES): each commit that changes a
baseline is named in the section "Baseline changes", with a summary.
"""

from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import check_baselines  # noqa: E402
import test_check_baselines as base  # noqa: E402
import test_fingerprint_changes as fp  # noqa: E402

FIRST = "src: change: first value"
SECOND = "src: change: second value"
PULL_REQUEST_CHECKER = HERE / "check-pull-request.sh"


class DescriptionSectionTest(unittest.TestCase):
    def test_the_heading_may_have_the_forms_of_markdown(self):
        for heading in ("Baseline changes", "## Baseline changes", "Baseline changes:", "**Baseline changes**",
                        "### Baseline Changes:", "  baseline changes  "):
            with self.subTest(heading=heading):
                self.assertEqual(check_baselines.description_section(f"Intro\n\n{heading}\nthe text\n"), "the text")

    def test_the_section_ends_at_the_next_line_that_starts_with_a_hash(self):
        text = "## Baseline changes\n- a: one\n- b: two\n\n## Tests\nthe tests\n"
        self.assertEqual(check_baselines.description_section(text), "- a: one\n- b: two\n")

    def test_the_section_ends_at_the_end_of_the_description(self):
        self.assertEqual(check_baselines.description_section("Text\n\nBaseline changes\nline 1\nline 2"), "line 1\nline 2")

    def test_a_description_with_no_such_heading_gives_none(self):
        self.assertIsNone(check_baselines.description_section("## Summary\nBaseline changes are many.\n"))
        self.assertIsNone(check_baselines.description_section(""))

    def test_a_heading_with_more_text_is_no_heading(self):
        self.assertIsNone(check_baselines.description_section("## Baseline changes of this series\ntext\n"))


    def test_a_section_in_an_html_comment_does_not_count(self):
        # the template of the pull request shows an example of the section in its comment
        template = "<!--\nFor example:\n\n## Baseline changes\n\n- x: y\n  z\n-->\nThe summary.\n"
        self.assertIsNone(check_baselines.description_section(template))
        self.assertEqual(check_baselines.description_section(template + "\n## Baseline changes\n- a: b\n  c\n"), "- a: b\n  c")


class DescriptionGateTest(fp.FingerprintGateTestCase):
    def two_steps(self):
        self.seed([fp.entry("examples/a")])
        self.store_commit([fp.entry("examples/a"), fp.entry("examples/b")], subject=FIRST, source=1)
        self.store_commit([fp.entry("examples/a"), fp.entry("examples/b"), fp.entry("examples/c")], subject=SECOND, source=1)

    def one_step(self):
        self.seed([fp.entry("examples/a")])
        self.store_commit([fp.entry("examples/a"), fp.entry("examples/b")], subject=FIRST, source=1)

    def with_description(self, text):
        file = Path(self.tempdir.name) / "description.md"
        file.write_text(text, encoding="utf-8")
        return self.fingerprint_gate("--description-file", str(file))

    def assert_description_fails(self, text, description):
        result = self.with_description(description)
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn(text, result.stdout)
        return result

    def test_a_series_with_no_step_needs_no_description(self):
        self.seed([fp.entry("examples/a")])
        self.git(self.inet, "commit", "-q", "--allow-empty", "-m", f"{base.SUBJECT}\n\nThe body.\n\n{fp.FINGERPRINT_CHANGE}")
        result = self.with_description("Nothing about baselines.\n")
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertIn("no commit changes a baseline", result.stdout)

    def test_steps_and_no_description_give_a_note_only(self):
        self.one_step()
        result = self.fingerprint_gate()
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertIn("note: no description given", result.stdout)

    def test_a_description_with_no_section_fails(self):
        self.one_step()
        self.assert_description_fails("the description has no section 'Baseline changes', and 1 commit(s) change baselines",
                                      f"## Summary\n{FIRST}: adds a run.\n")

    def test_a_section_that_misses_the_subject_of_a_step_fails(self):
        self.two_steps()
        result = self.assert_description_fails(f"does not name the commit '{SECOND}'",
                                               f"## Baseline changes\n- {FIRST}: adds the run examples/b.\n")
        self.assertNotIn(f"does not name the commit '{FIRST}'", result.stdout)

    def test_a_subject_outside_the_section_does_not_count(self):
        self.one_step()
        self.assert_description_fails(f"does not name the commit '{FIRST}'",
                                      f"## Baseline changes\nSome text.\n\n## Commits\n- {FIRST}: adds a run.\n")

    def test_a_subject_with_no_summary_fails(self):
        self.two_steps()
        result = self.assert_description_fails(f"names the commit '{FIRST}' with no summary",
                                               f"## Baseline changes\n- {FIRST}\n- {SECOND}: adds the run examples/c.\n")
        self.assertNotIn(f"names the commit '{SECOND}' with no summary", result.stdout)

    def test_a_subject_with_only_punctuation_after_it_has_no_summary(self):
        self.one_step()
        self.assert_description_fails("with no summary", f"## Baseline changes\n- `{FIRST}` —\n")

    def test_two_steps_with_summaries_pass(self):
        self.two_steps()
        result = self.with_description(f"## Baseline changes\n- {FIRST}: adds the run examples/b.\n"
                                       f"- {SECOND}: adds the run examples/c.\n\n## Tests\nAll pass.\n")
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertEqual(self.violations(result), [])

    def test_a_summary_may_be_on_the_lines_after_the_subject(self):
        self.two_steps()
        result = self.with_description(f"Baseline changes:\n\n{FIRST}\n\n  The run examples/b is new.\n\n{SECOND}\n\n"
                                       "  The run examples/c is new.\n")
        self.assertEqual(result.returncode, 0, result.stdout)

    def test_a_step_that_moves_a_pin_needs_its_subject_too(self):
        self.seed([fp.entry("examples/a")])
        pin = self.record(self.stat, {base.SCA_A: base.sca(sent="11")}, inet_subject=FIRST)
        self.write(self.inet, "inet.opp", base.opp_text({**self.pins, "statistical": pin}))
        self.pins["statistical"] = pin
        self.append_source(1)
        self.git(self.inet, "add", "-A")
        self.git(self.inet, "commit", "-qm", f"{FIRST}\n\nThe body.\n\n{base.CHANGE}statistical")
        self.assert_description_fails(f"does not name the commit '{FIRST}'", "## Baseline changes\nNothing here.\n")
        self.assertEqual(self.with_description(f"## Baseline changes\n{FIRST}: the host sends more packets.\n").returncode, 0)


class PullRequestCheckerTest(unittest.TestCase):
    def setUp(self):
        self.tempdir = tempfile.TemporaryDirectory()
        self.root = Path(self.tempdir.name) / "repository"
        self.root.mkdir()
        self.git("init", "-q")
        self.git("config", "user.email", "test@example.invalid")
        self.git("config", "user.name", "Pull Request Test")
        self.git("config", "commit.gpgsign", "false")
        self.write("src/inet/a/A.cc", "int a = 0;\n")
        self.write(fp.STORE, "[]")
        self.git("add", ".")
        self.git("commit", "-qm", "fixture")
        self.git("branch", "-M", "master")
        self.git("checkout", "-qb", "topic")
        self.write("src/inet/a/A.cc", "int a = 1;\n")
        self.write(fp.STORE, '[{"working_directory": "examples/a", "ini_file": "omnetpp.ini", "config": "General", '
                   '"run_number": 0, "sim_time_limit": "10s", "test_result": "PASS", "ingredients": "tplx", '
                   '"fingerprint": "1234-5678", "timestamp": 1.0, "itervars": ""}]')
        self.git("add", ".")
        message = (f"{FIRST}\n\nThe value changes.\n\nBaseline changes:\n  fingerprint runs added (1; examples/a 1) — "
                   f"the run is new\n\n{fp.FINGERPRINT_CHANGE}")
        self.git("commit", "-q", "-m", message)

    def tearDown(self):
        self.tempdir.cleanup()

    def write(self, relative_path, content):
        path = self.root / relative_path
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")

    def git(self, *args):
        return subprocess.run(["git", *args], cwd=self.root, check=True, capture_output=True, text=True)

    def check(self, description):
        file = Path(self.tempdir.name) / "description.md"
        file.write_text(description, encoding="utf-8")
        return subprocess.run(["bash", str(PULL_REQUEST_CHECKER), "--base", "master", "--head", "topic",
                               "--description-file", str(file)], cwd=self.root, capture_output=True, text=True)

    def test_the_description_reaches_the_baseline_gate_and_a_missing_section_is_a_violation(self):
        result = self.check("A description with no such section.\n")
        self.assertRegex(result.stdout, r"check-baselines.sh +FAIL +1 violation")
        self.assertIn("the description has no section 'Baseline changes'", result.stdout)

    def test_the_description_reaches_the_baseline_gate_and_a_section_with_the_step_passes(self):
        result = self.check(f"## Baseline changes\n- {FIRST}: adds the run examples/a.\n")
        self.assertRegex(result.stdout, r"check-baselines.sh +pass")
        self.assertNotIn("the description has no section", result.stdout)


if __name__ == "__main__":
    unittest.main()
