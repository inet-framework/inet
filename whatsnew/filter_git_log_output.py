#!/usr/bin/env python3
from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass
from typing import List, Optional


@dataclass
class CommitBlock:
    lines: List[str]


def compile_pattern(expr: str, ignore_case: bool) -> re.Pattern:
    flags = re.MULTILINE
    if ignore_case:
        flags |= re.IGNORECASE
    return re.compile(expr, flags)


def parse_commits(lines: List[str]) -> List[CommitBlock]:
    """
    Splits a git log output into commit blocks.
    Assumes commits start with a line beginning with 'commit ' (default git log format).
    Everything before the first 'commit ' is treated as a prologue block.
    """
    blocks: List[CommitBlock] = []
    cur: List[str] = []

    def flush():
        nonlocal cur
        if cur:
            blocks.append(CommitBlock(lines=cur))
            cur = []

    for line in lines:
        if line.startswith("commit "):
            flush()
            cur = [line]
        else:
            cur.append(line)

    flush()
    return blocks


def commit_message_text(block: CommitBlock) -> str:
    """
    Extracts the commit message (subject+body) from a default git log block.

    Default git log looks like:
      commit <sha>
      Author: ...
      Date: ...
      
          subject line
          body...
    We capture all indented-by-4-spaces lines after the first blank line
    following the headers, until patch metadata begins (diff --git)
    or next commit (not in this block).
    """
    msg_lines: List[str] = []
    in_msg = False

    for line in block.lines:
        if line.startswith("diff --git "):
            break

        # Detect start of message: first line that begins with 4 spaces
        # after the header area (we'll just use the indentation rule).
        if line.startswith("    "):
            in_msg = True
            msg_lines.append(line[4:].rstrip("\n"))
            continue

        # Once we started message, stop when indentation ends (unless blank)
        if in_msg:
            if line.strip() == "":
                msg_lines.append("")  # keep blank lines in body
                continue
            else:
                # non-indented non-blank line => probably stats or other output
                # but stop message extraction safely
                break

    return "\n".join(msg_lines)


def filter_blocks(blocks: List[CommitBlock], drop_re: re.Pattern) -> List[CommitBlock]:
    """
    Drops commit blocks whose commit message matches drop_re.
    Keeps the initial prologue block (if any) even if it doesn't start with 'commit '.
    """
    out: List[CommitBlock] = []
    for i, b in enumerate(blocks):
        # Prologue (doesn't start with commit) => keep
        if not b.lines or not b.lines[0].startswith("commit "):
            out.append(b)
            continue

        msg = commit_message_text(b)
        if drop_re.search(msg):
            continue
        out.append(b)
    return out


def main() -> int:
    ap = argparse.ArgumentParser(
        description="Filter out commits (including patches) from `git log -p` output based on commit message regex."
    )
    ap.add_argument(
        "input",
        nargs="?",
        default="-",
        help="Input file containing `git log -p` output (default: stdin).",
    )
    ap.add_argument(
        "-o",
        "--output",
        default="-",
        help="Output file (default: stdout).",
    )
    ap.add_argument(
        "-r",
        "--regex",
        required=True,
        help="Regex to match commit message text; matching commits are DROPPED. (Python regex)",
    )
    ap.add_argument(
        "--ignore-case",
        action="store_true",
        help="Case-insensitive regex matching.",
    )
    args = ap.parse_args()

    drop_re = compile_pattern(args.regex, args.ignore_case)

    if args.input == "-":
        text_lines = sys.stdin.read().splitlines(True)
    else:
        with open(args.input, "r", encoding="utf-8", errors="replace") as f:
            text_lines = f.read().splitlines(True)

    blocks = parse_commits(text_lines)
    kept = filter_blocks(blocks, drop_re)

    if args.output == "-":
        for b in kept:
            sys.stdout.writelines(b.lines)
    else:
        with open(args.output, "w", encoding="utf-8", newline="") as f:
            for b in kept:
                f.writelines(b.lines)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

