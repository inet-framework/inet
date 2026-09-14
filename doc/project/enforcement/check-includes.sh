#!/usr/bin/env bash
#
# Every #include a commit touches must resolve at that commit — a T3 fitness function.
#
# A header that arrives four commits later makes every commit before it fail to compile, and
# the failure is invisible to a reviewer reading the diff: the include line looks ordinary.
# PR-SERIES-BUILDS is what this protects.
#
# Usage (from the INET repository root):
#   doc/project/enforcement/check-includes.sh origin/master..HEAD
#
# Exit status 0 = clean, 1 = violations.

set -uo pipefail
RANGE="${1:-origin/master..HEAD}"
git rev-parse "$RANGE" >/dev/null 2>&1 || { echo "error: '$RANGE' is not a valid commit range" >&2; exit 2; }

python3 - "$RANGE" <<'ENDOFPY'
import re, subprocess, sys
rng = sys.argv[1]
def git(*a): return subprocess.run(["git", *a], capture_output=True, text=True).stdout

shas = git("log", "--reverse", "--format=%H", rng).split()
print(f"== every #include resolves at the commit that writes it: {rng} ==")
print(f"  {len(shas)} commit(s)")
INC = re.compile(r'^\s*#\s*include\s+"(inet/[^"]+)"')
bad = 0
for sha in shas:
    touched = [f for f in git("show", "--name-only", "--format=", sha).split()
               if f.endswith((".cc", ".h")) and f.startswith("src/")]
    if not touched:
        continue
    tree = set(git("ls-tree", "-r", "--name-only", sha).split())
    missing = {}
    for f in touched:
        if f not in tree:
            continue                      # deleted by this commit
        for line in git("show", f"{sha}:{f}").splitlines():
            m = INC.match(line)
            if not m:
                continue
            inc = m.group(1)
            # a generated header comes from the .msg beside it
            src = "src/" + inc
            if src in tree or src.replace("_m.h", ".msg") in tree:
                continue
            missing.setdefault(f, []).append(inc)
    if missing:
        bad += 1
        subj = git("log", "-1", "--format=%s", sha).strip()
        n = sum(len(v) for v in missing.values())
        print(f"  VIOLATION: {sha[:9]} {n} unresolvable include(s) in {len(missing)} file(s) — {subj[:48]}")
        for f, incs in sorted(missing.items())[:4]:
            print(f"             {f.split('/')[-1]}: {', '.join(sorted(set(incs))[:2])}")
print()
if bad:
    print(f"FAIL: {bad} commit(s) write an #include that does not resolve; each one fails to compile.")
    sys.exit(1)
print("PASS: every #include resolves at the commit that writes it.")
ENDOFPY
