#!/usr/bin/env python3
"""Keeps the master branch of each baseline repository at the baseline of INET master (TR-BASELINE-PIN).

After a change lands on INET master, the master branch of each baseline repository must hold the
baseline of the INET commit. The tool does this for each baseline entry of inet.opp, in its form:

The ref form: the ref of the entry is the pin. The tool compares master of the baseline repository
with the pin:

  at the pin   nothing to do
  behind       master is an ancestor of the pin: with --push the tool moves master to the pin by a
               fast-forward; without --push it prints the command and fails
  ahead        master holds commits that no INET commit pins: the tool fails
  diverged     neither commit is an ancestor of the other: the tool fails
  missing      the pin is not in the baseline repository: the tool fails

The trailer form: a commit that moves a result has the pin trailer of the entry, for example
"Statistical-Baseline: <hash>", and the pin is the last commit of a step of its baseline branch. A
mark is a commit on master of the baseline repository with no change of files. Its message has the
line "Baseline-Of: <INET hash> <baseline hash>" for each landed cause commit, and the line
"Baseline-Pull-Request: #N" for each pull request of the baseline repository whose head is a pin of
a rebased step: GitHub does not mark such a pull request as merged, so the tool names it, and a
maintainer closes it. The tool lands the cause commits of INET master after the newest commit that a mark
names, the oldest first:

  fast-forward  the step starts on the head of master: the step stays as it is, and GitHub marks
                a pull request whose head is the pin as merged
  rebase        else the tool makes the commits of the step again on the head of master, with
                their messages and authors, as "Rebase and merge" does
  overlap       a file that the baseline branch changes and that master changed since the base of
                the branch: the tool stops at that commit
  mark          after the steps, one mark names the baseline of each landed cause commit

With --push the tool pushes master; at a stop, it pushes the steps before the stop with their mark,
and fails. A later run tries the stopped commit again, because no mark names it.

The tool reads inet.opp and the messages from git objects, and gets the baseline commits without
file contents that it does not need.

Usage, from the INET repository root:
  baseline_masters.py [COMMIT] [--push] [--baseline-repository KIND=URL]...
  baseline_masters.py check COMMIT [--base BASE] [--baseline-repository KIND=URL]...

COMMIT is the INET commit whose baselines count (default HEAD). With --push, the push to a repository on
GitHub goes over SSH with the deploy key in the environment variable <NAME>_DEPLOY_KEY, where NAME is
the name of the repository in capitals: STATISTICS_DEPLOY_KEY, MEDIA_DEPLOY_KEY. With no key, the
tool prints the command and fails. A push is never forced, so the server refuses a push that is not
a fast-forward. --baseline-repository gives another URL or path for a baseline repository.

check examines whether the series BASE..COMMIT of a pull request can land: for each kind of the
trailer form at BASE (the entries of BASE count, so that a series cannot name another repository), the baseline branch of its newest pin has no merge commit, and no file is common to the
branch and to master of the baseline repository since the base of the branch, and its rebase onto
master has no conflict. BASE is the head of INET master (default: the merge base of COMMIT and
origin/master). Its last line, "RESULT: <text>", is the text of the commit status "Baseline merge".

Exit status 0 = each master holds its baseline at the end (check: the series can land), 1 = not,
2 = invalid usage or the tool cannot run.
"""
import argparse
import os
import re
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from baseline_pins import PinError, git, is_full_hash, is_trailer_form, pin_trailers, read_baselines  # noqa: E402

GITHUB_NAME = re.compile(r"[A-Za-z0-9._-]+/[A-Za-z0-9._-]+")
MARK = "Baseline-Of"
PULL_REQUEST = "Baseline-Pull-Request"
# the committer of the commits that the tool makes: the authors of the baseline commits stay
LANDING_IDENTITY = {"GIT_COMMITTER_NAME": "github-actions[bot]", "GIT_COMMITTER_EMAIL": "41898282+github-actions[bot]@users.noreply.github.com"}


class LandError(Exception):
    """A step cannot land on master of the baseline repository."""


def fetch_url(repository):
    return f"https://github.com/{repository}.git" if GITHUB_NAME.fullmatch(repository) else repository


def key_variable(repository):
    """The environment variable of the deploy key of a repository on GitHub, or None for another URL."""
    if not GITHUB_NAME.fullmatch(repository):
        return None
    return re.sub(r"[^A-Z0-9]", "_", repository.split("/")[1].upper()) + "_DEPLOY_KEY"


def short(commit):
    return commit[:10]


class Remote:
    """A bare clone with no file contents, of one baseline repository. It gets a file content only
    when it needs it."""

    def __init__(self, url, folder):
        self.url = url
        self.folder = folder
        os.makedirs(folder)
        git("init", "-q", "--bare", ".", cwd=folder)
        git("remote", "add", "origin", url, cwd=folder)
        git("config", "remote.origin.promisor", "true", cwd=folder)
        git("config", "remote.origin.partialclonefilter", "blob:none", cwd=folder)

    def git(self, *args, check=True, env=None, input=None):
        if env is None and input is None:
            return git(*args, cwd=self.folder, check=check)
        out = subprocess.run(["git", *args], cwd=self.folder, capture_output=True, text=True, env=env, input=input)
        if check and out.returncode != 0:
            raise RuntimeError(f"git {' '.join(args)} failed:\n{out.stderr.strip()}")
        return out

    def master(self):
        """The commit of the master branch, or None when the repository has no master branch."""
        out = self.git("ls-remote", "--exit-code", "origin", "refs/heads/master", check=False)
        if out.returncode == 2:
            return None
        if out.returncode != 0:
            raise RuntimeError(f"cannot read {self.url}:\n{out.stderr.strip()}")
        return out.stdout.split()[0]

    def fetch_master(self):
        """Gets the history of the master branch, with no file contents. Returns its commit, or None."""
        master = self.master()
        if master is not None:
            self.git("fetch", "-q", "--no-tags", "--filter=blob:none", "origin", "+refs/heads/master:refs/remotes/origin/master")
        return master

    def has(self, commit):
        return self.git("cat-file", "-e", f"{commit}^{{commit}}", check=False).returncode == 0

    def get(self, commit):
        """True when the commit is in the repository; it is then in the clone too."""
        if not self.has(commit):
            self.git("fetch", "-q", "--no-tags", "--filter=blob:none", "origin", commit, check=False)
        return self.has(commit)

    def is_ancestor(self, ancestor, commit):
        return self.git("merge-base", "--is-ancestor", ancestor, commit, check=False).returncode == 0

    def count(self, old, new):
        return int(self.git("rev-list", "--count", f"{old}..{new}").stdout)

    def marks(self, master):
        """{INET commit: baseline commit} of the marks on master. A newer mark wins."""
        out = self.git("log", f"--format=%(trailers:key={MARK},valueonly,separator=%x1d)%x1e", f"--grep=^{MARK}: ", master).stdout
        marks = {}
        for record in out.split("\x1e"):
            for value in record.split("\x1d"):
                fields = value.split()
                if len(fields) == 2 and all(is_full_hash(field) for field in fields):
                    marks.setdefault(fields[0], fields[1])
        return marks

    def pull_request_heads(self):
        """{commit: number} of the heads of the pull requests of the repository. Needs git access only."""
        out = self.git("ls-remote", "origin", "refs/pull/*/head", check=False).stdout
        heads = {}
        for line in out.splitlines():
            commit, _, ref = line.partition("\t")
            match = re.fullmatch(r"refs/pull/(\d+)/head", ref.strip())
            if match:
                heads.setdefault(commit, int(match.group(1)))
        return heads

    def changed(self, old, new):
        return set(self.git("diff", "--name-only", "--no-renames", old, new).stdout.split("\n")) - {""}

    def tree(self, commit):
        return self.git("rev-parse", f"{commit}^{{tree}}").stdout.strip()

    def step(self, pin, excluded):
        """[(commit, parent)] of the commits of the pin that no excluded commit reaches, the oldest first."""
        out = self.git("rev-list", "--reverse", "--topo-order", "--parents", pin, "--not", *excluded).stdout
        commits = [line.split() for line in out.splitlines()]
        merges = [fields[0] for fields in commits if len(fields) != 2]
        if merges:
            raise LandError(f"the baseline branch of {short(pin)} has the merge commit {short(merges[0])}; a baseline branch is a line")
        return [(fields[0], fields[1]) for fields in commits]

    def replay(self, commit, parent, onto):
        """The change parent..commit made again on onto: the tree of a merge with no conflict."""
        out = self.git("merge-tree", "--write-tree", "--name-only", f"--merge-base={parent}", onto, commit, check=False)
        if out.returncode != 0:
            raise LandError(f"the baseline commit {short(commit)} does not apply to {short(onto)} with no conflict:\n{out.stdout.strip()}")
        return out.stdout.split("\n", 1)[0].strip()

    def commit_tree(self, tree, parent, message, like=None):
        env = dict(os.environ, **LANDING_IDENTITY)
        if like:
            name, email, date = self.git("log", "-1", "--format=%an%n%ae%n%aI", like).stdout.splitlines()
            env.update(GIT_AUTHOR_NAME=name, GIT_AUTHOR_EMAIL=email, GIT_AUTHOR_DATE=date)
        else:
            env.update(GIT_AUTHOR_NAME=LANDING_IDENTITY["GIT_COMMITTER_NAME"], GIT_AUTHOR_EMAIL=LANDING_IDENTITY["GIT_COMMITTER_EMAIL"])
        return self.git("commit-tree", tree, "-p", parent, env=env, input=message).stdout.strip()

    def message(self, commit):
        """The raw message: the text after the first empty line of the commit object."""
        return self.git("cat-file", "commit", commit).stdout.split("\n\n", 1)[1]


def push(remote, repository, commit):
    """Moves master to the commit by a fast-forward. Returns an error text, or None after a push."""
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
        out = subprocess.run(["git", "push", "--quiet", url, f"{commit}:refs/heads/master"], cwd=remote.folder, env=env,
                             capture_output=True, text=True)
    if out.returncode != 0:
        return f"the push failed:\n{out.stderr.strip()}"
    return None


def pull_request_url(repository, number):
    return f"https://github.com/{repository}/pull/{number}" if GITHUB_NAME.fullmatch(repository) else f"#{number} of {repository}"


def landing_commits(head, kind, marks):
    """[(INET commit, pin)] of the cause commits of the kind on the first-parent line of head after the
    newest commit that a mark names, the oldest first. The line ends also at a commit of the ref form."""
    found = []
    for commit in git("rev-list", "--first-parent", head).stdout.split():
        if commit in marks:
            break
        entry = read_baselines(commit).get(kind)
        if not is_trailer_form(entry):
            break
        values = pin_trailers(commit, entry["trailer"])
        if len(values) > 1 or (values and not is_full_hash(values[0])):
            raise LandError(f"{short(commit)} has no valid trailer {entry['trailer']}; run check-baselines.sh on it")
        if values:
            found.append((commit, values[0]))
    return list(reversed(found))


def land(remote, master, commits):
    """Lands the steps of the cause commits on master. Returns (new head, [(INET commit, landed baseline
    commit)], [pins of rebased steps], the error at the stop or None). The new head is master when no
    step lands."""
    head, landed, pairs, rebased = master, {}, [], []
    for commit, pin in commits:
        try:
            if not remote.get(pin):
                raise LandError(f"{short(commit)} pins {short(pin)}, which is not in the baseline repository")
            if pin in landed or remote.is_ancestor(pin, master):
                pairs.append((commit, landed.get(pin, pin)))
                continue
            step = remote.step(pin, [master, *landed])
            base = step[0][1]
            onto = landed.get(base, base)
            if onto == head and base == onto:
                for baseline_commit, _ in step:
                    landed[baseline_commit] = baseline_commit
                head = pin
                print(f"    {short(commit)}: a fast-forward over {len(step)} commit(s) to {short(pin)}")
            else:
                common = sorted(remote.changed(base, pin) & remote.changed(onto, head))
                if common:
                    raise LandError(f"{short(commit)} pins {short(pin)}, and since the base {short(base)} of its baseline branch, "
                                    f"master also changes {', '.join(common)}; record these results again on the head of master")
                for baseline_commit, parent in step:
                    head = remote.commit_tree(remote.replay(baseline_commit, parent, head), head, remote.message(baseline_commit), like=baseline_commit)
                    landed[baseline_commit] = head
                rebased.append(pin)
                print(f"    {short(commit)}: a rebase of {len(step)} commit(s) of {short(pin)} as {short(head)}")
            pairs.append((commit, landed[pin]))
        except LandError as error:
            return head, pairs, rebased, f"{short(commit)}: {error}"
    return head, pairs, rebased, None


def mark_message(pairs, pull_requests):
    last = pairs[-1][0]
    subject = git("log", "-1", "--format=%s", last).stdout.strip()
    lines = [f"{MARK}: {commit} {baseline}" for commit, baseline in pairs]
    lines += [f"{PULL_REQUEST}: #{number}" for number in pull_requests]
    return f"Baseline of INET {short(last)}: {subject}\n\n" + "\n".join(lines) + "\n"


def land_trailer_kind(remote, kind, entry, commit, target, do_push):
    """The trailer form of one kind. target is the repository to push to. Returns the number of failures."""
    repository = entry["repository"]
    master = remote.fetch_master()
    if master is None:
        print(f"  {kind}: FAIL: {repository} has no master branch")
        return 1
    marks = remote.marks("refs/remotes/origin/master")
    try:
        commits = landing_commits(commit, kind, marks)
    except LandError as error:
        print(f"  {kind}: FAIL: {error}")
        return 1
    if not commits:
        print(f"  {kind}: master of {repository} holds the baseline of {short(commit)}; no cause commit is after the newest mark")
        return 0
    print(f"  {kind}: {len(commits)} cause commit(s) to land on master of {repository} at {short(master)}")
    head, pairs, rebased, error = land(remote, master, commits)
    failures = 0
    if pairs:
        heads = remote.pull_request_heads()
        pull_requests = sorted({heads[pin] for pin in rebased if pin in heads})
        head = remote.commit_tree(remote.tree(head), head, mark_message(pairs, pull_requests))
        print(f"  {kind}: the mark {short(head)} names {len(pairs)} commit(s)" + (f" and the pull request(s) {', '.join(f'#{n}' for n in pull_requests)}" if pull_requests else ""))
        if not do_push:
            print(f"  {kind}: FAIL: master of {repository} does not hold the baseline of {short(commit)}; run the tool with --push")
            failures += 1
        else:
            push_error = push(remote, target, head)
            if push_error is not None:
                print(f"  {kind}: FAIL: {push_error}")
                failures += 1
            else:
                print(f"  {kind}: pushed master of {repository} from {short(master)} to {short(head)}")
                for number in pull_requests:
                    # GitHub marks a pull request as merged only when its head is on master
                    print(f"  {kind}: a maintainer closes the pull request {pull_request_url(target, number)}: its step landed by a rebase")
    if error is not None:
        print(f"  {kind}: FAIL: the landing stops at {error}")
        print(f"  {kind}: repair it by hand: run the moved tests of the common files at that commit, commit their results on the head of master, "
              f"add a mark that names the commit, and push; or the next run tries the commit again")
        failures += 1
    return failures


def land_ref_kind(remote, kind, pin, repository, target, do_push):
    """The ref form of one kind. target is the repository to push to. Returns the number of failures."""
    master = remote.master()
    if not remote.get(pin):
        print(f"  {kind}: FAIL: the pin {pin[:10]} is not in {repository}; push the baseline commit first")
        return 1
    if master is None or not remote.get(master):
        print(f"  {kind}: FAIL: {repository} has no master branch")
        return 1
    if master == pin:
        print(f"  {kind}: master of {repository} is at the pin {pin[:10]}")
        return 0
    if remote.is_ancestor(master, pin):
        behind = remote.count(master, pin)
        command = f"git push git@github.com:{repository}.git {pin}:refs/heads/master"
        if not do_push:
            print(f"  {kind}: FAIL: master of {repository} is {behind} commit(s) behind the pin {pin[:10]}; move it: {command}")
            return 1
        error = push(remote, target, pin)
        if error is not None:
            print(f"  {kind}: FAIL: master of {repository} is {behind} commit(s) behind the pin {pin[:10]}; {error}")
            print(f"  {kind}: move it by hand: {command}")
            return 1
        print(f"  {kind}: moved master of {repository} by a fast-forward over {behind} commit(s) to the pin {pin[:10]}")
        return 0
    if remote.is_ancestor(pin, master):
        print(f"  {kind}: FAIL: master of {repository} is {remote.count(pin, master)} commit(s) ahead of the pin {pin[:10]}; "
              f"no INET commit pins them: pin them in an INET commit, or move master back by hand")
        return 1
    print(f"  {kind}: FAIL: master of {repository} ({master[:10]}) and the pin {pin[:10]} are on different lines; "
          f"the baseline branch must stand on master")
    return 1


def check_kind(remote, kind, pin):
    """Whether the baseline branch of the pin can land on master. Returns (passed, text)."""
    master = remote.fetch_master()
    if master is None:
        return False, f"{remote.url} has no master branch"
    if not remote.get(pin):
        return False, f"the pin {short(pin)} is not in the baseline repository"
    if remote.is_ancestor(pin, master):
        return True, f"the pin {short(pin)} is on master already"
    base = remote.git("merge-base", pin, master).stdout.strip()
    try:
        step = remote.step(pin, [master])
        common = sorted(remote.changed(base, pin) & remote.changed(base, master))
        if common:
            return False, f"overlap with master since {short(base)}: {', '.join(common)}"
        head = master
        for baseline_commit, parent in step:
            # a tree with no commit: the check keeps nothing
            head = remote.commit_tree(remote.replay(baseline_commit, parent, head), head, "check")
    except LandError as error:
        return False, str(error)
    return True, f"{len(step)} commit(s) of {short(pin)} apply to master {short(master)}"


def check_main(argv, sources_of):
    parser = argparse.ArgumentParser(prog="baseline_masters.py check")
    parser.add_argument("commit")
    parser.add_argument("--base", help="the head of INET master (default: the merge base of COMMIT and origin/master)")
    parser.add_argument("--baseline-repository", action="append", default=[], metavar="KIND=URL")
    args = parser.parse_args(argv)
    sources = sources_of(args.baseline_repository)
    if sources is None:
        return 2
    try:
        commit = git("rev-parse", "--verify", f"{args.commit}^{{commit}}").stdout.strip()
        base = git("rev-parse", "--verify", f"{args.base}^{{commit}}").stdout.strip() if args.base else git("merge-base", commit, "origin/master").stdout.strip()
        # the entries of the base, as for the gate: a series cannot send the check to another repository
        baselines = read_baselines(base)
        series = git("log", "--format=%H", "--first-parent", f"{base}..{commit}").stdout.split()
    except (PinError, RuntimeError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 2
    print(f"== can the baselines of {short(base)}..{short(commit)} land ==")
    failures, texts = 0, []
    with tempfile.TemporaryDirectory() as cache:
        for kind, entry in sorted(baselines.items()):
            if not is_trailer_form(entry):
                print(f"  {kind}: the ref form; a conflict of the pin shows in inet.opp")
                continue
            pin = next((values[-1] for values in (pin_trailers(c, entry["trailer"]) for c in series) if values), None)
            if pin is None:
                print(f"  {kind}: no pin trailer in the series")
                continue
            if not is_full_hash(pin):
                passed, text = False, f"the pin {pin!r} is not a full commit hash"
            else:
                try:
                    remote = Remote(fetch_url(sources.get(kind, entry["repository"])), os.path.join(cache, kind))
                    passed, text = check_kind(remote, kind, pin)
                except RuntimeError as error:
                    print(f"error: {error}", file=sys.stderr)
                    return 2
            print(f"  {kind}: {'PASS' if passed else 'FAIL'}: {text}")
            failures += not passed
            texts.append(f"{kind}: {text}")
    print()
    print("RESULT: " + ("; ".join(texts) if texts else "no baseline change"))
    return 1 if failures else 0


def main(argv=None):
    argv = sys.argv[1:] if argv is None else argv

    def sources_of(values):
        sources = dict(value.split("=", 1) for value in values if "=" in value)
        if len(sources) != len(values):
            print("error: --baseline-repository needs KIND=URL", file=sys.stderr)
            return None
        return sources

    if argv and argv[0] == "check":
        return check_main(argv[1:], sources_of)
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("commit", nargs="?", default="HEAD")
    parser.add_argument("--push", action="store_true", help="move master of a baseline repository to the baseline of COMMIT")
    parser.add_argument("--baseline-repository", action="append", default=[], metavar="KIND=URL",
                        help="another URL or path for the baseline repository of a test kind")
    args = parser.parse_args(argv)
    sources = sources_of(args.baseline_repository)
    if sources is None:
        return 2
    try:
        commit = git("rev-parse", "--verify", f"{args.commit}^{{commit}}").stdout.strip()
        baselines = read_baselines(commit)
    except (PinError, RuntimeError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 2
    print(f"== the baseline masters and the baselines of {commit[:10]} ==")
    failures = 0
    with tempfile.TemporaryDirectory() as cache:
        for kind, entry in sorted(baselines.items()):
            repository = entry["repository"]
            try:
                if is_trailer_form(entry):
                    remote = Remote(fetch_url(sources.get(kind, repository)), os.path.join(cache, kind))
                    failures += land_trailer_kind(remote, kind, entry, commit, sources.get(kind, repository), args.push)
                    continue
                pin = entry.get("ref")
                if not is_full_hash(pin):
                    print(f"  {kind}: no full-hash pin; nothing to check")
                    continue
                remote = Remote(fetch_url(sources.get(kind, repository)), os.path.join(cache, kind))
                failures += land_ref_kind(remote, kind, pin, repository, sources.get(kind, repository), args.push)
            except (PinError, RuntimeError) as error:
                print(f"error: {error}", file=sys.stderr)
                return 2
    print()
    print("FAIL: see TR-BASELINE-PIN in doc/project/rule/testing.md." if failures else "PASS: each baseline master holds its baseline.")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
