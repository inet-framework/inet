#!/usr/bin/env python3
"""The context `Master green`: M8 and M10 of plan/pending/master-guards.md.

`Master green` is green only when the full run of the head of master is finished and green. While
that run is in progress or red, nothing else lands. The full run is the newest attempt of each of
the ten workflows below on the commit, from the events push and workflow_run; a scheduled run, a
weekly run and a run by hand do not count. The job `Test map` of the fingerprint workflow does not
count, because it runs the checks V4 and V5 of the test selection. A run that is cancelled is red.

  state     prints the state of the full run of a commit (default: the head of master)
  post      posts the state of the head of master as a commit status:
              --on SHA...  on these commits (the head of a pull request, or of a pushed branch)
              --all        on the head of master, and, when the state changed since the last post
                           on master (or with --force), on the head of each open pull request into
                           master; between two posts it waits, for the limits of GitHub

When the state is red, `post --all` also names the commits of omnetpp (omnetpp-6.x) and opp_repl
(github_ci) at the start of the run, and the same for the newest green commit of master (M10).

It reads GitHub through `gh api`, with the token in GH_TOKEN. It needs the permissions actions:read,
pull-requests:read and statuses:write.

Usage:
  python3 doc/project/enforcement/master_green.py state [--commit SHA]
  python3 doc/project/enforcement/master_green.py post --all [--force] --context="Master green"
  python3 doc/project/enforcement/master_green.py post --on SHA --context="Master green"

Exit status 0 = done, 2 = the state cannot be read or a post failed.
"""
import argparse
import json
import os
import subprocess
import sys
import time

FULL_RUN = ("Check: baseline masters", "Test: unit", "Test: module", "Test: queueing", "Test: protocol",
            "Test: packet", "Test: validation", "Test: fingerprint", "Test: statistical", "Test: chart")
EVENTS = ("push", "workflow_run")
NOT_COUNTED_JOBS = {"Test: fingerprint": {"Test map"}}
GOOD = {"success", "skipped", "neutral"}
RULES = "plan/pending/master-guards.md"
ENVIRONMENT = (("omnetpp", "omnetpp/omnetpp", "omnetpp-6.x"), ("opp_repl", "omnetpp/opp_repl", "github_ci"))
PAUSE = 1.0   # seconds between two posts: GitHub allows about 80 writes in a minute


class CannotRun(Exception):
    pass


class GitHub:
    """The calls to GitHub. The tests replace it."""

    def __init__(self, repository):
        self.repository = repository

    def call(self, path, *fields, method="GET", lines=None):
        """The JSON of the answer; with lines, a jq filter, the words of the text of all pages."""
        command = ["gh", "api", "-X", method, path, *fields] + (["--paginate", "--jq", lines] if lines else [])
        out = subprocess.run(command, capture_output=True, text=True)
        if out.returncode != 0:
            raise CannotRun(f"gh api {path}: {out.stderr.strip()}")
        if lines:
            return out.stdout.split()
        return json.loads(out.stdout) if out.stdout.strip() else None

    def master_head(self):
        return self.call(f"repos/{self.repository}/commits/master")["sha"]

    def master_commits(self, count):
        return [c["sha"] for c in self.call(f"repos/{self.repository}/commits?sha=master&per_page={count}")]

    def runs(self, sha):
        return self.call(f"repos/{self.repository}/actions/runs?head_sha={sha}&per_page=100")["workflow_runs"]

    def jobs(self, run_id):
        return self.call(f"repos/{self.repository}/actions/runs/{run_id}/jobs?per_page=100")["jobs"]

    def statuses(self, sha):
        return self.call(f"repos/{self.repository}/commits/{sha}/statuses?per_page=100")

    def pull_request_heads(self):
        words = self.call(f"repos/{self.repository}/pulls?state=open&base=master&per_page=100",
                          lines='.[] | "\\(.number) \\(.head.sha)"')
        return [(int(words[i]), words[i + 1]) for i in range(0, len(words), 2)]

    def commit_before(self, repository, branch, time_text):
        commits = self.call(f"repos/{repository}/commits?sha={branch}&until={time_text}&per_page=1")
        return commits[0]["sha"] if commits else None

    def post(self, sha, state, context, description, target_url):
        fields = ["-f", f"state={state}", "-f", f"context={context}", "-f", f"description={description[:140]}"]
        if target_url:
            fields += ["-f", f"target_url={target_url}"]
        self.call(f"repos/{self.repository}/statuses/{sha}", *fields, method="POST")


class State:
    def __init__(self, sha, state, description, runs):
        self.sha, self.state, self.description, self.runs = sha, state, description, runs

    def __repr__(self):
        return f"{self.state}: {self.description}"


def full_run(github, sha):
    """The newest run of each workflow of the full run on the commit, by its name."""
    found = {}
    for run in github.runs(sha):
        if run["name"] in FULL_RUN and run["event"] in EVENTS and run["head_sha"] == sha:
            if run["name"] not in found or run["id"] > found[run["name"]]["id"]:
                found[run["name"]] = run
    return found


def state_of(github, sha):
    """The state of the full run of a commit: success, pending or failure, with a description."""
    runs = full_run(github, sha)
    red, finished = [], 0
    for name in FULL_RUN:
        run = runs.get(name)
        if run is None or run["status"] != "completed":
            continue
        finished += 1
        if run["conclusion"] == "success":
            continue
        excluded = NOT_COUNTED_JOBS.get(name, set())
        bad = [j["name"] for j in github.jobs(run["id"]) if j["name"] not in excluded and j["conclusion"] not in GOOD]
        if bad or run["conclusion"] in ("cancelled", "timed_out", "startup_failure", "action_required"):
            red.append(f"{name} ({len(bad)} job(s))" if bad else f"{name} ({run['conclusion']})")
    short = sha[:10]
    if red:
        return State(sha, "failure", f"master {short} is red: " + ", ".join(red), runs)
    if finished < len(FULL_RUN):
        return State(sha, "pending", f"the full run of master {short} is in progress: {finished} of {len(FULL_RUN)} workflows done", runs)
    return State(sha, "success", f"the full run of master {short} is green", runs)


def last_posted(github, context, depth=10):
    """The state of the newest status with the context on the newest commits of master, the head first,
    or None. A new head has no status yet, and the pull requests have the state of an older head."""
    for sha in github.master_commits(depth):
        for status in github.statuses(sha):
            if status["context"] == context:
                return status["state"]
    return None


def environment(github, state):
    """The commits of omnetpp and opp_repl at the start of the full run of a state, as a text."""
    starts = sorted(run["created_at"] for run in state.runs.values())
    if not starts:
        return "no run"
    return ", ".join(f"{name} {(github.commit_before(repository, branch, starts[0]) or '?')[:10]} ({branch})"
                     for name, repository, branch in ENVIRONMENT)


def report(github, current, depth=30):
    """The lines of M10 for a red state: the environment of this run and of the newest green commit."""
    lines = [f"{current.sha[:10]} (red): {environment(github, current)}"]
    for sha in github.master_commits(depth):
        if sha == current.sha:
            continue
        older = state_of(github, sha)
        if older.state == "success":
            lines.append(f"{sha[:10]} (the newest green commit): {environment(github, older)}")
            break
    else:
        lines.append(f"no green commit among the newest {depth} commits of master")
    return lines


def post_all(github, context, target_url, force, pause, out):
    head = github.master_head()
    current = state_of(github, head)
    previous = last_posted(github, context)
    print(f"master {head[:10]}: {current} (the last post on master: {previous or 'none'})", file=out)
    github.post(head, current.state, context, current.description, target_url)
    if current.state == "failure":
        for line in report(github, current):
            print(f"  {line}", file=out)
    if previous == current.state and not force:
        print("the state did not change: no post on the pull requests", file=out)
        return 0
    heads = github.pull_request_heads()
    for number, sha in heads:
        github.post(sha, current.state, context, current.description, target_url)
        time.sleep(pause)
    print(f"posted on {len(heads)} open pull request(s)", file=out)
    return 0


def post_on(github, context, target_url, shas, out):
    current = state_of(github, github.master_head())
    for sha in shas:
        github.post(sha, current.state, context, current.description, target_url)
        print(f"{sha[:10]}: {current}", file=out)
    return 0


def main(argv=None, github=None, out=sys.stdout, pause=PAUSE):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("action", choices=("state", "post"))
    parser.add_argument("--commit", help="state: the commit (default: the head of master)")
    parser.add_argument("--all", action="store_true", help="post: on master and, at a change, on each open pull request")
    parser.add_argument("--on", nargs="+", default=[], metavar="SHA", help="post: on these commits")
    parser.add_argument("--force", action="store_true", help="post --all: also when the state did not change")
    parser.add_argument("--context", default="Master green", help="the context of the commit status")
    parser.add_argument("--repository", default=os.environ.get("GITHUB_REPOSITORY", "inet-framework/inet"))
    args = parser.parse_args(argv)
    github = github or GitHub(args.repository)
    target_url = (f"{os.environ['GITHUB_SERVER_URL']}/{os.environ['GITHUB_REPOSITORY']}/actions/runs/{os.environ['GITHUB_RUN_ID']}"
                  if all(k in os.environ for k in ("GITHUB_SERVER_URL", "GITHUB_REPOSITORY", "GITHUB_RUN_ID")) else None)
    try:
        if args.action == "state":
            current = state_of(github, args.commit or github.master_head())
            print(current, file=out)
            for name in FULL_RUN:
                run = current.runs.get(name)
                print(f"  {name}: {'no run' if run is None else (run['conclusion'] or run['status'])}", file=out)
            return 0
        if args.all == bool(args.on):
            parser.error("post needs either --all or --on")
        if args.all:
            return post_all(github, args.context, target_url, args.force, pause, out)
        return post_on(github, args.context, target_url, args.on, out)
    except CannotRun as error:
        print(f"error: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
