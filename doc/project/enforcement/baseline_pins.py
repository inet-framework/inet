#!/usr/bin/env python3
"""Reads the baseline pins of INET commits.

inet.opp declares a baseline repository for some test kinds (statistical, chart), in the baseline
entry of the kind in test_parameters. The entry gives the pin, the commit of the baseline repository
that the INET commit passes with (TR-BASELINE-PIN), in one of two forms:

  ref      the ref field of the entry is the pin; a commit that moves a result changes it
  trailer  the trailer field of the entry is the key of a commit trailer, for example
           Statistical-Baseline; a commit that moves a result has the trailer with the pin, and
           inet.opp does not change. After the landing, a mark on the master branch of the
           baseline repository names the baseline of the landed commit (baseline_masters.py).

This module reads the entries and the pin trailers of a commit from git objects only. It never runs
inet.opp, so a gate can use it on the content of a pull request.

It also writes new pins into a series, after a baseline branch changed. A baseline commit gets a new
hash when its branch is rebased onto the baseline of another pull request that landed, and when its
message or its values change. Then the pins of the series name commits that are gone. The rebase
command makes the commits of the series again on the new base, with the same messages, authors and
changes. It gives the pin moves of each test kind the commits of the baseline branch above the pin
of the new base, in order: a pin move takes as many commits as it took before, which is one commit
when the old commits are not in the clone. It writes git objects only; the files of the work tree change at the end,
when the branch moves, and only if no local change is lost.

For the trailer form, the retrail command writes new pins into the pin trailers of a series, after a
baseline commit changed: a new record or new values give a new hash. A pin takes as many commits of
the baseline branch as it took before, as for rebase. The convert command changes a series of the
ref form into a series of the trailer form, on an UPSTREAM of the trailer form: it takes the ref
changes out of inet.opp and gives each commit that moved a ref the pin trailer with that ref.

Usage, from the repository root:
  baseline_pins.py show [COMMIT]    the baseline entries of the commit (default HEAD)
  baseline_pins.py moved COMMIT     the test kinds whose pin the commit moves
  baseline_pins.py rebase UPSTREAM [--baseline KIND=REVISION]... [--baseline-repository KIND=PATH]...
                                    make the commits UPSTREAM..HEAD again on UPSTREAM with new pins;
                                    the baseline branch of a kind is REVISION (default HEAD) in the
                                    clone at PATH (default: the folder of the baseline entry)
  baseline_pins.py retrail UPSTREAM [--baseline KIND=REVISION]... [--baseline-repository KIND=PATH]...
                                    make the commits UPSTREAM..HEAD again with new pin trailers
  baseline_pins.py convert UPSTREAM make the commits UPSTREAM..HEAD of the ref form again on UPSTREAM,
                                    which has the trailer form

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


def is_trailer_form(entry):
    """True when the baseline entry names its pins with a commit trailer, not with a ref."""
    return bool((entry or {}).get("trailer")) and not (entry or {}).get("ref")


def pin_trailers(commit, key, cwd=None):
    """The values of the pin trailer with the key in the message of the commit."""
    out = git("log", "-1", f"--format=%(trailers:key={key},valueonly,unfold,separator=%x00)", commit, cwd=cwd).stdout
    return [value.strip() for value in out.split("\0") if value.strip()]


def trailer_pins(commit, baselines, cwd=None):
    """{kind: pin} of the pin trailers of the commit, for the kinds of the trailer form in baselines."""
    pins = {}
    for kind, entry in baselines.items():
        if is_trailer_form(entry):
            values = pin_trailers(commit, entry["trailer"], cwd)
            if values:
                pins[kind] = values[-1]
    return pins


def moved_pins(commit, cwd=None):
    """[(kind, old ref, new ref)] for the pins of the commit that differ from the pins of its first
    parent. A pin that the commit adds or removes has None on the other side. For the trailer form,
    the new pin is the value of the trailer, and the old pin is None: only the series gives it."""
    parent = git("rev-parse", "--verify", "--quiet", f"{commit}^", cwd=cwd, check=False).stdout.strip()
    old = read_pins(parent, cwd) if parent else {}
    new = read_pins(commit, cwd)
    moves = [(kind, old.get(kind), new.get(kind)) for kind in sorted(set(old) | set(new)) if old.get(kind) != new.get(kind)]
    return moves + [(kind, None, pin) for kind, pin in sorted(trailer_pins(commit, read_baselines(commit, cwd), cwd).items())]


def series_pin_moves(base, commits, cwd=None):
    """[(commit, parent, kind, old pin, new pin)] of each pin move of the series base..commits, in order,
    for the two forms. A pin that a commit adds is its first recording, not a move. For the trailer form,
    the old pin of the first move of a kind is None: it is the base of the baseline branch, which only
    the baseline repository gives (the merge base of the pin and the master branch there)."""
    moves = []
    previous, parent = read_baselines(base, cwd), base
    last = {}
    for commit in commits:
        current = read_baselines(commit, cwd)
        for kind, entry in sorted(current.items()):
            if is_trailer_form(entry):
                values = pin_trailers(commit, entry["trailer"], cwd)
                if values:
                    moves.append((commit, parent, kind, last.get(kind), values[-1]))
                    last[kind] = values[-1]
            else:
                old, new = (previous.get(kind) or {}).get("ref"), entry.get("ref")
                if is_full_hash(old) and is_full_hash(new) and old != new:
                    moves.append((commit, parent, kind, old, new))
        previous, parent = current, commit
    return moves


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
    """A new commit of the tree. With like, it has the author and the author date of that commit, and
    its message when no message is given."""
    env = dict(os.environ)
    if like:
        name, email, date = git("log", "-1", "--format=%an%n%ae%n%aI", like).stdout.splitlines()
        env.update(GIT_AUTHOR_NAME=name, GIT_AUTHOR_EMAIL=email, GIT_AUTHOR_DATE=date)
        if message is None:
            message = raw_message(like)
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


def replace_trailer(message, key, old, new):
    """The message with the value of the pin trailer key changed from old to new."""
    line = re.compile(rf"^({re.escape(key)}:[ \t]*){re.escape(old)}[ \t]*$", re.MULTILINE | re.IGNORECASE)
    if len(line.findall(message)) != 1:
        raise RebaseError(f"the trailer {key}: {old[:9]} is not exactly one time in the message")
    return line.sub(lambda match: match.group(1) + new, message)


def add_trailer(message, key, value):
    """The message with one more trailer, in its trailer block."""
    return subprocess.run(["git", "interpret-trailers", "--if-exists", "addIfDifferent", "--trailer", f"{key}: {value}"],
                          input=message, capture_output=True, text=True, check=True).stdout


def raw_message(commit):
    """The raw message: the text after the first empty line of the commit object, with no line added."""
    return git("cat-file", "commit", commit).stdout.split("\n\n", 1)[1]


def series_commits(upstream):
    if git("status", "--porcelain", "--untracked-files=no").stdout.strip():
        raise RebaseError("the work tree has changes; commit them or set them aside first")
    base = git("rev-parse", "--verify", f"{upstream}^{{commit}}").stdout.strip()
    commits = git("rev-list", "--reverse", f"{base}..HEAD").stdout.split()
    if git("rev-list", "--merges", f"{base}..HEAD").stdout.strip():
        raise RebaseError("the series has a merge commit")
    if not commits:
        raise RebaseError(f"no commit is in {upstream}..HEAD")
    return base, commits


def master_of(folder):
    """The master branch of a baseline clone: origin/master when the clone has it, else master."""
    for name in ("refs/remotes/origin/master", "refs/heads/master"):
        out = git("rev-parse", "--verify", "--quiet", f"{name}^{{commit}}", cwd=folder, check=False)
        if out.returncode == 0:
            return out.stdout.strip()
    raise RebaseError(f"the baseline clone in {folder} has no master branch")


def retrail(upstream, tips, folders):
    """Makes the commits upstream..HEAD again with new pin trailers: the commits of the baseline branch
    of each kind, in order. Returns the new head and [(old commit, new commit, [(kind, old pin, new pin)])]."""
    base, commits = series_commits(upstream)
    moves = series_pin_moves(base, commits)
    new_pins = {}
    for kind in sorted({move[2] for move in moves}):
        kind_moves = [move for move in moves if move[2] == kind]
        entry = read_baselines(kind_moves[0][0])[kind]
        if not is_trailer_form(entry):
            raise RebaseError(f"the {kind} baseline has the ref form in the series; use the rebase command")
        folder = folders.get(kind) or entry.get("folder") or "."
        if not os.path.exists(os.path.join(folder, ".git")):
            raise RebaseError(f"the {kind} baseline has no clone in {folder}; give one with --baseline-repository {kind}=PATH")
        tip = git("rev-parse", "--verify", f"{tips.get(kind, 'HEAD')}^{{commit}}", cwd=folder).stdout.strip()
        master = master_of(folder)
        start = git("merge-base", tip, master, cwd=folder).stdout.strip()
        chain = git("rev-list", "--reverse", "--first-parent", f"{start}..{tip}", cwd=folder).stdout.split()
        spans = []
        for index, (_, _, _, old, new) in enumerate(kind_moves):
            if old is None:
                present = git("cat-file", "-e", f"{new}^{{commit}}", cwd=folder, check=False).returncode == 0
                old = git("merge-base", new, master, cwd=folder).stdout.strip() if present else None
            spans.append(span(folder, old, new) if old else 1)
        if sum(spans) != len(chain):
            raise RebaseError(f"the series has {len(kind_moves)} {kind} pin trailer(s) over {sum(spans)} baseline commit(s), and the baseline branch has {len(chain)} commit(s) above master; "
                              f"fetch the clone first (git -C {folder} fetch origin), and start the baseline branch on origin/master")
        new_pins[kind] = {commit: chain[sum(spans[:index + 1]) - 1] for index, (commit, _, _, _, _) in enumerate(kind_moves)}
    head, result = base, []
    for commit in commits:
        message, changed = raw_message(commit), []
        for kind, pins in new_pins.items():
            if commit in pins:
                old = next(move[4] for move in moves if move[0] == commit and move[2] == kind)
                if pins[commit] != old:
                    message = replace_trailer(message, read_baselines(commit)[kind]["trailer"], old, pins[commit])
                changed.append((kind, old, pins[commit]))
        tree = git("rev-parse", f"{commit}^{{tree}}").stdout.strip()
        head = commit_tree(tree, [head], like=commit, message=message)
        result.append((commit, head, changed))
    git("reset", "-q", "--keep", head)
    return head, result


def convert(upstream):
    """Makes the commits upstream..HEAD of the ref form again on upstream, which has the trailer form.
    Each pin move of the ref form becomes a pin trailer with the same pin, and the series no longer
    changes the refs in inet.opp. Returns the new head and [(old commit, new commit, [(kind, None, pin)])]."""
    new_base, commits = series_commits(upstream)
    upstream_entries = read_baselines(new_base)
    keys = {kind: entry["trailer"] for kind, entry in upstream_entries.items() if is_trailer_form(entry)}
    if not keys:
        raise RebaseError(f"{upstream} has no baseline entry of the trailer form")
    head, result = new_base, []
    with tempfile.TemporaryDirectory() as folder:
        index = os.path.join(folder, "index")
        for commit in commits:
            parent = git("rev-parse", f"{commit}^").stdout.strip()
            parent_pins, commit_pins = read_pins(parent), read_pins(commit)
            # the commit gets the refs of its parent, so the merge sees no change of a ref
            text = git("show", f"{commit}:{OPP_FILE}", check=False)
            neutral = commit
            if text.returncode == 0:
                tree = tree_with_opp(f"{commit}^{{tree}}", substitute(text.stdout, commit_pins, parent_pins), index)
                neutral = commit_tree(tree, [], message="ref-neutral copy")
            merged = git("merge-tree", "--write-tree", f"--merge-base={parent}", head, neutral, check=False)
            if merged.returncode != 0:
                subject = git("log", "-1", "--format=%s", commit).stdout.strip()
                raise RebaseError(f"{commit[:9]} '{subject}' does not apply to {upstream} without a conflict; rebase the series with git first, keep the ref of {upstream} in {OPP_FILE}, then run this command again")
            message, changed = raw_message(commit), []
            for kind, key in sorted(keys.items()):
                old, new = parent_pins.get(kind), commit_pins.get(kind)
                if is_full_hash(old) and is_full_hash(new) and old != new:
                    message = add_trailer(message, key, new)
                    changed.append((kind, None, new))
            head = commit_tree(merged.stdout.split()[0], [head], like=commit, message=message)
            result.append((commit, head, changed))
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


def series_main(name, argv):
    """The commands retrail and convert."""
    parser = argparse.ArgumentParser(prog=f"baseline_pins.py {name}")
    parser.add_argument("upstream")
    if name == "retrail":
        parser.add_argument("--baseline", action="append", default=[], metavar="KIND=REVISION")
        parser.add_argument("--baseline-repository", action="append", default=[], metavar="KIND=PATH")
    args = parser.parse_args(argv)
    try:
        if name == "retrail":
            tips = dict(value.split("=", 1) for value in args.baseline if "=" in value)
            folders = dict(value.split("=", 1) for value in args.baseline_repository if "=" in value)
            if len(tips) != len(args.baseline) or len(folders) != len(args.baseline_repository):
                parser.error("--baseline needs KIND=REVISION, and --baseline-repository needs KIND=PATH")
            head, result = retrail(args.upstream, tips, folders)
        else:
            head, result = convert(args.upstream)
    except RebaseError as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    for old, new, changed in result:
        print(f"{old[:9]} -> {new[:9]} {git('log', '-1', '--format=%s', new).stdout.strip()}")
        for kind, old_pin, new_pin in changed:
            print(f"    {kind} pin trailer {(old_pin or 'none')[:9]} -> {new_pin[:9]}")
    print(f"HEAD is now {head[:9]}. Nothing is pushed.")
    return 0


def main(argv=None):
    argv = sys.argv[1:] if argv is None else argv
    try:
        if argv and argv[0] == "rebase":
            return rebase_main(argv[1:])
        if argv and argv[0] in ("retrail", "convert"):
            return series_main(argv[0], argv[1:])
        if len(argv) in (1, 2) and argv[0] == "show":
            commit = argv[1] if len(argv) == 2 else "HEAD"
            baselines = read_baselines(commit)
            trailers = trailer_pins(commit, baselines)
            for kind, baseline in baselines.items():
                if is_trailer_form(baseline):
                    print(kind, baseline.get("repository"), f"trailer {baseline['trailer']}", trailers.get(kind) or "-", baseline.get("folder"))
                else:
                    print(kind, baseline.get("repository"), baseline.get("ref"), baseline.get("folder"))
            return 0
        if len(argv) == 2 and argv[0] == "moved":
            # a pin that a commit adds to the ref form is its first recording, not a move; a pin trailer is a move
            trailers = trailer_pins(argv[1], read_baselines(argv[1]))
            print(" ".join(kind for kind, old, new in moved_pins(argv[1]) if new and (old or kind in trailers)))
            return 0
        print(__doc__, file=sys.stderr)
        return 2
    except (PinError, RuntimeError, subprocess.CalledProcessError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
