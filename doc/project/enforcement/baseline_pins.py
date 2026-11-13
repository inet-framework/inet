#!/usr/bin/env python3
"""Reads the baseline pins of INET commits.

inet.opp declares a baseline repository for some test kinds (statistical, chart), in the baseline
entry of the kind in test_parameters. The ref field of that entry is the pin: the commit of the
baseline repository that the INET commit passes with (TR-BASELINE-PIN). This module reads the
entries of a commit from git objects only. It never runs inet.opp, so a gate can use it on the
content of a pull request.

It also writes new pins into a series, after a baseline branch changed. A baseline commit gets a new
hash when its branch is rebased onto the baseline of another pull request that landed, and when its
message or its values change. Then the pins of the series name commits that are gone. The rebase
command makes the commits of the series again on the new base, with the same messages, authors and
changes. It gives the pin moves of each test kind the commits of the baseline branch above the pin
of the new base, in order: a pin move takes as many commits as it took before, which is one commit
when the old commits are not in the clone. It writes git objects only; the files of the work tree change at the end,
when the branch moves, and only if no local change is lost.

Usage, from the repository root:
  baseline_pins.py show [COMMIT]    the baseline entries of the commit (default HEAD)
  baseline_pins.py moved COMMIT     the test kinds whose pin differs from the pin of the parent
  baseline_pins.py rebase UPSTREAM [--baseline KIND=REVISION]... [--baseline-repository KIND=PATH]...
                                    make the commits UPSTREAM..HEAD again on UPSTREAM with new pins;
                                    the baseline branch of a kind is REVISION (default HEAD) in the
                                    clone at PATH (default: the folder of the baseline entry)

Exit status 0 = done, 1 = the rebase cannot be done, 2 = invalid usage or an inet.opp that cannot be read.
"""
import argparse
import ast
import os
import re
import subprocess
import sys
import tempfile

OPP_FILE = "inet.opp"
FULL_HASH = re.compile(r"[0-9a-f]{40}")


class PinError(Exception):
    """inet.opp of a commit does not give its baseline entries."""


def git(*args, cwd=None, check=True):
    out = subprocess.run(["git", *args], cwd=cwd, capture_output=True, text=True)
    if check and out.returncode != 0:
        raise RuntimeError(f"git {' '.join(args)} failed:\n{out.stderr.strip()}")
    return out


def is_full_hash(ref):
    return isinstance(ref, str) and FULL_HASH.fullmatch(ref) is not None


def parse_baselines(text):
    """{kind: baseline entry} of the text of an inet.opp. The text is data: it is parsed, not run."""
    try:
        tree = ast.parse(text, mode="eval")
    except SyntaxError as error:
        raise PinError(f"{OPP_FILE} is not a Python expression: {error.msg}, line {error.lineno}") from None
    if not isinstance(tree.body, ast.Call):
        raise PinError(f"{OPP_FILE} is not one call of SimulationProject")
    for keyword in tree.body.keywords:
        if keyword.arg == "test_parameters":
            try:
                test_parameters = ast.literal_eval(keyword.value)
            except ValueError:
                raise PinError(f"test_parameters of {OPP_FILE} is not a literal") from None
            if not isinstance(test_parameters, dict):
                raise PinError(f"test_parameters of {OPP_FILE} is not a dict")
            baselines = {}
            for kind, parameters in test_parameters.items():
                if isinstance(parameters, dict) and parameters.get("baseline"):
                    if not isinstance(parameters["baseline"], dict):
                        raise PinError(f"the baseline entry of the {kind} tests is not a dict")
                    baselines[kind] = parameters["baseline"]
            return baselines
    return {}


def read_baselines(commit, cwd=None):
    """{kind: baseline entry} at the commit; empty when the commit has no inet.opp."""
    out = git("show", f"{commit}:{OPP_FILE}", cwd=cwd, check=False)
    if out.returncode != 0:
        return {}
    return parse_baselines(out.stdout)


def read_pins(commit, cwd=None):
    """{kind: ref} at the commit, for the kinds whose baseline entry has a ref."""
    return {kind: baseline["ref"] for kind, baseline in read_baselines(commit, cwd).items() if baseline.get("ref")}


def moved_pins(commit, cwd=None):
    """[(kind, old ref, new ref)] for the pins of the commit that differ from the pins of its first
    parent. A pin that the commit adds or removes has None on the other side."""
    parent = git("rev-parse", "--verify", "--quiet", f"{commit}^", cwd=cwd, check=False).stdout.strip()
    old = read_pins(parent, cwd) if parent else {}
    new = read_pins(commit, cwd)
    return [(kind, old.get(kind), new.get(kind)) for kind in sorted(set(old) | set(new)) if old.get(kind) != new.get(kind)]


class RebaseError(Exception):
    """The series cannot get its new pins."""


def baseline_chain(folder, old, tip):
    """The commits of a baseline branch above a pin, the oldest first."""
    if git("merge-base", "--is-ancestor", old, tip, cwd=folder, check=False).returncode != 0:
        raise RebaseError(f"the baseline branch {tip} in {folder} does not stand on the pin of the new base, {old[:9]}; rebase the baseline branch first")
    return git("rev-list", "--reverse", "--first-parent", f"{old}..{tip}", cwd=folder).stdout.split()


def span(folder, old, new):
    """The number of baseline commits that a pin move went over; 1 when the clone does not have the two commits."""
    if git("merge-base", "--is-ancestor", old, new, cwd=folder, check=False).returncode != 0:
        return 1
    return max(1, int(git("rev-list", "--count", f"{old}..{new}", cwd=folder).stdout))


def substitute(text, old_pins, new_pins):
    """The text of an inet.opp with each pin of old_pins replaced by the pin of the same kind in new_pins."""
    for kind, old in old_pins.items():
        new = new_pins.get(kind)
        if new and new != old:
            if text.count(old) != 1:
                raise RebaseError(f"the pin {old[:9]} is not exactly one time in {OPP_FILE}")
            text = text.replace(old, new)
    return text


def tree_with_opp(tree, text, index):
    """The tree with another text of inet.opp."""
    env = dict(os.environ, GIT_INDEX_FILE=index)
    blob = subprocess.run(["git", "hash-object", "-w", "--stdin"], input=text, capture_output=True, text=True, check=True).stdout.strip()
    subprocess.run(["git", "read-tree", tree], env=env, check=True)
    subprocess.run(["git", "update-index", "--add", "--cacheinfo", f"100644,{blob},{OPP_FILE}"], env=env, check=True)
    return subprocess.run(["git", "write-tree"], env=env, capture_output=True, text=True, check=True).stdout.strip()


def commit_tree(tree, parents, like=None, message=None):
    """A new commit of the tree. With like, it has the message, the author and the author date of that commit."""
    env = dict(os.environ)
    if like:
        name, email, date = git("log", "-1", "--format=%an%n%ae%n%aI", like).stdout.splitlines()
        env.update(GIT_AUTHOR_NAME=name, GIT_AUTHOR_EMAIL=email, GIT_AUTHOR_DATE=date)
        # the raw message: the text after the first empty line of the commit object, with no line added
        message = git("cat-file", "commit", like).stdout.split("\n\n", 1)[1]
    args = ["git", "commit-tree", tree] + [arg for parent in parents for arg in ("-p", parent)]
    return subprocess.run(args, input=message, env=env, capture_output=True, text=True, check=True).stdout.strip()


def rebase(upstream, tips, folders):
    """Makes the commits upstream..HEAD again on upstream, with new pins. Returns the new head and
    [(old commit, new commit, [(kind, old pin, new pin)])]."""
    if git("status", "--porcelain", "--untracked-files=no").stdout.strip():
        raise RebaseError("the work tree has changes; commit them or set them aside first")
    new_base = git("rev-parse", "--verify", f"{upstream}^{{commit}}").stdout.strip()
    commits = git("rev-list", "--reverse", f"{new_base}..HEAD").stdout.split()
    if git("rev-list", "--merges", f"{new_base}..HEAD").stdout.strip():
        raise RebaseError("the series has a merge commit")
    if not commits:
        raise RebaseError(f"no commit is in {upstream}..HEAD")
    base_entries = read_baselines(new_base)
    # the baseline commits that the pin moves of each kind get, in order
    chains, clones = {}, {}
    for kind, entry in base_entries.items():
        if is_full_hash(entry.get("ref")):
            folder = folders.get(kind) or entry.get("folder") or "."
            if not os.path.exists(os.path.join(folder, ".git")):
                if any(old and new for commit in commits for move_kind, old, new in moved_pins(commit) if move_kind == kind):
                    raise RebaseError(f"the {kind} baseline has no clone in {folder}; give one with --baseline-repository {kind}=PATH")
                continue
            tip = git("rev-parse", "--verify", f"{tips.get(kind, 'HEAD')}^{{commit}}", cwd=folder).stdout.strip()
            chains[kind], clones[kind] = baseline_chain(folder, entry["ref"], tip), folder
    # the new pin of each pin move: a move takes as many commits of the baseline branch as it took before
    new_pins = {}
    for kind, chain in chains.items():
        moves = [(commit, old, new) for commit in commits for k, old, new in moved_pins(commit) if k == kind and old and new]
        spans = [span(clones[kind], old, new) for _, old, new in moves]
        if sum(spans) != len(chain):
            raise RebaseError(f"the series moves the {kind} pin {len(moves)} time(s) over {sum(spans)} baseline commit(s), and the baseline branch has {len(chain)} commit(s) above the pin of the new base")
        new_pins[kind] = {commit: chain[sum(spans[:index + 1]) - 1] for index, (commit, _, _) in enumerate(moves)}
    head, current = new_base, read_pins(new_base)
    result = []
    with tempfile.TemporaryDirectory() as folder:
        index = os.path.join(folder, "index")
        for commit in commits:
            parent = git("rev-parse", f"{commit}^").stdout.strip()
            parent_pins, commit_pins = read_pins(parent), read_pins(commit)
            # take the pins out of the change: the parent and the commit get the pins of the new
            # head, so the merge sees no pin change and has no conflict in a pin
            neutral = []
            for revision, pins in ((parent, parent_pins), (commit, commit_pins)):
                text = git("show", f"{revision}:{OPP_FILE}", check=False)
                if text.returncode != 0:
                    neutral.append(revision)
                    continue
                tree = tree_with_opp(f"{revision}^{{tree}}", substitute(text.stdout, pins, current), index)
                neutral.append(commit_tree(tree, [], message="pin-neutral copy"))
            merged = git("merge-tree", "--write-tree", f"--merge-base={neutral[0]}", head, neutral[1], check=False)
            if merged.returncode != 0:
                subject = git("log", "-1", "--format=%s", commit).stdout.strip()
                raise RebaseError(f"{commit[:9]} '{subject}' does not apply to the new base without a conflict; rebase the series with git first, take either side for a pin in {OPP_FILE}, then run this command again")
            tree = merged.stdout.split()[0]
            changed = []
            for kind in chains:
                if commit in new_pins[kind]:
                    new_pin = new_pins[kind][commit]
                    changed.append((kind, commit_pins.get(kind), new_pin))
                    text = git("show", f"{tree}:{OPP_FILE}").stdout
                    tree = tree_with_opp(tree, substitute(text, {kind: current[kind]}, {kind: new_pin}), index)
                    current[kind] = new_pin
            head = commit_tree(tree, [head], like=commit)
            result.append((commit, head, changed))
    # --keep moves the branch and the files, and stops if a local change is in the way
    git("reset", "-q", "--keep", head)
    return head, result


def rebase_main(argv):
    parser = argparse.ArgumentParser(prog="baseline_pins.py rebase", description="Make the commits UPSTREAM..HEAD again on UPSTREAM, with new pins.")
    parser.add_argument("upstream")
    parser.add_argument("--baseline", action="append", default=[], metavar="KIND=REVISION")
    parser.add_argument("--baseline-repository", action="append", default=[], metavar="KIND=PATH")
    args = parser.parse_args(argv)
    tips = dict(value.split("=", 1) for value in args.baseline if "=" in value)
    folders = dict(value.split("=", 1) for value in args.baseline_repository if "=" in value)
    if len(tips) != len(args.baseline) or len(folders) != len(args.baseline_repository):
        parser.error("--baseline needs KIND=REVISION, and --baseline-repository needs KIND=PATH")
    try:
        head, result = rebase(args.upstream, tips, folders)
    except RebaseError as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    for old, new, changed in result:
        print(f"{old[:9]} -> {new[:9]} {git('log', '-1', '--format=%s', new).stdout.strip()}")
        for kind, old_pin, new_pin in changed:
            print(f"    {kind} pin {(old_pin or 'none')[:9]} -> {new_pin[:9]}")
    print(f"HEAD is now {head[:9]}. Nothing is pushed.")
    return 0


def main(argv=None):
    argv = sys.argv[1:] if argv is None else argv
    try:
        if argv and argv[0] == "rebase":
            return rebase_main(argv[1:])
        if len(argv) in (1, 2) and argv[0] == "show":
            for kind, baseline in read_baselines(argv[1] if len(argv) == 2 else "HEAD").items():
                print(kind, baseline.get("repository"), baseline.get("ref"), baseline.get("folder"))
            return 0
        if len(argv) == 2 and argv[0] == "moved":
            # a pin that a commit adds is its first recording, not a move
            print(" ".join(kind for kind, old, new in moved_pins(argv[1]) if old and new))
            return 0
        print(__doc__, file=sys.stderr)
        return 2
    except (PinError, RuntimeError, subprocess.CalledProcessError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
