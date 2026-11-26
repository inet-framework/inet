#!/usr/bin/env python3
"""Tests of baseline_masters.py: the master branch of a baseline repository and the pin of an INET commit.

Each test builds a bare baseline repository and an INET-like repository with an inet.opp that pins
one of its commits. The tests use local paths, so no test needs a network or a deploy key.

The tests of the trailer form land cause commits with pin trailers: by a fast-forward, by a rebase,
or not at all at an overlap, and with a mark after the steps. They also test the command check.
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


TRAILER_OPP = """SimulationProject(
    name="inet",
    test_parameters={
        "statistical": {"baseline": {"repository": "inet-framework/statistics", "trailer": "Statistical-Baseline", "folder": "statistics"}},
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


class TrailerLandingTest(BaselineMastersTest):
    """The trailer form: the landing of cause commits, the mark, and the command check."""

    def setUp(self):
        super().setUp()
        self.pin(self.first)
        (self.inet / "inet.opp").write_text(TRAILER_OPP)
        git(self.inet, "add", "-A")
        git(self.inet, "commit", "-q", "-m", "switch to the trailer form")
        self.switch = git(self.inet, "rev-parse", "HEAD")
        # a series starts on a detached head; land_inet puts its commits onto master
        git(self.inet, "checkout", "-q", "--detach")

    def branch(self, name, files, start=None):
        """A baseline branch: one commit for each file, on start (default: the head of master)."""
        git(self.work, "fetch", "-q", "origin")
        git(self.work, "checkout", "-q", "--detach", start or "origin/master")
        commits = []
        for path, content in files:
            (self.work / path).write_text(content)
            git(self.work, "add", "-A")
            git(self.work, "commit", "-q", "-m", f"{name}: {path}")
            commits.append(git(self.work, "rev-parse", "HEAD"))
        self.publish(commits[-1], f"refs/heads/{name}")
        return commits

    def cause(self, subject, pin, start=None):
        if start:
            git(self.inet, "checkout", "-q", "--detach", start)
        (self.inet / f"{subject}.cc").write_text(subject + "\n")
        git(self.inet, "add", "-A")
        git(self.inet, "commit", "-q", "-m", subject, "-m", f"Statistical-Baseline: {pin}")
        return git(self.inet, "rev-parse", "HEAD")

    def land_inet(self, *commits):
        """Lands INET commits on master with new hashes, as "Rebase and merge" does."""
        git(self.inet, "checkout", "-q", "master")
        for commit in commits:
            subprocess.run(["git", "-c", "user.name=lander", "-c", "user.email=lander@example.invalid", "cherry-pick", commit],
                           cwd=self.inet, check=True, capture_output=True, env={**os.environ, "GIT_COMMITTER_DATE": "2030-01-01T00:00:00+0000"})
        return git(self.inet, "rev-parse", "HEAD")

    def marks(self):
        return git(self.remote, "log", "--format=%(trailers:key=Baseline-Of,valueonly)", "master").split()

    def test_a_series_on_the_head_of_master_lands_by_a_fast_forward_and_gets_one_mark(self):
        first_pin, second_pin = self.branch("topic", [("a.sca", "2\n"), ("b.sca", "2\n")])
        one = self.cause("one", first_pin)
        two = self.cause("two", second_pin)
        landed_two = self.land_inet(one, two)
        landed_one = git(self.inet, "rev-parse", "HEAD~1")
        result = self.run_tool("--push")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("a fast-forward over 1 commit(s)", result.stdout)
        # the pins are on master as they are, and one mark after them names the two commits
        self.assertEqual(git(self.remote, "rev-parse", "master~1"), second_pin)
        self.assertEqual(git(self.remote, "rev-parse", "master~2"), first_pin)
        self.assertEqual(self.marks(), [landed_one, first_pin, landed_two, second_pin])
        self.assertEqual(git(self.remote, "diff", "master~1", "master"), "")
        # a second run has nothing to land
        result = self.run_tool("--push")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("no cause commit is after the newest mark", result.stdout)

    def test_a_second_series_lands_by_a_rebase_and_names_its_pull_request(self):
        pin_a, = self.branch("topic-a", [("a.sca", "2\n")])
        pin_b, = self.branch("topic-b", [("b.sca", "2\n")])
        self.publish(pin_b, "refs/pull/7/head")
        cause_a = self.cause("a", pin_a)
        cause_b = self.cause("b", pin_b, start=self.switch)
        self.land_inet(cause_a)
        self.assertEqual(self.run_tool("--push").returncode, 0)
        landed_b = self.land_inet(cause_b)
        result = self.run_tool("--push")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("a rebase of 1 commit(s)", result.stdout)
        rebased = git(self.remote, "rev-parse", "master~1")
        self.assertNotEqual(rebased, pin_b)
        self.assertEqual(git(self.remote, "show", "master:a.sca"), "2")
        self.assertEqual(git(self.remote, "show", "master:b.sca"), "2")
        # the rebased commit keeps its message and its author; the mark names the rebased commit and the pull request
        self.assertEqual(git(self.remote, "log", "-1", "--format=%s %an", rebased), "topic-b: b.sca test")
        message = git(self.remote, "log", "-1", "--format=%B", "master")
        self.assertIn(f"Baseline-Of: {landed_b} {rebased}", message)
        self.assertIn("Baseline-Pull-Request: #7", message)
        self.assertIn(f"a maintainer closes the pull request #7 of {self.remote}", result.stdout)
        self.assertTrue(message.startswith(f"Baseline of INET {landed_b[:10]}: b"))

    def test_an_overlap_stops_the_landing_and_pushes_the_steps_before_it(self):
        pin_a, = self.branch("topic-a", [("a.sca", "2\n")])
        pin_c, = self.branch("topic-c", [("a.sca", "3\n")])
        cause_a = self.cause("a", pin_a)
        cause_c = self.cause("c", pin_c, start=self.switch)
        landed_a = self.land_inet(cause_a)
        landed_c = self.land_inet(cause_c)
        result = self.run_tool("--push")
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn(f"the landing stops at {landed_c[:10]}", result.stdout)
        self.assertIn("master also changes a.sca", result.stdout)
        self.assertEqual(self.marks(), [landed_a, pin_a])
        # a later run tries the stopped commit again
        result = self.run_tool("--push")
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("1 cause commit(s) to land", result.stdout)

    def test_without_push_the_tool_fails_and_changes_nothing(self):
        pin_a, = self.branch("topic-a", [("a.sca", "2\n")])
        self.land_inet(self.cause("a", pin_a))
        result = self.run_tool()
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("run the tool with --push", result.stdout)
        self.assertEqual(self.master(), self.first)

    def run_check(self, commit, base):
        return subprocess.run([sys.executable, str(TOOL), "check", commit, "--base", base, "--baseline-repository", f"statistical={self.remote}"],
                              cwd=self.inet, capture_output=True, text=True)

    def test_the_check_passes_a_branch_with_no_common_file_and_fails_an_overlap(self):
        pin_a, = self.branch("topic-a", [("a.sca", "2\n")])
        pin_b, = self.branch("topic-b", [("b.sca", "2\n")])
        pin_c, = self.branch("topic-c", [("a.sca", "3\n")])
        cause_a = self.cause("a", pin_a)
        cause_b = self.cause("b", pin_b, start=self.switch)
        cause_c = self.cause("c", pin_c, start=self.switch)
        for cause in (cause_a, cause_b, cause_c):
            result = self.run_check(cause, self.switch)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        master = self.land_inet(cause_a)
        self.assertEqual(self.run_tool("--push").returncode, 0)
        result = self.run_check(cause_b, master)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("RESULT: statistical: 1 commit(s)", result.stdout)
        result = self.run_check(cause_c, master)
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn("RESULT: statistical: overlap with master", result.stdout)
        self.assertIn("a.sca", result.stdout.splitlines()[-1])

    def test_the_check_of_a_series_with_no_pin_passes(self):
        git(self.inet, "commit", "-q", "--allow-empty", "-m", "no pin")
        result = self.run_check("HEAD", self.switch)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("RESULT: no baseline change", result.stdout)


if __name__ == "__main__":
    unittest.main()
