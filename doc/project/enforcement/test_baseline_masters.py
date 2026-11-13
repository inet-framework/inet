#!/usr/bin/env python3
"""Tests of baseline_masters.py: the master branch of a baseline repository and the pin of an INET commit.

Each test builds a bare baseline repository and an INET-like repository with an inet.opp that pins
one of its commits. The tests use local paths, so no test needs a network or a deploy key.
"""

import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))
import baseline_masters  # noqa: E402

TOOL = Path(__file__).with_name("baseline_masters.py").resolve()

OPP = """SimulationProject(
    name="inet",
    test_parameters={
        "statistical": {"baseline": {"repository": "inet-framework/statistics", "ref": "%s", "folder": "statistics"}},
    },
)
"""


def git(cwd, *args):
    return subprocess.run(["git", "-c", "user.name=test", "-c", "user.email=test@example.invalid", *args], cwd=cwd,
                          check=True, capture_output=True, text=True).stdout.strip()


class BaselineMastersTest(unittest.TestCase):
    def setUp(self):
        self.tempdir = tempfile.TemporaryDirectory()
        root = Path(self.tempdir.name)
        # the baseline repository: a bare repository, as on a server, and a work clone that makes its commits
        self.remote = root / "statistics.git"
        git(root, "init", "-q", "--bare", "-b", "master", str(self.remote))
        git(self.remote, "config", "uploadpack.allowAnySHA1InWant", "true")
        git(self.remote, "config", "uploadpack.allowFilter", "true")
        self.work = root / "work"
        git(root, "clone", "-q", str(self.remote), str(self.work))
        self.first = self.commit("first")
        git(self.work, "push", "-q", "origin", "HEAD:refs/heads/master")
        self.inet = root / "inet"
        self.inet.mkdir()
        git(self.inet, "init", "-q", "-b", "master")

    def tearDown(self):
        self.tempdir.cleanup()

    def commit(self, name):
        (self.work / f"{name}.sca").write_text(name + "\n")
        git(self.work, "add", "-A")
        git(self.work, "commit", "-q", "-m", name)
        return git(self.work, "rev-parse", "HEAD")

    def publish(self, commit, ref="refs/heads/topic"):
        git(self.work, "push", "-q", "-f", "origin", f"{commit}:{ref}")

    def pin(self, ref):
        (self.inet / "inet.opp").write_text(OPP % ref)
        git(self.inet, "add", "-A")
        git(self.inet, "commit", "-q", "-m", f"pin {ref[:10]}")

    def run_tool(self, *args):
        return subprocess.run([sys.executable, str(TOOL), "HEAD", "--baseline-repository", f"statistical={self.remote}",
                               *args], cwd=self.inet, capture_output=True, text=True)

    def master(self):
        return git(self.remote, "rev-parse", "refs/heads/master")

    def test_a_master_at_the_pin_passes(self):
        self.pin(self.first)
        result = self.run_tool()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("is at the pin", result.stdout)

    def test_a_master_behind_the_pin_fails_without_push_and_names_the_command(self):
        second = self.commit("second")
        self.publish(second)
        self.pin(second)
        result = self.run_tool()
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("1 commit(s) behind the pin", result.stdout)
        self.assertIn(f"git push git@github.com:inet-framework/statistics.git {second}:refs/heads/master", result.stdout)
        self.assertEqual(self.master(), self.first)

    def test_push_moves_a_master_behind_the_pin_by_a_fast_forward(self):
        self.commit("second")
        third = self.commit("third")
        self.publish(third)
        self.pin(third)
        result = self.run_tool("--push")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("by a fast-forward over 2 commit(s)", result.stdout)
        self.assertEqual(self.master(), third)

    def test_push_moves_master_to_a_pin_that_only_a_pull_request_head_holds(self):
        second = self.commit("second")
        self.publish(second, "refs/pull/1/head")
        self.pin(second)
        result = self.run_tool("--push")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(self.master(), second)

    def test_a_master_ahead_of_the_pin_fails_and_stays(self):
        self.pin(self.first)
        second = self.commit("second")
        git(self.work, "push", "-q", "origin", "HEAD:refs/heads/master")
        result = self.run_tool("--push")
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("1 commit(s) ahead of the pin", result.stdout)
        self.assertEqual(self.master(), second)

    def test_a_pin_on_another_line_fails_and_push_changes_nothing(self):
        second = self.commit("second")
        git(self.work, "push", "-q", "origin", "HEAD:refs/heads/master")
        git(self.work, "checkout", "-q", "--detach", self.first)
        other = self.commit("other")
        self.publish(other)
        self.pin(other)
        result = self.run_tool("--push")
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("are on different lines", result.stdout)
        self.assertEqual(self.master(), second)

    def test_the_push_refuses_a_move_that_is_not_a_fast_forward(self):
        # master can move after the tool reads it; the push itself must then refuse the move
        second = self.commit("second")
        git(self.work, "push", "-q", "origin", "HEAD:refs/heads/master")
        git(self.work, "checkout", "-q", "--detach", self.first)
        other = self.commit("other")
        self.publish(other)
        with tempfile.TemporaryDirectory() as folder:
            remote = baseline_masters.Remote(str(self.remote), os.path.join(folder, "r"))
            self.assertTrue(remote.get(other))
            error = baseline_masters.push(remote, str(self.remote), other)
        self.assertIn("the push failed", error or "")
        self.assertEqual(self.master(), second)

    def test_a_pin_that_is_not_in_the_repository_fails(self):
        second = self.commit("second")
        self.pin(second)
        result = self.run_tool("--push")
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("is not in inet-framework/statistics", result.stdout)
        self.assertEqual(self.master(), self.first)

    def test_a_branch_name_as_pin_is_not_checked(self):
        self.pin("master")
        result = self.run_tool()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("no full-hash pin", result.stdout)

    def test_a_commit_with_no_inet_opp_passes(self):
        (self.inet / "README").write_text("x\n")
        git(self.inet, "add", "-A")
        git(self.inet, "commit", "-q", "-m", "no project file")
        result = self.run_tool()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_an_invalid_commit_is_a_usage_error(self):
        self.pin(self.first)
        result = subprocess.run([sys.executable, str(TOOL), "nothing"], cwd=self.inet, capture_output=True, text=True)
        self.assertEqual(result.returncode, 2, result.stdout)

    def test_a_repository_argument_without_a_url_is_a_usage_error(self):
        self.pin(self.first)
        result = subprocess.run([sys.executable, str(TOOL), "--baseline-repository", "statistical"], cwd=self.inet,
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 2, result.stdout)


class HelperTest(unittest.TestCase):
    def test_the_deploy_key_variable_of_a_github_repository(self):
        self.assertEqual(baseline_masters.key_variable("inet-framework/statistics"), "STATISTICS_DEPLOY_KEY")
        self.assertEqual(baseline_masters.key_variable("inet-framework/media"), "MEDIA_DEPLOY_KEY")
        self.assertIsNone(baseline_masters.key_variable("/tmp/statistics.git"))
        self.assertIsNone(baseline_masters.key_variable("https://example.org/statistics.git"))

    def test_a_push_to_github_with_no_key_does_not_try_the_network(self):
        with tempfile.TemporaryDirectory() as folder:
            remote = baseline_masters.Remote("https://github.com/inet-framework/statistics.git", os.path.join(folder, "r"))
            environment = dict(os.environ)
            os.environ.pop("STATISTICS_DEPLOY_KEY", None)
            try:
                error = baseline_masters.push(remote, "inet-framework/statistics", "0" * 40)
            finally:
                os.environ.clear()
                os.environ.update(environment)
        self.assertIn("STATISTICS_DEPLOY_KEY holds no deploy key", error)


if __name__ == "__main__":
    unittest.main()
