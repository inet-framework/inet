#!/usr/bin/env python3
"""Read a pull request description and give notes where its opening does not answer the reviewer's
questions (PR-REQ-STORY in doc/project/rule/pull-request.md).

A word list cannot judge an answer, so every finding is a note and the exit status stays 0. The
notes point the reviewer at the clear cases: no opening at all, evidence before the summary, an
opening that starts with actions or is a list of changes and names no problem, and a large series or
a long description whose first screen names no problem.

Usage, from the repository root:
  check-pr-description.sh --file description.md [--range BASE..HEAD]
  check-pr-description.sh --pr 1273 [--range BASE..HEAD]
  gh pr view 1273 --json body -q .body | check-pr-description.sh

--range adds the size of the series: the number of commits and the changed source lines.
Exit status 0 = done (notes never fail), 2 = invalid usage or no description.
"""
import argparse
import json
import re
import subprocess
import sys
import urllib.request
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from message_opening import (PREAMBLE_HEADING, REASON, action_first, blocks, first_paragraph,  # noqa: E402
                             is_preamble, opening)

REPOSITORY = "inet-framework/inet"
EVIDENCE_HEADING = re.compile(r"^#+\s*(validation|tests?|testing|evidence|results?|commands|how to (run|test)|build)\b", re.I)
LARGE_COMMITS = 3            # more commits than this is a series that needs its own summary
LARGE_SOURCE_LINES = 400     # the PR-SPLIT-SIZE limit of one commit
LONG_DESCRIPTION_WORDS = 1000  # 1428 is the p90 of 100 recent descriptions; 1000 is a long read
FIRST_SCREEN_WORDS = 150       # what a reviewer reads before deciding what the pull request is


def read_pr(number):
    """The description of a pull request: through gh when it is logged in, else the public API."""
    try:
        out = subprocess.run(["gh", "pr", "view", str(number), "--repo", REPOSITORY, "--json", "body", "-q", ".body"],
                             capture_output=True, text=True, timeout=60)
        if out.returncode == 0:
            return out.stdout
    except (OSError, subprocess.TimeoutExpired):
        pass
    url = f"https://api.github.com/repos/{REPOSITORY}/pulls/{number}"
    with urllib.request.urlopen(url, timeout=60) as response:
        return json.load(response).get("body") or ""


def series_size(commit_range):
    """(commits, changed source lines) of a range, counted as PR-SPLIT-SIZE counts them."""
    commits = subprocess.run(["git", "rev-list", "--count", commit_range], capture_output=True, text=True, check=True)
    numstat = subprocess.run(["git", "diff", "--numstat", commit_range, "--", "src/*.cc", "src/*.h", "src/*.ned", "src/*.msg"],
                             capture_output=True, text=True, check=True)
    lines = 0
    for row in numstat.stdout.splitlines():
        added, deleted, path = row.split("\t", 2)
        if added != "-" and not re.search(r"_m\.(h|cc)$", path):
            lines += int(added) + int(deleted)
    return int(commits.stdout), lines


def first_screen(text, limit=FIRST_SCREEN_WORDS):
    """The first words of prose and lists after the preamble, across headings: what a reviewer reads
    first. A short opening is fine when a motivation section follows it at once."""
    words, preamble = [], False
    for kind, block in blocks(text):
        if kind == "heading":
            preamble = bool(PREAMBLE_HEADING.match(block))
        elif not preamble and kind in ("prose", "list") and not is_preamble(kind, block):
            words.extend(block.split())
            if len(words) >= limit:
                break
    return " ".join(words[:limit])


def evidence_first(text):
    """True when a code block, a table or an evidence heading comes before the opening."""
    preamble = False
    for kind, block in blocks(text):
        if kind == "heading":
            if EVIDENCE_HEADING.match(block):
                return True
            preamble = bool(PREAMBLE_HEADING.match(block))
        elif preamble:
            continue
        elif kind in ("code", "table"):
            return True
        elif not is_preamble(kind, block):
            return False
    return False


def notes(text, size=None):
    """The notes for one description; size is (commits, source lines) or None."""
    found = []
    words = len(re.sub(r"<!--.*?-->", "", text, flags=re.S).split())
    kind, opening_text = opening(text)
    if not kind:
        return ["no opening paragraph: which problem does the pull request solve, and why do we want it?"]
    if evidence_first(text):
        found.append("the description opens with evidence: put the summary that answers the reviewer's questions first")
    names_problem = bool(REASON.search(opening_text))
    if kind == "list" and not names_problem:
        found.append("the opening is a list of changes that names no problem: which problem does the pull request solve?")
    elif kind == "prose" and action_first(opening_text):
        found.append(f'the opening starts with an action and names no problem: "{action_first(opening_text)[:80]}"')
    large = size and (size[0] > LARGE_COMMITS or size[1] > LARGE_SOURCE_LINES)
    if (large or words > LONG_DESCRIPTION_WORDS) and not REASON.search(first_screen(text)) \
            and not any("names no problem" in f for f in found):
        extent = f"{size[0]} commits and {size[1]} source lines" if large else f"{words} words"
        found.append(f"{extent}, and the first {FIRST_SCREEN_WORDS} words name no problem: "
                     "is the problem stated up front?")
    return found


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    source = parser.add_mutually_exclusive_group()
    source.add_argument("--file", help="a file that holds the description")
    source.add_argument("--pr", type=int, help="a pull request number in " + REPOSITORY)
    parser.add_argument("--range", help="BASE..HEAD of the series, for the size note")
    args = parser.parse_args()
    try:
        if args.file:
            text, label = Path(args.file).read_text(encoding="utf-8"), args.file
        elif args.pr:
            text, label = read_pr(args.pr), f"pull request {args.pr}"
        else:
            if sys.stdin.isatty():
                parser.error("give --file, --pr, or a description on standard input")
            text, label = sys.stdin.read(), "standard input"
        size = series_size(args.range) if args.range else None
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 2
    print(f"== PR-REQ-STORY: the description of {label} ==")
    print(f"  {len(re.sub(r'<!--.*?-->', '', text, flags=re.S).split())} words; opening of {len(first_paragraph(text).split())} words"
          + (f"; {size[0]} commits, {size[1]} source lines" if size else ""))
    found = notes(text, size)
    for note in found:
        print(f"  note: {note}")
    print()
    if found:
        print(f"NOTES: {len(found)}. A note asks a question; see PR-REQ-STORY in doc/project/rule/pull-request.md.")
    else:
        print("PASS: no note. Whether the answers are true needs a reviewer.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
