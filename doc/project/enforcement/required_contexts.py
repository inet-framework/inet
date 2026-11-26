#!/usr/bin/env python3
"""W1, W2, W4 and W5 of plan/pending/master-guards.md over a range of commits: each required context
of the ruleset of master still reports on a pull request after the change.

A required context comes from a job of a workflow that runs on pull requests into master, or from a
commit status that a workflow posts with a literal context, such as "Baseline merge". The gate
computes the contexts at the base and at the head of the range from the workflow files, as GitHub
names them: the name of the job, or its id, and for a job with a matrix the values of each
combination in parentheses, for example "fingerprint-tests (debug, 0)".

  W2, W4  VIOLATION: a required context that the base reports and the head does not: the range
          renames or removes it, and an admin must change the ruleset at the same landing
  W1      VIOLATION: the range gives a required job with a matrix a job-level `if:` (GitHub then
          reports the skipped job under the name without the matrix values), or it gives the
          workflow of a required job a path filter in its pull request trigger
          note: the range adds or changes the job-level `if:` of another required job, which must
          be true for each event of a pull request
  note    a required context that neither the base nor the head reports, a job whose names the
          gate cannot compute, and a list in doc/project/enforcement/README.md that differs from
          the ruleset (W5)

The required contexts come from --required-file (one context on each line), or else from the rules
of master through `gh api`. Without them the gate gives a note and passes. It reads the workflows
with PyYAML. It reads only git objects, so the trusted pull-request workflow can
run it on the head of a pull request.

Usage (from the INET repository root):
  doc/project/enforcement/check-required-contexts.sh origin/master..HEAD
  doc/project/enforcement/check-required-contexts.sh BASE..HEAD --required-file contexts.txt

Exit status 0 = clean, 1 = violations, 2 = invalid usage or the gate cannot run.
"""
import argparse
import itertools
import os
import re
import subprocess
import sys
from dataclasses import dataclass, field

WORKFLOWS = ".github/workflows"
README = "doc/project/enforcement/README.md"
README_HEADING = "## Required checks of master"
RULES = "plan/pending/master-guards.md"
STATUS = re.compile(r"""\bcontext=["']([^"'$\n]+)["']""")
MATRIX_EXPRESSION = re.compile(r"\$\{\{\s*matrix\.([A-Za-z0-9_-]+)\s*\}\}")
PR_EVENTS = ("pull_request", "pull_request_target")


class CannotRun(Exception):
    pass


def git(*args):
    out = subprocess.run(["git", *args], capture_output=True, text=True)
    if out.returncode != 0:
        raise CannotRun(f"git {' '.join(args)}: {out.stderr.strip()}")
    return out.stdout


def load_yaml(text):
    """The data of a YAML text. The system Python of the GitHub runner has no PyYAML, so each job that
    runs this gate installs it."""
    try:
        import yaml
    except ImportError:
        raise CannotRun("the gate needs PyYAML to read the workflows: python3 -m pip install pyyaml") from None
    return yaml.safe_load(text)


@dataclass
class Source:
    """Where a context comes from, at one commit."""
    path: str
    job: str
    kind: str                     # "job" or "status"
    matrix: bool = False
    job_if: str = None            # the text of the job-level if:, or None
    path_filter: bool = False     # the pull request trigger of the workflow has paths or paths-ignore


@dataclass
class Contexts:
    found: dict = field(default_factory=dict)      # context -> Source
    unknown: list = field(default_factory=list)    # (path, job, reason) of jobs with names the gate cannot compute


def triggers(document):
    """The triggers of a workflow as a dictionary. PyYAML reads the key `on` as True."""
    on = document.get("on", document.get(True)) if isinstance(document, dict) else None
    if isinstance(on, str):
        return {on: None}
    if isinstance(on, list):
        return {event: None for event in on}
    return on if isinstance(on, dict) else {}


def pull_request_trigger(on):
    """(the workflow runs on pull requests into master, the pull request trigger has a path filter)."""
    runs, filtered = False, False
    for event in PR_EVENTS:
        if event not in on:
            continue
        config = on[event] or {}
        branches, ignored = config.get("branches"), config.get("branches-ignore") or []
        if (branches is None or any(b in ("master", "*", "**") for b in branches)) and "master" not in ignored:
            runs = True
            filtered = filtered or "paths" in config or "paths-ignore" in config
    return runs, filtered


def combinations(matrix):
    """The value lists of the matrix combinations in key order, or None when the gate cannot compute them."""
    if not isinstance(matrix, dict) or "include" in matrix:
        return None
    keys = [key for key in matrix if key != "exclude"]
    if not keys or any(not isinstance(matrix[key], list) for key in keys):
        return None
    excluded = matrix.get("exclude") or []
    result = []
    for values in itertools.product(*(matrix[key] for key in keys)):
        combination = dict(zip(keys, values))
        if any(all(combination.get(k) == v for k, v in rule.items()) for rule in excluded):
            continue
        result.append(combination)
    return result


def text(value):
    return ("true" if value else "false") if isinstance(value, bool) else str(value)


def job_names(job_id, job):
    """The check names of a job, or a reason why the gate cannot compute them."""
    if "uses" in job:
        return None, "a reusable workflow"
    name = job.get("name", job_id)
    if not isinstance(name, str):
        return None, "a name that is not a text"
    matrix = (job.get("strategy") or {}).get("matrix")
    if matrix is None:
        if "${{" in name:
            return None, "an expression in the name"
        return [name], None
    combos = combinations(matrix)
    if combos is None:
        return None, "a matrix with include or with computed values"
    if "${{" in name:
        names = []
        for combo in combos:
            filled = MATRIX_EXPRESSION.sub(lambda m: text(combo.get(m.group(1), m.group(0))), name)
            if "${{" in filled:
                return None, "an expression in the name"
            names.append(filled)
        return names, None
    return [f"{name} ({', '.join(text(v) for v in combo.values())})" for combo in combos], None


def contexts_at(commit):
    """The contexts that the workflows at a commit report on a pull request into master."""
    result = Contexts()
    listing = git("ls-tree", "--name-only", f"{commit}:{WORKFLOWS}") if workflows_exist(commit) else ""
    for name in sorted(listing.split()):
        if not name.endswith((".yml", ".yaml")):
            continue
        path = f"{WORKFLOWS}/{name}"
        content = git("show", f"{commit}:{path}")
        for context in STATUS.findall(content):
            result.found.setdefault(context, Source(path, "", "status"))
        try:
            document = load_yaml(content)
        except CannotRun:
            raise
        except Exception as error:  # a workflow that GitHub cannot read reports nothing
            result.unknown.append((path, "", f"the file cannot be read: {str(error).splitlines()[0] if str(error) else error}"))
            continue
        runs, filtered = pull_request_trigger(triggers(document))
        if not runs or not isinstance(document.get("jobs"), dict):
            continue
        for job_id, job in document["jobs"].items():
            if not isinstance(job, dict):
                continue
            names, reason = job_names(job_id, job)
            if names is None:
                result.unknown.append((path, job_id, reason))
                continue
            has_matrix = (job.get("strategy") or {}).get("matrix") is not None
            job_if = job.get("if")
            for context in names:
                result.found[context] = Source(path, job_id, "job", has_matrix,
                                               None if job_if is None else str(job_if), filtered)
    return result


def workflows_exist(commit):
    return subprocess.run(["git", "cat-file", "-e", f"{commit}:{WORKFLOWS}"], capture_output=True).returncode == 0


def required_contexts(args):
    """The required contexts of master, or raise CannotRun with the reason."""
    if args.required_file:
        try:
            with open(args.required_file, encoding="utf-8") as f:
                return [line.strip() for line in f if line.strip() and not line.startswith("#")]
        except OSError as error:
            raise CannotRun(f"cannot read {args.required_file}: {error}") from None
    repository = args.repository or os.environ.get("GITHUB_REPOSITORY") or "inet-framework/inet"
    query = '.[] | select(.type == "required_status_checks") | .parameters.required_status_checks[].context'
    try:
        out = subprocess.run(["gh", "api", f"repos/{repository}/rules/branches/master", "--jq", query],
                             capture_output=True, text=True, timeout=60)
    except (FileNotFoundError, subprocess.TimeoutExpired) as error:
        raise CannotRun(f"gh api: {error}") from None
    if out.returncode != 0:
        raise CannotRun(f"gh api: {out.stderr.strip()}")
    return [line.strip() for line in out.stdout.splitlines() if line.strip()]


def readme_list(commit):
    """The contexts that the section of the README at a commit lists, or None without the section."""
    out = subprocess.run(["git", "show", f"{commit}:{README}"], capture_output=True, text=True)
    if out.returncode != 0 or README_HEADING not in out.stdout:
        return None
    section = out.stdout.split(README_HEADING, 1)[1].split("\n## ", 1)[0]
    return [m.group(1) for m in re.finditer(r"^\s*[-*] `([^`]+)`", section, re.M)]


def where(source):
    return f"{source.path}, job {source.job}" if source.kind == "job" else f"a commit status in {source.path}"


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("range", help="BASE..HEAD")
    parser.add_argument("--required-file", help="a file with one required context on each line")
    parser.add_argument("--repository", help="OWNER/NAME for gh api (default: $GITHUB_REPOSITORY or inet-framework/inet)")
    args = parser.parse_args(argv)
    if ".." not in args.range:
        print(f"error: '{args.range}' is not a range BASE..HEAD", file=sys.stderr)
        return 2
    base, head = args.range.split("..", 1)
    violations, notes = [], []
    print(f"== the series: {args.range} ==")
    try:
        base, head = git("rev-parse", base).strip(), git("rev-parse", head).strip()
        before, after = contexts_at(base), contexts_at(head)
    except CannotRun as error:
        print(f"error: {error}", file=sys.stderr)
        return 2
    try:
        required = required_contexts(args)
    except CannotRun as error:
        print()
        print("== W2 and W4: each required context of master still reports on a pull request ==")
        print(f"  note: the required contexts of master are unknown, so the gate checks nothing: {error}")
        return 0

    print()
    print("== W2 and W4: each required context of master still reports on a pull request ==")
    print(f"  {len(required)} required context(s)")
    for context in required:
        if context in after.found:
            continue
        source = before.found.get(context)
        unknown_now = [(p, j, r) for p, j, r in after.unknown if source and (p, j) == (source.path, source.job)]
        if unknown_now:
            notes.append(f"the gate cannot compute the names of the job '{source.job}' of {source.path} at the head "
                         f"({unknown_now[0][2]}), so it cannot see if the required context '{context}' still reports")
        elif source:
            violations.append(f"the series renames or removes the required context '{context}' ({where(before.found[context])}): "
                              f"an admin changes the ruleset of master at the same landing (W2 of {RULES})")
        elif after.unknown:
            notes.append(f"no workflow is known to report the required context '{context}', and the gate cannot compute "
                         f"the names of {len(after.unknown)} job(s): " + "; ".join(f"{p} {j} ({r})" for p, j, r in after.unknown))
        else:
            notes.append(f"no workflow at the base or at the head reports the required context '{context}': each pull request waits for it")
    for line in violations:
        print(f"  VIOLATION: {line}")
    for line in notes:
        print(f"  note: {line}")
    if not violations and not notes:
        print("  ok")

    print()
    print("== W1: a required job reports a result on each pull request ==")
    quiet = True
    for context in required:
        now, then = after.found.get(context), before.found.get(context)
        if now is None or now.kind != "job":
            continue
        if now.matrix and now.job_if is not None and (then is None or then.job_if is None):
            violations.append(f"the series gives the job '{now.job}' of {now.path} a job-level if:, and the job has a matrix: "
                              f"a skipped job reports under the name without the matrix values, so '{context}' never reports (W1)")
            print(f"  VIOLATION: {violations[-1]}")
            quiet = False
        elif now.job_if is not None and (then is None or then.job_if != now.job_if):
            print(f"  note: the series adds or changes the job-level if: of the required job '{now.job}' of {now.path}: "
                  f"it must be true for each event of a pull request, because a skipped job counts as passed (W1)")
            quiet = False
        if now.path_filter and (then is None or not then.path_filter):
            violations.append(f"the series gives {now.path} a path filter in its pull request trigger: the required context "
                              f"'{context}' then never reports for a pull request outside the paths (W1)")
            print(f"  VIOLATION: {violations[-1]}")
            quiet = False
    if quiet:
        print("  ok")

    print()
    print(f"== W5: the list of the required contexts in {README} ==")
    listed = readme_list(head)
    if listed is None:
        print(f"  note: {README} has no section '{README_HEADING[3:]}'")
    elif sorted(listed) != sorted(required):
        missing, extra = sorted(set(required) - set(listed)), sorted(set(listed) - set(required))
        print(f"  note: the list differs from the ruleset of master: not listed {missing or 'none'}; listed but not required {extra or 'none'}")
    else:
        print("  ok")

    print()
    if violations:
        print(f"FAIL: {len(violations)} violation(s) of the rules for a required job ({RULES}).")
        return 1
    print("PASS: each required context of master still reports on a pull request.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
