#!/usr/bin/env bash
#
# Build every commit of a range — the T2 half of PR-SERIES-BUILDS and TR-CI-EVERY-COMMIT.
#
# The static gates beside this one (check-includes.sh, check-ned-params.sh) find the cheap
# shapes of "used before it exists": a header, a NED parameter, a state field. They cannot find
# a method or a signal that arrives later, because that needs a compiler. This runs one.
#
# It is slow and it rewrites the working tree, so it refuses on a dirty tree and puts the
# original ref back when it finishes or is interrupted.
#
# Usage (from the INET repository root, with the OMNeT++ environment set):
#   doc/project/enforcement/check-series-builds.sh origin/master..HEAD [MODE]
#
# Build notes that cost a session to learn, and that this script therefore encodes:
#   - build from the repository ROOT: the root makefile generates src/inet/features.h
#   - objects land in src/out, not out
#   - a libINET.so left in src/ makes the whole tree look current, so make does nothing
#   - git checkout keeps an unchanged file's mtime, so the differing files must be touched
#   - a message dependency file (src/out/**/*_m.h.d) can name a .msg file that a later commit
#     moved, and a commit whose message compilation fails never rewrites it; the next commit
#     then runs opp_msgtool on a file that no longer exists, and one failure shows as two
#
# Exit status 0 = every commit builds, 1 = at least one does not.

set -uo pipefail
RANGE="${1:-origin/master..HEAD}"
MODE="${2:-release}"
git rev-parse "$RANGE" >/dev/null 2>&1 || { echo "error: '$RANGE' is not a valid commit range" >&2; exit 2; }
[ -z "$(git status --porcelain)" ] || { echo "error: the working tree is dirty; this script rewrites it" >&2; exit 2; }
command -v opp_makemake >/dev/null || { echo "error: opp_makemake is not on the PATH" >&2; exit 2; }

START=$(git symbolic-ref -q --short HEAD || git rev-parse HEAD)
restore() { git checkout -q "$START" 2>/dev/null; }
trap restore EXIT INT TERM

mapfile -t SHAS < <(git log --reverse --format=%H "$RANGE")
echo "== every commit builds: $RANGE (MODE=$MODE) =="
echo "  ${#SHAS[@]} commit(s); this takes minutes per commit"
log=$(mktemp); status=0; prev="$START"; n=0
for sha in "${SHAS[@]}"; do
  n=$((n+1))
  # a build leaves generated _m files behind, and they block the next checkout
  git clean -xdfq -e src/out
  if ! git checkout -q --detach "$sha"; then
    # a skipped commit is not a passed commit
    printf "  %3d/%d  SKIP  %s  checkout failed\n" "$n" "${#SHAS[@]}" "${sha:0:9}"
    status=1; continue
  fi
  # make cannot see a file whose mtime git left alone
  git diff --name-only "$sha" "$prev" -- 'src/*' | xargs -r touch 2>/dev/null
  rm -f src/libINET.so src/libINET_dbg.so
  # every _m file is generated again after the clean above, so its dependency file costs nothing
  find src/out -name '*_m.h.d' -delete 2>/dev/null
  if make MODE="$MODE" -j"$(nproc)" >"$log" 2>&1; then
    printf "  %3d/%d  ok    %s  %s\n" "$n" "${#SHAS[@]}" "${sha:0:9}" "$(git log -1 --format=%s | cut -c1-46)"
  else
    status=1
    printf "  %3d/%d  FAIL  %s  %s\n" "$n" "${#SHAS[@]}" "${sha:0:9}" "$(git log -1 --format=%s | cut -c1-46)"
    grep -E 'error:' "$log" | head -3 | sed 's/^/          /'
  fi
  prev="$sha"
done
rm -f "$log"
echo
if [ "$status" -eq 0 ]; then
  echo "PASS: all ${#SHAS[@]} commit(s) in the range build."
else
  echo "FAIL: a commit did not build, or could not be checked; see PR-SERIES-BUILDS."
  echo "      A skipped commit counts as a failure: a gate that checks nothing must not say PASS."
fi
exit "$status"
