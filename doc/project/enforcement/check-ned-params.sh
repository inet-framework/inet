#!/usr/bin/env bash
#
# Every NED parameter a commit reads must be declared at that commit — a T3 fitness function.
#
# A C++ `par("x")` on a parameter the NED does not declare throws at run time, so a commit that
# reads ahead of its declaration compiles and cannot run. `git bisect` over any test is then
# useless across the run of such commits, which is what PR-SERIES-BUILDS exists to prevent.
#
# Usage (from the INET repository root):
#   doc/project/enforcement/check-ned-params.sh origin/master..HEAD
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
print(f"== every par() read is declared in the NED beside it: {rng} ==")
print(f"  {len(shas)} commit(s)")

DECL = re.compile(r'^\s*(?:volatile\s+)?(?:bool|int|double|string|xml|object)\s+([A-Za-z_]\w*)\s*[@=;]')
bad = 0
for sha in shas:
    files = git("show", "--name-only", "--format=", sha).split()
    cc = [f for f in files if f.endswith((".cc", ".h"))]
    if not cc:
        continue
    # what this commit newly reads
    plus = "\n".join(l for l in git("show", sha).splitlines() if l.startswith("+"))
    reads = set(re.findall(r'par\("([A-Za-z_]\w*)"\)', plus))
    if not reads:
        continue
    # what any NED in the tree declares at this commit
    neds = git("ls-tree", "-r", "--name-only", sha).splitlines()
    declared = set()
    for n in (x for x in neds if x.endswith(".ned")):
        for line in git("show", f"{sha}:{n}").splitlines():
            m = DECL.match(line)
            if m:
                declared.add(m.group(1))
    missing = sorted(reads - declared)
    if missing:
        bad += 1
        subj = git("log", "-1", "--format=%s", sha).strip()
        print(f"  VIOLATION: {sha[:9]} reads {len(missing)} undeclared parameter(s) — {subj[:56]}")
        print(f"             {' '.join(missing)}")
print()
if bad:
    print(f"FAIL: {bad} commit(s) read a parameter no NED declares yet; each one compiles and cannot run.")
    sys.exit(1)
print("PASS: every parameter read is declared at the commit that reads it.")
ENDOFPY
