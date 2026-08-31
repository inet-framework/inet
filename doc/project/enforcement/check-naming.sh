#!/usr/bin/env bash
#
# The naming gate for INET — a T3 fitness function (see AR-QUAL-ENFORCED).
# It enforces the mechanical half of doc/project/rule/naming.md: the names a script can see.
# The C++ identifiers are check-cpp.sh; the NED, .msg and semantic names are T4 agent review.
#
#   NR-PKG    — a directory is lowercase, run-together, with no underscore and no hyphen
#   NR-DIR    — a package directory is singular
#   NR-ASSET  — an icon file is lowercase and run-together
#   NR-CI     — a workflow file is lowercase and hyphenated
#   NR-GEN    — a generated *_m.h / *_m.cc sits beside its .msg
#
# Usage (from the INET repository root):
#   doc/project/enforcement/check-naming.sh            # the whole tree
#   doc/project/enforcement/check-naming.sh src/inet/linklayer   # one subtree
#
# Exit status 0 = clean, 1 = violations. A hit that is already a row in
# doc/project/audit/naming-exceptions.md is known — record a genuinely new one as NV-*.

set -uo pipefail
SCOPE="${1:-src/inet}"
if [ ! -d "$SCOPE" ]; then
  echo "error: '$SCOPE' not found (run from the INET repository root)" >&2
  exit 2
fi
status=0
report() { if [ -n "$1" ]; then echo "$1" | sed 's/^/  VIOLATION: /'; status=1; else echo "  ok"; fi }

echo "== NR-PKG: a package directory is lowercase and run-together: $SCOPE =="
report "$(find "$SCOPE" -type d | grep -E '/[^/]*([A-Z]|_|-)[^/]*$' | grep -v '/\.')"

echo
echo "== NR-DIR: a package directory is singular =="
report "$(find "$SCOPE" -type d | grep -E '/(tables|modes|messages|profiles|flavours|headers|packets|nodes)$')"

echo
echo "== NR-GEN: every *_m.h has its .msg beside it =="
missing=""
while IFS= read -r f; do
  [ -f "${f%_m.h}.msg" ] || missing+="$f (no ${f%_m.h}.msg)"$'\n'
done < <(find "$SCOPE" -name '*_m.h')
report "${missing%$'\n'}"

if [ "$SCOPE" = "src/inet" ]; then
  echo
  echo "== NR-ASSET: an icon file is lowercase and run-together =="
  # The trailing _vs / _s / _l / _vl size suffix is the OMNeT++ icon convention, not a violation.
  report "$(find images -type f \( -name '*.png' -o -name '*.svg' \) 2>/dev/null \
            | sed -E 's/_(vs|s|l|vl)\.(png|svg)$/.\2/' \
            | grep -E '/[^/]*([A-Z]|_|-)[^/]*\.(png|svg)$' | sort -u)"

  echo
  echo "== NR-CI: a workflow file is lowercase and hyphenated =="
  report "$(find .github/workflows -type f -name '*.yml' 2>/dev/null | grep -E '/[^/]*([A-Z]|_)[^/]*\.yml$')"
fi

echo
if [ "$status" -eq 0 ]; then
  echo "PASS: naming checks clean."
else
  echo "FAIL: naming violations found. A hit already listed in"
  echo "      doc/project/audit/naming-exceptions.md is known; record a new one as an NV-* row."
fi
exit "$status"
