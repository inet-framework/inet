#!/usr/bin/env python3
"""Tests of check-required-contexts.sh: each required context of master still reports on a pull request."""

from pathlib import Path
import subprocess
import tempfile
import textwrap
import unittest

HERE = Path(__file__).resolve().parent
CHECKER = HERE / "check-required-contexts.sh"

MATRIX = """\
name: "Test: fingerprint"
on:
  push:
    branches:
      - master
  pull_request:
    branches:
      - master
jobs:
  fingerprint-tests:
    runs-on: ubuntu-latest
    strategy:
      matrix:
        mode: ["debug", "release"]
        split: [0, 1]
    steps:
      - run: echo test
"""

SINGLE = """\
name: "Check: pull request"
on:
  pull_request_target:
    branches:
      - master
jobs:
  audit:
    name: Mechanical audit
    runs-on: ubuntu-latest
    steps:
      - run: echo audit
  baseline-merge:
    runs-on: ubuntu-latest
    steps:
      - run: |
          gh api "repos/x/y/statuses/$HEAD" -f state=success \\
            -f context="Baseline merge" -f description="ok"
"""

FINGERPRINT = [f"fingerprint-tests ({m}, {s})" for m in ("debug", "release") for s in (0, 1)]
REQUIRED = FINGERPRINT + ["Mechanical audit", "Baseline merge"]


class RequiredContextsTest(unittest.TestCase):
    def setUp(self):
        self.tempdir = tempfile.TemporaryDirectory()
        self.root = Path(self.tempdir.name)
        self.git("init", "-q")
        self.git("config", "user.email", "test@example.invalid")
        self.git("config", "user.name", "Required Contexts Test")
        self.write(".github/workflows/fingerprint-tests.yml", MATRIX)
        self.write(".github/workflows/check-pull-request.yml", SINGLE)
        self.commit("fixture")
        self.base = self.git("rev-parse", "HEAD").stdout.strip()
        self.required(REQUIRED)

    def tearDown(self):
        self.tempdir.cleanup()

    def git(self, *args):
        return subprocess.run(["git", *args], cwd=self.root, check=True, capture_output=True, text=True)

    def write(self, relative_path, content):
        path = self.root / relative_path
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")

    def commit(self, message):
        self.git("add", "-A")
        self.git("commit", "-q", "--allow-empty", "-m", message)

    def required(self, contexts):
        self.required_file = self.root.parent / f"{self.root.name}-required.txt"
        self.required_file.write_text("".join(f"{c}\n" for c in contexts), encoding="utf-8")
        self.addCleanup(lambda: self.required_file.unlink(missing_ok=True))

    def change(self, relative_path, old, new):
        path = self.root / relative_path
        text = path.read_text(encoding="utf-8")
        self.assertIn(old, text)
        path.write_text(text.replace(old, new), encoding="utf-8")

    def check(self, *extra):
        return subprocess.run(["bash", str(CHECKER), f"{self.base}..HEAD", "--required-file", str(self.required_file), *extra],
                              cwd=self.root, capture_output=True, text=True)

    def assert_pass(self, result):
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertNotIn("VIOLATION", result.stdout)

    def assert_violation(self, result, text):
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn("VIOLATION", result.stdout)
        self.assertIn(text, result.stdout)

    def test_an_unchanged_series_passes(self):
        self.write("README.txt", "a document\n")
        self.commit("doc: add a document")
        result = self.check()
        self.assert_pass(result)
        self.assertIn("6 required context(s)", result.stdout)

    def test_a_renamed_matrix_job_is_a_violation(self):
        self.change(".github/workflows/fingerprint-tests.yml", "  fingerprint-tests:", "  fingerprints:")
        self.commit("build: rename the job")
        self.assert_violation(self.check(), "renames or removes the required context 'fingerprint-tests (debug, 0)'")

    def test_a_changed_matrix_is_a_violation(self):
        self.change(".github/workflows/fingerprint-tests.yml", "split: [0, 1]", "split: [0]")
        self.commit("build: one split")
        result = self.check()
        self.assert_violation(result, "'fingerprint-tests (debug, 1)'")
        self.assertNotIn("'fingerprint-tests (debug, 0)'", result.stdout)

    def test_a_job_name_with_the_matrix_values_gives_the_same_contexts(self):
        self.change(".github/workflows/fingerprint-tests.yml", "    runs-on: ubuntu-latest\n    strategy:",
                    "    name: fingerprint-tests (${{ matrix.mode }}, ${{ matrix.split }})\n    runs-on: ubuntu-latest\n    strategy:")
        self.commit("build: name the job")
        self.assert_pass(self.check())

    def test_a_removed_workflow_is_a_violation(self):
        self.git("rm", "-q", ".github/workflows/check-pull-request.yml")
        self.commit("build: remove the audit")
        result = self.check()
        self.assert_violation(result, "'Mechanical audit'")
        self.assertIn("'Baseline merge'", result.stdout)

    def test_a_job_that_runs_only_on_a_push_does_not_report_on_a_pull_request(self):
        self.change(".github/workflows/fingerprint-tests.yml", "  pull_request:\n    branches:\n      - master\n", "")
        self.commit("build: test only master")
        self.assert_violation(self.check(), "'fingerprint-tests (release, 1)'")

    def test_a_trigger_for_another_branch_does_not_report_on_a_pull_request_into_master(self):
        self.change(".github/workflows/check-pull-request.yml", "  pull_request_target:\n    branches:\n      - master",
                    "  pull_request_target:\n    branches:\n      - website")
        self.commit("build: audit the website branch")
        self.assert_violation(self.check(), "'Mechanical audit'")

    def test_a_removed_status_context_is_a_violation(self):
        self.change(".github/workflows/check-pull-request.yml", 'context="Baseline merge"', 'context="Baseline landing"')
        self.commit("build: rename the status")
        self.assert_violation(self.check(), "'Baseline merge'")

    def test_a_job_level_if_on_a_matrix_job_is_a_violation(self):
        self.change(".github/workflows/fingerprint-tests.yml", "    runs-on: ubuntu-latest\n    strategy:",
                    "    if: github.event_name == 'push'\n    runs-on: ubuntu-latest\n    strategy:")
        self.commit("build: skip the job")
        self.assert_violation(self.check(), "a skipped job reports under the name without the matrix values")

    def test_a_job_level_if_on_another_job_is_a_note(self):
        self.change(".github/workflows/check-pull-request.yml", "    name: Mechanical audit\n",
                    "    name: Mechanical audit\n    if: github.event_name == 'pull_request_target'\n")
        self.commit("build: audit only pull requests")
        result = self.check()
        self.assert_pass(result)
        self.assertIn("note: the series adds or changes the job-level if: of the required job 'audit'", result.stdout)

    def test_an_unchanged_job_level_if_gives_no_note(self):
        self.change(".github/workflows/check-pull-request.yml", "    name: Mechanical audit\n",
                    "    name: Mechanical audit\n    if: github.event_name == 'pull_request_target'\n")
        self.commit("build: audit only pull requests")
        self.base = self.git("rev-parse", "HEAD").stdout.strip()
        self.write("README.txt", "a document\n")
        self.commit("doc: add a document")
        result = self.check()
        self.assert_pass(result)
        self.assertNotIn("job-level if", result.stdout)

    def test_a_path_filter_is_a_violation(self):
        self.change(".github/workflows/fingerprint-tests.yml", "  pull_request:\n    branches:\n      - master\n",
                    "  pull_request:\n    branches:\n      - master\n    paths:\n      - 'src/**'\n")
        self.commit("build: test only source changes")
        self.assert_violation(self.check(), "a path filter in its pull request trigger")

    def test_a_context_that_nothing_reports_is_a_note(self):
        self.required(REQUIRED + ["Master green"])
        result = self.check()
        self.assert_pass(result)
        self.assertIn("no workflow at the base or at the head reports the required context 'Master green'", result.stdout)

    def test_a_matrix_with_include_gives_a_note_and_no_violation(self):
        self.change(".github/workflows/fingerprint-tests.yml", "        split: [0, 1]\n",
                    "        split: [0, 1]\n        include:\n          - mode: debug\n            split: 2\n")
        self.commit("build: one more split")
        result = self.check()
        self.assert_pass(result)
        self.assertIn("the gate cannot compute the names of the job 'fingerprint-tests'", result.stdout)

    def test_unknown_required_contexts_give_a_note_and_pass(self):
        result = subprocess.run(["bash", str(CHECKER), f"{self.base}..HEAD", "--required-file", str(self.root / "missing.txt")],
                                cwd=self.root, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("note: the required contexts of master are unknown", result.stdout)

    def test_the_readme_list_is_compared_with_the_ruleset(self):
        items = "".join(f"- `{c}`\n" for c in REQUIRED)
        self.write("doc/project/enforcement/README.md", f"# Gates\n\n## Required checks of master\n\n{items}\n## Other\n\n- `x`\n")
        self.commit("doc: list the required checks")
        result = self.check()
        self.assert_pass(result)
        self.assertIn("== W5: the list of the required contexts in doc/project/enforcement/README.md ==\n  ok", result.stdout)
        self.required(REQUIRED[1:] + ["Master green"])
        result = self.check()
        self.assertIn("not listed ['Master green']; listed but not required ['fingerprint-tests (debug, 0)']", result.stdout)

    def test_a_range_without_dots_is_a_usage_error(self):
        result = subprocess.run(["bash", str(CHECKER), "HEAD"], cwd=self.root, capture_output=True, text=True)
        self.assertEqual(result.returncode, 2)


if __name__ == "__main__":
    unittest.main()
