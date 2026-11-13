#!/usr/bin/env python3
"""Keeps the master branch of each baseline repository at the pin of an INET commit (TR-BASELINE-PIN).

After a change lands on INET master, the master branch of each baseline repository must be the
commit that inet.opp of INET master pins. For each baseline entry with a full-hash pin, the tool
compares the master branch of the baseline repository with the pin:

  at the pin   nothing to do
  behind       master is an ancestor of the pin: with --push the tool moves master to the pin by a
               fast-forward; without --push it prints the command and fails
  ahead        master holds commits that no INET commit pins: the tool fails
  diverged     neither commit is an ancestor of the other: the tool fails
  missing      the pin is not in the baseline repository: the tool fails

The tool reads inet.opp from git objects and gets the baseline commits without file contents.

Usage, from the INET repository root:
  baseline_masters.py [COMMIT] [--push] [--baseline-repository KIND=URL]...

COMMIT is the INET commit whose pins count (default HEAD). With --push, the push to a repository on
GitHub goes over SSH with the deploy key in the environment variable <NAME>_DEPLOY_KEY, where NAME is
the name of the repository in capitals: STATISTICS_DEPLOY_KEY, MEDIA_DEPLOY_KEY. With no key, the
tool prints the command and fails. A push is never forced, so the server refuses a push that is not
a fast-forward. --baseline-repository gives another URL or path for a baseline repository.

Exit status 0 = each master is at its pin at the end, 1 = a master is not, 2 = invalid usage or the
tool cannot run.
"""
import argparse
import os
import re
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from baseline_pins import PinError, git, is_full_hash, read_baselines  # noqa: E402

GITHUB_NAME = re.compile(r"[A-Za-z0-9._-]+/[A-Za-z0-9._-]+")


def fetch_url(repository):
    return f"https://github.com/{repository}.git" if GITHUB_NAME.fullmatch(repository) else repository


def key_variable(repository):
    """The environment variable of the deploy key of a repository on GitHub, or None for another URL."""
    if not GITHUB_NAME.fullmatch(repository):
        return None
    return re.sub(r"[^A-Z0-9]", "_", repository.split("/")[1].upper()) + "_DEPLOY_KEY"


class Remote:
    """A bare clone with no file contents, of one baseline repository."""

    def __init__(self, url, folder):
        self.url = url
        self.folder = folder
        os.makedirs(folder)
        git("init", "-q", "--bare", ".", cwd=folder)
        git("remote", "add", "origin", url, cwd=folder)

    def master(self):
        """The commit of the master branch, or None when the repository has no master branch."""
        out = git("ls-remote", "--exit-code", "origin", "refs/heads/master", cwd=self.folder, check=False)
        if out.returncode == 2:
            return None
        if out.returncode != 0:
            raise RuntimeError(f"cannot read {self.url}:\n{out.stderr.strip()}")
        return out.stdout.split()[0]

    def has(self, commit):
        return git("cat-file", "-e", f"{commit}^{{commit}}", cwd=self.folder, check=False).returncode == 0

    def get(self, commit):
        """True when the commit is in the repository; it is then in the clone too."""
        if not self.has(commit):
            git("fetch", "-q", "--no-tags", "--filter=blob:none", "origin", commit, cwd=self.folder, check=False)
        return self.has(commit)

    def is_ancestor(self, ancestor, commit):
        return git("merge-base", "--is-ancestor", ancestor, commit, cwd=self.folder, check=False).returncode == 0

    def count(self, old, new):
        return int(git("rev-list", "--count", f"{old}..{new}", cwd=self.folder).stdout)


def push(remote, repository, pin):
    """Moves master to the pin by a fast-forward. Returns an error text, or None after a push."""
    variable = key_variable(repository)
    env = dict(os.environ)
    url = remote.url
    with tempfile.TemporaryDirectory() as folder:
        if variable is not None:
            key = os.environ.get(variable, "")
            if not key.strip():
                return f"the environment variable {variable} holds no deploy key"
            key_file = os.path.join(folder, "key")
            with open(os.open(key_file, os.O_WRONLY | os.O_CREAT, 0o600), "w") as f:
                f.write(key.strip() + "\n")
            url = f"git@github.com:{repository}.git"
            env["GIT_SSH_COMMAND"] = f"ssh -i {key_file} -o IdentitiesOnly=yes -o StrictHostKeyChecking=accept-new"
        # a plain push: the server refuses it when it is not a fast-forward, also when master moved
        # after the tool read it
        out = subprocess.run(["git", "push", "--quiet", url, f"{pin}:refs/heads/master"], cwd=remote.folder, env=env,
                             capture_output=True, text=True)
    if out.returncode != 0:
        return f"the push failed:\n{out.stderr.strip()}"
    return None


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("commit", nargs="?", default="HEAD")
    parser.add_argument("--push", action="store_true", help="move a master that is behind its pin to the pin")
    parser.add_argument("--baseline-repository", action="append", default=[], metavar="KIND=URL",
                        help="another URL or path for the baseline repository of a test kind")
    args = parser.parse_args(argv)
    sources = dict(value.split("=", 1) for value in args.baseline_repository if "=" in value)
    if len(sources) != len(args.baseline_repository):
        print("error: --baseline-repository needs KIND=URL", file=sys.stderr)
        return 2
    try:
        commit = git("rev-parse", "--verify", f"{args.commit}^{{commit}}").stdout.strip()
        baselines = read_baselines(commit)
    except (PinError, RuntimeError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 2
    print(f"== the baseline masters and the pins of {commit[:10]} ==")
    failures = 0
    with tempfile.TemporaryDirectory() as cache:
        for kind, entry in sorted(baselines.items()):
            pin, repository = entry.get("ref"), entry["repository"]
            if not is_full_hash(pin):
                print(f"  {kind}: no full-hash pin; nothing to check")
                continue
            try:
                remote = Remote(fetch_url(sources.get(kind, repository)), os.path.join(cache, kind))
                master = remote.master()
                if not remote.get(pin):
                    print(f"  {kind}: FAIL: the pin {pin[:10]} is not in {repository}; push the baseline commit first")
                    failures += 1
                    continue
                if master is None or not remote.get(master):
                    print(f"  {kind}: FAIL: {repository} has no master branch")
                    failures += 1
                    continue
                if master == pin:
                    print(f"  {kind}: master of {repository} is at the pin {pin[:10]}")
                elif remote.is_ancestor(master, pin):
                    behind = remote.count(master, pin)
                    command = f"git push git@github.com:{repository}.git {pin}:refs/heads/master"
                    if not args.push:
                        print(f"  {kind}: FAIL: master of {repository} is {behind} commit(s) behind the pin {pin[:10]}; move it: {command}")
                        failures += 1
                        continue
                    error = push(remote, sources.get(kind, repository), pin)
                    if error is not None:
                        print(f"  {kind}: FAIL: master of {repository} is {behind} commit(s) behind the pin {pin[:10]}; {error}")
                        print(f"  {kind}: move it by hand: {command}")
                        failures += 1
                    else:
                        print(f"  {kind}: moved master of {repository} by a fast-forward over {behind} commit(s) to the pin {pin[:10]}")
                elif remote.is_ancestor(pin, master):
                    print(f"  {kind}: FAIL: master of {repository} is {remote.count(pin, master)} commit(s) ahead of the pin {pin[:10]}; "
                          f"no INET commit pins them: pin them in an INET commit, or move master back by hand")
                    failures += 1
                else:
                    print(f"  {kind}: FAIL: master of {repository} ({master[:10]}) and the pin {pin[:10]} are on different lines; "
                          f"the baseline branch must stand on master")
                    failures += 1
            except RuntimeError as error:
                print(f"error: {error}", file=sys.stderr)
                return 2
    print()
    print("FAIL: see TR-BASELINE-PIN in doc/project/rule/testing.md." if failures else "PASS: each baseline master is at its pin.")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
