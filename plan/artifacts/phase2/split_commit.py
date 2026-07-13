#!/usr/bin/env python3
"""Split the commit at HEAD (a rebase `edit` stop) into pieces per a JSON spec.

Usage: split_commit.py <spec.json>   (run inside the worktree, at the edit stop)

Spec format:
{
  "expect_subject": "todo1",              # sanity check against HEAD's subject
  "pieces": [
    {
      "message": "Module: Did X.",
      "files": {
        "src/inet/foo/Bar.cc": "all",     # whole file's diff
        "src/inet/foo/Baz.cc": [1, 3]     # only hunks 1 and 3 (1-based, per file)
      }
    },
    ...
  ]
}

Every file/hunk of HEAD's diff must be claimed exactly once across all pieces
(verified); the resulting tree must equal HEAD's original tree (verified).
"""
import json
import subprocess
import sys


def run(*args, **kw):
    return subprocess.run(args, check=True, capture_output=True, text=True, **kw).stdout


def parse_patch(patch):
    """Return {path: [hunk_text, ...]} plus per-file headers {path: header_text}."""
    files, headers = {}, {}
    cur_path, cur_header, cur_hunks, cur_hunk = None, None, None, None

    def flush_hunk():
        nonlocal cur_hunk
        if cur_hunk is not None:
            cur_hunks.append("".join(cur_hunk))
            cur_hunk = None

    def flush_file():
        nonlocal cur_path, cur_header, cur_hunks
        flush_hunk()
        if cur_path is not None:
            files[cur_path] = cur_hunks
            headers[cur_path] = cur_header
        cur_path, cur_header, cur_hunks = None, None, None

    for line in patch.splitlines(keepends=True):
        if line.startswith("diff --git "):
            flush_file()
            # path from "diff --git a/X b/X" -- use the b/ side
            cur_path = line.split(" b/", 1)[1].rstrip("\n")
            cur_header, cur_hunks = [line], []
            continue
        if cur_path is None:
            continue
        if line.startswith("@@"):
            flush_hunk()
            cur_hunk = [line]
        elif cur_hunk is not None:
            cur_hunk.append(line)
        else:
            cur_header.append(line)
    flush_file()
    return files, {p: "".join(h) for p, h in headers.items()}


def recount_hunks(hunks):
    """Recompute @@ -a,b +c,d @@ offsets for a subset of a file's hunks.

    When earlier hunks of the same file are omitted, later hunks' NEW-side start
    lines must shift by the omitted delta. git apply with --recount fixes counts
    but not start lines; we adjust starts ourselves.
    """
    out, shift = [], 0
    for h in hunks:
        first, rest = h.split("\n", 1)
        import re
        m = re.match(r"@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@(.*)", first)
        old_start, old_count = int(m.group(1)), int(m.group(2) or 1)
        new_count = int(m.group(4) or 1)
        tail = m.group(5)
        new_start = old_start + shift
        if old_count == 0:
            new_start += 1  # pure-insert hunks anchor after the old line
        out.append(f"@@ -{old_start},{old_count} +{new_start},{new_count} @@{tail}\n{rest}")
        shift += new_count - old_count
    return out


def main():
    spec = json.load(open(sys.argv[1]))
    head = run("git", "rev-parse", "HEAD").strip()
    subject = run("git", "log", "-1", "--format=%s", head).strip()
    if spec.get("expect_subject") and subject != spec["expect_subject"]:
        sys.exit(f"HEAD subject {subject!r} != expected {spec['expect_subject']!r}")
    orig_tree = run("git", "rev-parse", "HEAD^{tree}").strip()
    patch = run("git", "diff-tree", "-p", "--binary", "-M", "--no-color", head)
    files, headers = parse_patch(patch)

    # verify full coverage, no overlap
    claimed = {p: set() for p in files}
    for piece in spec["pieces"]:
        for path, sel in piece["files"].items():
            if path not in files:
                sys.exit(f"piece claims unknown path {path}")
            idx = set(range(1, len(files[path]) + 1)) if sel == "all" else set(sel)
            if claimed[path] & idx:
                sys.exit(f"overlapping claim on {path}: {claimed[path] & idx}")
            claimed[path] |= idx
    for path, got in claimed.items():
        want = set(range(1, len(files[path]) + 1))
        if got != want:
            sys.exit(f"uncovered hunks in {path}: {sorted(want - got)} (has {len(files[path])})")

    run("git", "reset", "--hard", "HEAD^")
    for i, piece in enumerate(spec["pieces"], 1):
        parts = []
        for path, sel in piece["files"].items():
            hunks = files[path]
            chosen = hunks if sel == "all" else [hunks[j - 1] for j in sorted(sel)]
            if sel != "all" and len(chosen) != len(hunks):
                chosen = recount_hunks(chosen)
            parts.append(headers[path] + "".join(chosen))
        piece_patch = "".join(parts)
        subprocess.run(["git", "apply", "--index", "--whitespace=nowarn", "-"],
                       input=piece_patch, text=True, check=True)
        run("git", "commit", "-m", piece["message"])
        print(f"piece {i}/{len(spec['pieces'])}: {piece['message']}")

    new_tree = run("git", "rev-parse", "HEAD^{tree}").strip()
    if new_tree != orig_tree:
        sys.exit(f"TREE MISMATCH after split: {new_tree} != {orig_tree} — do NOT continue the rebase")
    print(f"OK: tree identical ({orig_tree[:12]}), {len(spec['pieces'])} pieces")


if __name__ == "__main__":
    main()
