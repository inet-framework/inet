#!/usr/bin/env bash
#
# The C++ gate for INET — a T3 fitness function (see AR-QUAL-ENFORCED).
# It runs clang-tidy with the repository-root .clang-tidy, which holds two rule families:
#
#   readability-identifier-naming  — the C++ half of doc/project/rule/naming.md
#   bugprone-* / modernize-*       — the C++ half of doc/project/rule/quality.md
#
# Usage (from the INET repository root):
#   doc/project/enforcement/check-cpp.sh                       # all of src/inet
#   doc/project/enforcement/check-cpp.sh src/inet/linklayer    # one subtree
#   doc/project/enforcement/check-cpp.sh --fix src/inet/queueing
#
# --fix rewrites the sources. A gate reports and does not rewrite, so --fix is off by
# default; when you do use it, the result is a mechanical commit of its own
# (PR-SPLIT-MECHANICAL) and nothing else may ride in it.
#
# It needs a compilation database. Build once with a tool that writes compile_commands.json,
# or point -p at an existing one:
#   make MODE=debug -j$(nproc)   # then set DB to the directory that holds compile_commands.json
#
# Exit status 0 = clean, 1 = findings. A finding that is already a row in
# doc/project/audit/naming-exceptions.md is known, not new — do not re-report it.

set -uo pipefail
FIX=""
if [ "${1:-}" = "--fix" ]; then FIX="--fix"; shift; fi
SCOPE="${1:-src/inet}"
DB="${INET_COMPILE_DB:-src}"

if ! command -v clang-tidy >/dev/null 2>&1; then
  echo "error: clang-tidy not found in PATH" >&2
  exit 2
fi
if [ ! -d "$SCOPE" ]; then
  echo "error: '$SCOPE' not found (run from the INET repository root)" >&2
  exit 2
fi
if [ ! -f "$DB/compile_commands.json" ]; then
  echo "error: no compile_commands.json under '$DB'" >&2
  echo "       build first, or set INET_COMPILE_DB to the directory that holds it" >&2
  exit 2
fi

echo "== C++ names and quality: $SCOPE (config: $(pwd)/.clang-tidy) =="
mapfile -t FILES < <(find "$SCOPE" -name '*.cc' -not -name '*_m.cc' | sort)
echo "   ${#FILES[@]} files, generated *_m.cc excluded (NR-GEN)"

status=0
clang-tidy -p "$DB" $FIX "${FILES[@]}" 2>/dev/null | tee /tmp/inet-check-cpp.out
if grep -q "warning:\|error:" /tmp/inet-check-cpp.out; then status=1; fi

echo
if [ "$status" -eq 0 ]; then
  echo "PASS: no findings."
else
  echo "FAIL: findings above. Record a genuinely new one in"
  echo "      doc/project/audit/naming-exceptions.md (NV-*); a row that is already there is known."
fi
exit "$status"
