#!/usr/bin/env python3
"""Run the mechanical half of the pull-request audit over one pull request: the series gates of steps
2 and 3 of doc/project/guide/review-a-pull-request.md, over merge-base(BASE, HEAD)..HEAD.

Usage, from the repository root:
  check-pull-request.sh --base origin/master --head refs/pr/1308 --pr 1308
  check-pull-request.sh --base <base-sha> --head <head-sha> --description-file body.md --markdown report.md

Without --pr or --description-file the description gate does not run, and the report says so.
--markdown also writes the report as Markdown, for a CI job summary. The report escapes the output
of each gate, because a commit message and a description are untrusted text.

Exit status 0 = clean, 1 = a gate found a violation, 2 = invalid usage or a gate could not run.
A note never changes the exit status.
"""
import argparse
import html
import os
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
GUIDE = "doc/project/guide/review-a-pull-request.md"
RULES = ["doc/project/rule/pull-request.md", "doc/project/rule/classification.md", "doc/project/rule/release.md"]


def git(*args, cwd=None):
    return subprocess.run(["git", *args], capture_output=True, text=True, cwd=cwd)


def commit_of(ref):
    out = git("rev-parse", "--verify", "--quiet", f"{ref}^{{commit}}")
    if out.returncode != 0:
        raise ValueError(f"'{ref}' is not a commit")
    return out.stdout.strip()


def gates(commit_range, description):
    """(gate, rules, arguments) for each gate, in the order of the review guide."""
    found = [
        ("check-commits.sh", "PR-SPLIT-*, PR-SERIES-*, PR-MSG-*", [commit_range]),
        ("check-classification.sh", "CR-*", [commit_range]),
        ("check-includes.sh", "PR-SERIES-BUILDS: every #include resolves", [commit_range]),
        ("check-ned-params.sh", "PR-SERIES-BUILDS: every par() is declared", [commit_range]),
        ("check-whatsnew.sh", "RR-WHATSNEW-FILE, RR-MIGRATE-FILE", [commit_range]),
    ]
    if description:
        found.append(("check-pr-description.sh", "PR-REQ-STORY", [*description, "--range", commit_range]))
    return found


def run(gate, arguments):
    """(exit status, output) of one gate, with its error output in place."""
    out = subprocess.run(["bash", str(HERE / gate), *arguments], stdout=subprocess.PIPE,
                         stderr=subprocess.STDOUT, text=True)
    return out.returncode, out.stdout


def severity(status):
    """0 = clean, 1 = a violation, 2 = the gate could not run: any other status, a signal included."""
    return status if status in (0, 1) else 2


def verdict(status):
    return ("pass", "FAIL", "ERROR")[severity(status)]


def counts(output):
    lines = output.splitlines()
    return (sum("VIOLATION:" in line for line in lines),
            sum(line.lstrip().startswith("note:") for line in lines))


def listing(items):
    """'a, b and c'."""
    return ", ".join(items[:-1]) + " and " + items[-1] if len(items) > 1 else "".join(items)


def link(path, gate_commit):
    """A link to a rule document in a CI job summary, or the plain path elsewhere."""
    server, repository = os.environ.get("GITHUB_SERVER_URL"), os.environ.get("GITHUB_REPOSITORY")
    if server and repository:
        return f"[`{path}`]({server}/{repository}/blob/{gate_commit}/{path})"
    return f"`{path}`"


def markdown(series, results, gate_commit, description_ran):
    violations = sum(counts(output)[0] for _, _, _, output in results)
    worst = max(severity(status) for _, _, status, _ in results)
    lines = ["## Pull-request audit: the mechanical gates", "",
             f"The series is `{series['range']}`, {series['commits']} commit(s). "
             f"The gates are from `{gate_commit[:10]}`.", "",
             "| Gate | Rules | Result |", "| --- | --- | --- |"]
    for gate, rules, status, output in results:
        v, n = counts(output)
        lines.append(f"| `{gate}` | {rules} | {verdict(status)}: {v} violation(s), {n} note(s) |")
    if not description_ran:
        lines.append("| `check-pr-description.sh` | PR-REQ-STORY | not run: no description given |")
    lines.append("")
    for gate, _, status, output in results:
        state = " open" if status != 0 else ""
        lines += [f"<details{state}><summary><code>{gate}</code>: {verdict(status)}</summary>", "",
                  f"<pre>{html.escape(output.rstrip())}</pre>", "", "</details>", ""]
    if worst == 0:
        lines.append("No violation. A note asks a question for the reviewer, and it never fails the check.")
    else:
        lines.append(f"{violations} violation(s). A note asks a question for the reviewer, and it never fails the check.")
    lines += ["", f"The rules are {listing([link(rule, gate_commit) for rule in RULES])}. "
                  f"The audit procedure is {link(GUIDE, gate_commit)}.", ""]
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--base", required=True, help="the branch that the pull request targets")
    parser.add_argument("--head", default="HEAD", help="the head commit of the pull request (default HEAD)")
    source = parser.add_mutually_exclusive_group()
    source.add_argument("--description-file", help="a file that holds the description")
    source.add_argument("--pr", type=int, help="a pull request number, to read its description")
    parser.add_argument("--markdown", help="also write the report as Markdown to this file")
    args = parser.parse_args()
    try:
        base, head = commit_of(args.base), commit_of(args.head)
        merge_base = git("merge-base", base, head)
        if merge_base.returncode != 0:
            raise ValueError(f"'{args.base}' and '{args.head}' have no merge base")
        commit_range = f"{merge_base.stdout.strip()}..{head}"
        commits = int(git("rev-list", "--count", commit_range).stdout)
        gate_commit = git("rev-parse", "HEAD", cwd=HERE).stdout.strip() or "unknown"
    except ValueError as error:
        print(f"error: {error}", file=sys.stderr)
        return 2

    description = (["--file", args.description_file] if args.description_file
                   else ["--pr", str(args.pr)] if args.pr else [])
    series = {"range": f"{commit_range.split('..')[0][:10]}..{head[:10]}", "commits": commits}
    print(f"== the pull-request audit: {commit_range} ==")
    print(f"  {commits} commit(s); the gates are from {gate_commit[:10]}")
    results = []
    for gate, rules, arguments in gates(commit_range, description):
        print()
        print(f"######## {gate} ({rules})")
        status, output = run(gate, arguments)
        print(output.rstrip())
        results.append((gate, rules, status, output))

    print()
    print("== the result of each gate ==")
    for gate, _, status, output in results:
        v, n = counts(output)
        print(f"  {gate:26} {verdict(status):5}  {v} violation(s), {n} note(s)")
    if not description:
        print(f"  {'check-pr-description.sh':26} not run: no description given")
    if args.markdown:
        Path(args.markdown).write_text(markdown(series, results, gate_commit, bool(description)), encoding="utf-8")

    worst = max(severity(status) for _, _, status, _ in results)
    print()
    if worst == 0:
        print("PASS: no violation. A note asks a question, and it does not fail the audit.")
    elif worst == 1:
        print(f"FAIL: see {listing(RULES)} for each rule.")
    else:
        print("ERROR: a gate could not run; its output above says why.")
    return worst


if __name__ == "__main__":
    sys.exit(main())
