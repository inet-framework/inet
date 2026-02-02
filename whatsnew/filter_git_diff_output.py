#!/usr/bin/env python3
from __future__ import annotations

import argparse
import re
import sys
from typing import List

from unidiff import PatchSet


def should_keep_empty_file_section(pf) -> bool:
    """
    Return True if this file section should be kept even when it has 0 hunks.
    We keep renames/adds/deletes and mode changes.
    """
    if getattr(pf, "is_rename", False):
        return True
    if getattr(pf, "is_added_file", False):
        return True
    if getattr(pf, "is_removed_file", False):
        return True

    # Mode changes (best-effort; depends on unidiff version/input)
    if getattr(pf, "source_mode", None) or getattr(pf, "target_mode", None):
        return True

    return False


def should_drop_hunk(hunk, pattern: re.Pattern) -> bool:
    """
    Drop hunk iff:
      - it has at least one changed line (+/-), AND
      - EVERY changed line matches pattern.match(), AND
      - NO changed line is blank/whitespace-only.
    """
    saw_changed = False

    for line in hunk:
        if not (line.is_added or line.is_removed):
            continue

        saw_changed = True
        text = line.value.rstrip("\r\n")

        # blank changed line => keep the hunk
        if text.strip() == "":
            return False

        # must match regex (prefix match)
        if pattern.match(text) is None:
            return False

    return saw_changed


def main() -> int:
    ap = argparse.ArgumentParser(
        description=(
            "Drop hunks from a unified git diff where all changed lines match a regex "
            "(prefix match), but blank changed lines keep the hunk. "
            "Removes file sections that end up with no hunks, except for renames/adds/deletes/mode-changes."
        )
    )
    ap.add_argument("input", nargs="?", default="-", help="Input diff (default: stdin).")
    ap.add_argument("-o", "--output", default="-", help="Output diff (default: stdout).")
    ap.add_argument(
        "-r",
        "--regex",
        required=True,
        help="Python regex. A hunk is dropped if ALL changed lines match via re.match(), and none are blank.",
    )
    ap.add_argument("--ignore-case", action="store_true", help="Case-insensitive regex matching.")
    args = ap.parse_args()

    flags = re.IGNORECASE if args.ignore_case else 0
    pattern = re.compile(args.regex, flags)

    if args.input == "-":
        diff_text = sys.stdin.read()
    else:
        with open(args.input, "r", encoding="utf-8", errors="replace") as f:
            diff_text = f.read()

    try:
        patch = PatchSet(diff_text)
    except Exception as e:
        print(f"ERROR: could not parse unified diff with unidiff: {e}", file=sys.stderr)
        return 2

    files_to_remove: List[int] = []

    for i, pf in enumerate(patch):
        kept_hunks = []
        for hunk in pf:
            if not should_drop_hunk(hunk, pattern):
                kept_hunks.append(hunk)

        # replace hunks in-place
        del pf[:]
        for h in kept_hunks:
            pf.append(h)

        # If no hunks remain, drop the file section unless it is rename/add/delete/mode-change
        if len(pf) == 0 and not should_keep_empty_file_section(pf):
            files_to_remove.append(i)

    for i in reversed(files_to_remove):
        del patch[i]

    out_text = str(patch)

    if args.output == "-":
        sys.stdout.write(out_text)
    else:
        with open(args.output, "w", encoding="utf-8", newline="") as f:
            f.write(out_text)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

