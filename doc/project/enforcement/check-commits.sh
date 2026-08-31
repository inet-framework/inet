#!/usr/bin/env bash
#
# The commit gate for INET — a T3 fitness function (see AR-QUAL-ENFORCED).
# It enforces the mechanical half of doc/project/rule/pull-request.md over a commit range.
# The judgment rules — PR-SPLIT-ONE-CHANGE, PR-SPLIT-UPSTREAM, PR-SPLIT-PREPARE,
# PR-SPLIT-DRIVEBY, PR-MSG-WHY — are T4 agent review.
#
#   PR-SPLIT-WHITESPACE  — a file whose diff is empty ignoring whitespace, in a commit with real changes
#   PR-SPLIT-MOVE        — a rename together with a content change
#   PR-SPLIT-BASELINE    — a baseline and a source file in one commit
#   PR-SERIES-ORDER      — a fixup, squash or "address review" subject
#   PR-SERIES-LINEAR     — a merge commit inside the series
#   PR-MSG-SUBJECT       — "area: what it does", under 72 characters, no file path, no link
#   PR-MSG-FACTS         — no attribution trailer
#
# Usage (from the INET repository root):
#   doc/project/enforcement/check-commits.sh origin/master..HEAD
#
# Exit status 0 = clean, 1 = violations.

set -uo pipefail
RANGE="${1:-origin/master..HEAD}"
if ! git rev-parse "$RANGE" >/dev/null 2>&1; then
  echo "error: '$RANGE' is not a valid commit range" >&2
  exit 2
fi
status=0
flag() { echo "  VIOLATION: $1"; status=1; }

echo "== the series: $RANGE =="
COMMITS=$(git log --format=%H --reverse "$RANGE")
echo "  $(echo "$COMMITS" | grep -c .) commit(s)"

echo
echo "== PR-SERIES-LINEAR: no merge commit inside the series =="
merges=$(git log --merges --format="%h %s" "$RANGE")
[ -n "$merges" ] && flag "merge commit(s): $merges" || echo "  ok"

echo
echo "== PR-SERIES-ORDER: no fixup or review-answering commit =="
fixups=$(git log --format="%h %s" "$RANGE" | grep -Ei "fixup!|squash!|^[0-9a-f]+ (fix )?typo|address(ing)? (review|feedback|comments)|review comments")
[ -n "$fixups" ] && flag "rebase these into the commit they repair: $fixups" || echo "  ok"

echo
echo "== PR-MSG-SUBJECT and PR-MSG-FACTS =="
ok=1
while read -r sha; do
  subj=$(git log -1 --format=%s "$sha")
  short=${sha:0:9}
  [ ${#subj} -gt 72 ] && { flag "$short subject is ${#subj} characters: $subj"; ok=0; }
  echo "$subj" | grep -qE "^[a-z0-9_./-]+(\([a-z0-9_/-]+\))?: " || { flag "$short subject is not 'area: what it does': $subj"; ok=0; }
  echo "$subj" | grep -qE "\.(cc|h|ned|msg|ini|py|sh|md)\b|https?://" && { flag "$short subject names a file or a link: $subj"; ok=0; }
  git log -1 --format=%B "$sha" | grep -qiE "^(Co-Authored-By|Generated-By|Signed-off-by: .*\[bot\]):" && { flag "$short carries an attribution trailer"; ok=0; }
done <<< "$COMMITS"
[ "$ok" -eq 1 ] && echo "  ok"

echo
echo "== PR-SPLIT-BASELINE: a baseline and a source file in one commit =="
ok=1
while read -r sha; do
  files=$(git show --name-only --pretty="" "$sha")
  if echo "$files" | grep -qE "^tests/.*\.(csv|json)$" && echo "$files" | grep -qE "^src/"; then
    flag "${sha:0:9} changes a baseline and a source file together"; ok=0
  fi
done <<< "$COMMITS"
[ "$ok" -eq 1 ] && echo "  ok"

echo
echo "== PR-SPLIT-WHITESPACE: a whitespace-only file inside a functional commit =="
ok=1
while read -r sha; do
  files=$(git show --name-only --pretty="" "$sha" | grep -E "\.(cc|h|ned|msg|py)$")
  [ -z "$files" ] && continue
  real=0; ws=""
  while read -r f; do
    [ -z "$f" ] && continue
    # -w alone is not enough: a branch that removes blank lines reports nothing without
    # --ignore-blank-lines. That is a real finding from audit/report/pull-request/pr-1144.md.
    if [ -z "$(git show -w --ignore-blank-lines --numstat --pretty="" "$sha" -- "$f")" ]; then
      ws+="$f "
    else
      real=1
    fi
  done <<< "$files"
  [ "$real" -eq 1 ] && [ -n "$ws" ] && { flag "${sha:0:9} mixes whitespace-only files with real changes: $ws"; ok=0; }
done <<< "$COMMITS"
[ "$ok" -eq 1 ] && echo "  ok"

echo
echo "== PR-SPLIT-MOVE: a rename together with a content change =="
ok=1
while read -r sha; do
  renames=$(git show --name-status -M -C --pretty="" "$sha" | grep -cE "^R[0-9]" || true)
  edits=$(git show --name-status -M -C --pretty="" "$sha" | grep -cE "^M" || true)
  [ "$renames" -gt 0 ] && [ "$edits" -gt 0 ] && { flag "${sha:0:9} has $renames rename(s) and $edits edit(s)"; ok=0; }
done <<< "$COMMITS"
[ "$ok" -eq 1 ] && echo "  ok"

echo
if [ "$status" -eq 0 ]; then
  echo "PASS: commit checks clean. The judgment rules still need T4 review —"
  echo "      see doc/project/enforcement/checklist/general.md."
else
  echo "FAIL: see doc/project/rule/pull-request.md for each rule."
fi
exit "$status"
