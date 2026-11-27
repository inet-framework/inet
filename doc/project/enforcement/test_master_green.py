#!/usr/bin/env python3
"""Tests of master_green.py: the context `Master green` of M8 and M10 in plan/pending/master-guards.md."""

import io
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))
import master_green  # noqa: E402
from master_green import FULL_RUN, main, state_of  # noqa: E402

HEAD = "a" * 40
OLD = "b" * 40


def run(name, conclusion="success", status="completed", event=None, run_id=None, sha=HEAD):
    event = event or ("workflow_run" if name in ("Test: statistical", "Test: chart") else "push")
    return {"id": run_id or (FULL_RUN.index(name) + 1) * 10, "name": name, "event": event, "head_sha": sha,
            "status": status, "conclusion": conclusion if status == "completed" else None,
            "created_at": "2026-11-27T12:00:00Z"}


class FakeGitHub:
    def __init__(self, runs, jobs=None, statuses=None, heads=((1, "c" * 40), (2, "d" * 40)), commits=(HEAD, OLD)):
        self.runs_of = runs
        self.jobs_of = jobs or {}
        self.statuses_of = statuses or {}
        self.heads = list(heads)
        self.commits = list(commits)
        self.posts = []
        self.cancelled = []

    def master_head(self):
        return HEAD

    def master_commits(self, count):
        return self.commits[:count]

    def runs(self, sha):
        return [r for r in self.runs_of if r["head_sha"] == sha]

    def jobs(self, run_id):
        return self.jobs_of.get(run_id, [{"name": "a job", "conclusion": "failure"}])

    def statuses(self, sha):
        return self.statuses_of.get(sha, [])

    def pull_request_heads(self):
        return self.heads

    def commit_before(self, repository, branch, time_text):
        return {"omnetpp/omnetpp": "1" * 40, "omnetpp/opp_repl": "2" * 40}[repository]

    def post(self, sha, state, context, description, target_url):
        self.posts.append((sha, state, context, description))

    def cancel(self, run_id):
        self.cancelled.append(run_id)


def green(sha=HEAD):
    return [run(name, sha=sha) for name in FULL_RUN]


class StateTest(unittest.TestCase):
    def test_a_finished_green_run_is_green(self):
        self.assertEqual(state_of(FakeGitHub(green()), HEAD).state, "success")

    def test_a_missing_or_running_workflow_is_pending(self):
        runs = green()[:-1]
        current = state_of(FakeGitHub(runs), HEAD)
        self.assertEqual(current.state, "pending")
        self.assertIn("9 of 10 workflows done", current.description)
        runs = green()
        runs[0] = run(FULL_RUN[0], status="in_progress")
        self.assertEqual(state_of(FakeGitHub(runs), HEAD).state, "pending")

    def test_a_red_workflow_is_red_also_while_others_run(self):
        runs = green()
        runs[1] = run(FULL_RUN[1], "failure")
        runs[2] = run(FULL_RUN[2], status="in_progress")
        current = state_of(FakeGitHub(runs), HEAD)
        self.assertEqual(current.state, "failure")
        self.assertIn("Test: unit (1 job(s))", current.description)

    def test_the_job_test_map_of_the_fingerprint_workflow_does_not_count(self):
        runs = green()
        fingerprint = FULL_RUN.index("Test: fingerprint")
        runs[fingerprint] = run("Test: fingerprint", "failure")
        jobs = {runs[fingerprint]["id"]: [{"name": "fingerprint-tests (release, 0)", "conclusion": "success"},
                                          {"name": "Test map", "conclusion": "failure"}]}
        self.assertEqual(state_of(FakeGitHub(runs, jobs), HEAD).state, "success")
        jobs[runs[fingerprint]["id"]][0]["conclusion"] = "failure"
        self.assertEqual(state_of(FakeGitHub(runs, jobs), HEAD).state, "failure")

    def test_a_cancelled_run_is_red(self):
        runs = green()
        runs[0] = run(FULL_RUN[0], "cancelled")
        jobs = {runs[0]["id"]: []}
        current = state_of(FakeGitHub(runs, jobs), HEAD)
        self.assertEqual(current.state, "failure")
        self.assertIn("Check: baseline masters (cancelled)", current.description)

    def test_a_scheduled_run_does_not_count_and_the_newest_run_counts(self):
        runs = green()
        runs.append(run("Test: unit", "failure", event="schedule", run_id=999))
        self.assertEqual(state_of(FakeGitHub(runs), HEAD).state, "success")
        runs.append(run("Test: unit", "failure", run_id=998))
        self.assertEqual(state_of(FakeGitHub(runs), HEAD).state, "failure")


class PostTest(unittest.TestCase):
    def post(self, github, *args):
        out = io.StringIO()
        status = main(["post", *args], github=github, out=out, pause=0)
        self.assertEqual(status, 0, out.getvalue())
        return out.getvalue()

    def test_a_change_of_the_state_posts_on_master_and_each_pull_request(self):
        github = FakeGitHub(green()[:-1], statuses={HEAD: [{"context": "Master green", "state": "success"}]})
        self.post(github, "--all")
        self.assertEqual([(sha[0], state) for sha, state, _, _ in github.posts], [("a", "pending"), ("c", "pending"), ("d", "pending")])

    def test_the_same_state_posts_only_on_master(self):
        github = FakeGitHub(green()[:-1], statuses={HEAD: [{"context": "Master green", "state": "pending"}]})
        output = self.post(github, "--all")
        self.assertEqual([sha[0] for sha, _, _, _ in github.posts], ["a"])
        self.assertIn("no post on the pull requests", output)
        github.posts.clear()
        self.post(github, "--all", "--force")
        self.assertEqual(len(github.posts), 3)

    def test_a_new_head_takes_the_last_state_of_an_older_commit_of_master(self):
        github = FakeGitHub(green()[:-1], statuses={OLD: [{"context": "Master green", "state": "pending"}]})
        self.post(github, "--all")
        self.assertEqual([sha[0] for sha, _, _, _ in github.posts], ["a"])
        github = FakeGitHub(green()[:-1], statuses={OLD: [{"context": "Master green", "state": "success"}]})
        self.post(github, "--all")
        self.assertEqual(len(github.posts), 3)

    def test_a_post_on_a_commit_gives_the_state_of_master(self):
        github = FakeGitHub(green())
        self.post(github, "--on", "e" * 40, "--context=Master green")
        self.assertEqual(github.posts, [("e" * 40, "success", "Master green", f"the full run of master {HEAD[:10]} is green")])

    def test_a_red_state_names_the_environment_of_the_run_and_of_the_newest_green_commit(self):
        runs = green() + green(OLD)
        runs[1] = run(FULL_RUN[1], "failure")
        output = self.post(FakeGitHub(runs), "--all")
        self.assertIn(f"{HEAD[:10]} (red): omnetpp 1111111111 (omnetpp-6.x), opp_repl 2222222222 (github_ci)", output)
        self.assertIn(f"{OLD[:10]} (the newest green commit): omnetpp", output)

    def test_post_needs_all_or_on(self):
        with self.assertRaises(SystemExit):
            main(["post"], github=FakeGitHub(green()), out=io.StringIO())


MARK = {"context": "Trivial landing", "state": "success", "description": f"Master green follows {OLD}"}


class TrivialLandingTest(unittest.TestCase):
    def test_master_green_follows_a_trivial_landing(self):
        github = FakeGitHub(green(OLD), statuses={HEAD: [MARK]})
        head, current = master_green.master_state(github)
        self.assertEqual((head, current.sha, current.state), (HEAD, OLD, "success"))
        self.assertIn(f"{HEAD[:10]} is a trivial landing", current.description)
        runs = green(OLD)
        runs[1] = run(FULL_RUN[1], "failure", sha=OLD)
        self.assertEqual(master_green.master_state(FakeGitHub(runs, statuses={HEAD: [MARK]}))[1].state, "failure")

    def test_a_trivial_landing_does_not_change_the_pull_requests(self):
        github = FakeGitHub(green(OLD), statuses={HEAD: [MARK], OLD: [{"context": "Master green", "state": "success"}]},
                            commits=(HEAD, OLD))
        out = io.StringIO()
        self.assertEqual(main(["post", "--all"], github=github, out=out, pause=0), 0)
        self.assertEqual([(sha[0], state) for sha, state, _, _ in github.posts], [("a", "success")])
        self.assertIn("no post on the pull requests", out.getvalue())

    def test_the_mark_cancels_the_opp_tests_and_the_fingerprint_tests_but_not_the_landing_job(self):
        runs = [run(name, status="in_progress") for name in FULL_RUN] + [dict(run("Test: unit"), name="Build: Linux", id=500, status="queued")]
        github = FakeGitHub(runs)
        self.assertEqual(main(["trivial-landing", "--commit", HEAD, "--before", OLD], github=github, out=io.StringIO()), 0)
        self.assertEqual(github.posts, [(HEAD, "success", "Trivial landing", f"Master green follows {OLD}")])
        cancelled = {r["name"] for r in runs if r["id"] in github.cancelled}
        self.assertEqual(cancelled, set(master_green.CANCELLED_ON_TRIVIAL))
        self.assertNotIn("Check: baseline masters", cancelled)
        self.assertNotIn("Test: statistical", cancelled)

    def test_is_trivial_waits_for_the_job_landing(self):
        decider = dict(run("Test: unit"), name="Check: master green", id=700)
        github = FakeGitHub([decider], jobs={700: [{"name": "Landing", "status": "completed", "conclusion": "success"}]},
                            statuses={HEAD: [MARK]})
        out = io.StringIO()
        master_green.is_trivial(github, HEAD, timeout=0, out=out, poll=0)
        self.assertEqual(out.getvalue().strip(), "trivial")
        out = io.StringIO()
        master_green.is_trivial(FakeGitHub([decider], jobs={700: [{"name": "Landing", "status": "completed"}]}), HEAD, 0, out, 0)
        self.assertEqual(out.getvalue().strip(), "not trivial")


class WorkflowTest(unittest.TestCase):
    """A renamed workflow would keep Master green pending for ever."""

    def test_each_workflow_of_the_full_run_exists_and_triggers_the_check(self):
        try:
            import yaml
        except ImportError:
            self.skipTest("needs PyYAML")
        root = Path(__file__).resolve().parents[3]
        names = {}
        for path in sorted((root / ".github/workflows").glob("*.yml")):
            names[yaml.safe_load(path.read_text(encoding="utf-8")).get("name")] = path.name
        for name in FULL_RUN:
            self.assertIn(name, names, f"no workflow has the name {name!r}")
        check = yaml.safe_load((root / ".github/workflows/check-master-green.yml").read_text(encoding="utf-8"))
        self.assertEqual(tuple(check[True]["workflow_run"]["workflows"]), FULL_RUN)


if __name__ == "__main__":
    unittest.main()
