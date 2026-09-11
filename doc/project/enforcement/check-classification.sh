#!/usr/bin/env bash
#
# The classification gate for INET — a T3 fitness function (see AR-QUAL-ENFORCED).
# It enforces the mechanical half of doc/project/rule/classification.md over a commit range.
# The judgment rules — CR-SCOPE-POSITION, CR-SCOPE-ABOUT, CR-DEPTH-FIX, CR-GROUP-* — are
# T4 agent review, and so is whether an inert claim is true.
#
#   CR-TAG-TRAILER   — every commit ends with one Change: line
#   CR-TAG-FORM      — three fields separated by |, an optional fourth, and a known value in each
#   CR-TAG-SUBJECT   — the subject carries the kind, and every prefix agrees with the trailer
#   CR-SCOPE-AREA    — the claimed area matches the paths the commit touches
#   CR-DEPTH-ONE     — one commit reaches one depth level
#   CR-OBL-INERT     — a commit below the behavior level moves no recorded expectation
#
# Usage (from the INET repository root):
#   doc/project/enforcement/check-classification.sh origin/master..HEAD
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
note() { echo "  note: $1"; }

DEPTHS="comment format location name refactor behavior"
DIRS="add change remove"
OBLS="fingerprint statistical expected test whatsnew migration"
AREAS="src tests doc examples build plan"

COMMITS=$(git log --format=%H --reverse "$RANGE")
echo "== the series: $RANGE =="
echo "  $(echo "$COMMITS" | grep -c .) commit(s)"

# The kind a subject must carry, derived from the trailer's depth field:
#   behavior.change.fix -> fix      behavior.add+change -> add+change
#   name+refactor       -> name+refactor        refactor -> refactor
want_kind() { echo "$1" | sed -e 's/behavior\.//g' -e 's/[a-z]*\.fix/fix/g'; }

echo
echo "== CR-TAG-TRAILER and CR-TAG-FORM: one well-formed Change: line =="
ok=1
while read -r sha; do
  [ -z "$sha" ] && continue
  t=$(git log -1 --format='%(trailers:key=Change,valueonly)' "$sha" | tr -d '\n')
  if [ -z "$t" ]; then flag "${sha:0:9} has no Change: line"; ok=0; continue; fi
  nf=$(awk -F'|' '{print NF}' <<< "$t")
  if [ "$nf" -lt 3 ] || [ "$nf" -gt 4 ]; then
    flag "${sha:0:9} Change: has $nf field(s), not 3 or 4: $t"; ok=0; continue
  fi
  kind=$(awk -F'|' '{gsub(/ /,"");print $2}' <<< "$t")
  for part in $(tr '+.' '  ' <<< "$kind"); do
    case " $DEPTHS $DIRS fix " in *" $part "*) ;; *) flag "${sha:0:9} unknown kind word '$part' in '$kind'"; ok=0 ;; esac
  done
  obl=$(awk -F'|' '{print $3}' <<< "$t")
  for part in $obl; do
    case " $OBLS - ? " in *" $part "*) ;; *) flag "${sha:0:9} unknown obligation '$part'"; ok=0 ;; esac
  done
done <<< "$COMMITS"
[ "$ok" -eq 1 ] && echo "  ok"

echo
echo "== CR-DEPTH-ONE: one commit reaches one depth level =="
ok=1
while read -r sha; do
  [ -z "$sha" ] && continue
  kind=$(git log -1 --format='%(trailers:key=Change,valueonly)' "$sha" | awk -F'|' '{gsub(/ /,"");print $2}')
  n=0
  for p in $(tr '+' ' ' <<< "$kind"); do
    head=${p%%.*}
    case " $DEPTHS " in *" $head "*) n=$((n+1)) ;; esac
  done
  [ "$n" -gt 1 ] && { flag "${sha:0:9} claims $n depth levels: $kind"; ok=0; }
done <<< "$COMMITS"
[ "$ok" -eq 1 ] && echo "  ok"

echo
echo "== CR-TAG-SUBJECT: the subject carries the kind and agrees with the trailer =="
ok=1
while read -r sha; do
  [ -z "$sha" ] && continue
  s=$(git log -1 --format=%s "$sha")
  t=$(git log -1 --format='%(trailers:key=Change,valueonly)' "$sha" | tr -d '\n')
  [ -z "$t" ] && continue
  kind=$(awk -F'|' '{gsub(/ /,"");print $2}' <<< "$t")
  want=$(want_kind "$kind")
  # A mixed kind carries a '+', which is a quantifier in an extended regular expression,
  # so 'add+change' would look for 'ad' and one or more 'd'. Escape it: the rule names
  # 'add+change:' and 'name+refactor:' as subject markers, so the gate has to match them.
  want_re=$(sed 's/+/\\+/g' <<< "$want")
  if ! grep -qE "(^|: )${want_re}: " <<< "$s"; then
    flag "${sha:0:9} subject has no '${want}:' — $s"; ok=0
  fi
  # every other prefix must be a run of segments of the scope, or the group
  scope=$(awk -F'|' '{gsub(/ /,"");print $1}' <<< "$t")
  grp=$(awk -F'|' '{gsub(/ /,"");print $4}' <<< "$t")
  head=${s%%: *}
  if [ "$head" != "$s" ] && [ "$head" != "$want" ]; then
    case ".$scope." in *".$head."*) ;; *)
      [ -n "$grp" ] && [ "$head" = "$grp" ] || { flag "${sha:0:9} subject prefix '$head' is in neither the scope '$scope' nor the group"; ok=0; } ;;
    esac
  fi
done <<< "$COMMITS"
[ "$ok" -eq 1 ] && echo "  ok"

echo
echo "== CR-SCOPE-AREA: the claimed area matches the paths touched =="
ok=1
while read -r sha; do
  [ -z "$sha" ] && continue
  claimed=$(git log -1 --format='%(trailers:key=Change,valueonly)' "$sha" | awk -F'|' '{print $1}')
  [ -z "$claimed" ] && continue
  claimed=$(awk '{print $1}' <<< "$claimed" | cut -d. -f1)
  [ -z "$claimed" ] && continue
  case " $AREAS " in *" $claimed "*) ;; *) flag "${sha:0:9} unknown area '$claimed'"; ok=0; continue ;; esac
  files=$(git show --name-only --format='' "$sha")
  seen=""
  grep -q '^src/'                       <<< "$files" && seen="$seen src"
  grep -qE '^tests/'                    <<< "$files" && seen="$seen tests"
  grep -qE '^(doc/|WHATSNEW)'           <<< "$files" && seen="$seen doc"
  grep -qE '^(examples|showcases|tutorials)/' <<< "$files" && seen="$seen examples"
  grep -qE '^(python/|\.github/|Makefile|configure)' <<< "$files" && seen="$seen build"
  grep -q '^plan/'                      <<< "$files" && seen="$seen plan"
  case " $seen " in *" $claimed "*) ;; *) flag "${sha:0:9} claims area '$claimed' and touches:$seen"; ok=0 ;; esac
done <<< "$COMMITS"
[ "$ok" -eq 1 ] && echo "  ok"

echo
echo "== CR-OBL-INERT: a depth below behavior moves no recorded expectation =="
ok=1
while read -r sha; do
  [ -z "$sha" ] && continue
  kind=$(git log -1 --format='%(trailers:key=Change,valueonly)' "$sha" | awk -F'|' '{gsub(/ /,"");print $2}')
  case "$kind" in *behavior*) continue ;; esac
  n=$(git show --name-only --format='' "$sha" | grep -cE 'tests/(fingerprint|statistical)/.*\.(csv|json|sca)$')
  [ "$n" -gt 0 ] && { flag "${sha:0:9} claims '$kind' and moves $n recorded expectation(s)"; ok=0; }
done <<< "$COMMITS"
[ "$ok" -eq 1 ] && echo "  ok"

echo
echo "== the breakdown (CR-* is descriptive here, not a check) =="
git log --reverse --format='%s%x09%(trailers:key=Change,valueonly)' "$RANGE" \
  | python3 "$(dirname "$0")/commit_breakdown.py" | sed 's/^/  /'

echo
if [ "$status" -eq 0 ]; then
  echo "PASS: classification checks clean. Whether an inert claim is TRUE needs a run —"
  echo "      see CR-OBL-BASELINE in doc/project/rule/classification.md."
else
  echo "FAIL: see doc/project/rule/classification.md for each rule."
fi
exit "$status"
