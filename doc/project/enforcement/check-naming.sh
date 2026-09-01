#!/usr/bin/env bash
#
# The naming gate for INET — a T3 fitness function (see AR-QUAL-ENFORCED).
# It enforces the path/asset half of doc/project/rule/naming.md: the names a script can see.
# The C++ identifiers are check-cpp.sh; declaration-level NED/MSG names are checked by the
# canonical diff-aware check-ned-msg-naming.py gate below.
#
#   NR-PKG    — a directory is lowercase, run-together, with no underscore and no hyphen
#   NR-DIR    — a package directory is singular
#   NR-ASSET  — an icon file is lowercase and run-together
#   NR-CI     — a workflow file is build-*, *-tests, or check-* and lowercase/hyphenated
#   NR-GEN    — a generated *_m.h / *_m.cc sits beside its .msg
#
# Usage (from the INET repository root):
#   doc/project/enforcement/check-naming.sh                         # whole-tree paths + working-tree declarations
#   doc/project/enforcement/check-naming.sh --base origin/master    # whole-tree paths + committed declarations
#   doc/project/enforcement/check-naming.sh src/inet/linklayer      # complete audit of one subtree
#   doc/project/enforcement/check-naming.sh --base origin/master src/inet/linklayer
#
# Exit status 0 = clean, 1 = candidates, 2 = invalid invocation or missing canonical gate. A hit that is already a row in
# doc/project/audit/naming-exceptions.md is known — record a genuinely new one as NV-*.

set -uo pipefail

usage() {
  echo "usage: $0 [--base REF] [src/inet/<subtree>]" >&2
}

BASE=""
SCOPE="src/inet"
SCOPE_EXPLICIT=0
while [ $# -gt 0 ]; do
  case "$1" in
    --base)
      if [ $# -lt 2 ] || [[ "$2" == --* ]]; then
        echo "error: --base requires a Git reference" >&2
        usage
        exit 2
      fi
      BASE="$2"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    --*)
      echo "error: unknown option: '$1'" >&2
      usage
      exit 2
      ;;
    *)
      if [ "$SCOPE_EXPLICIT" -eq 1 ]; then
        echo "error: multiple scopes provided: '$SCOPE' and '$1'" >&2
        usage
        exit 2
      fi
      SCOPE="$1"
      SCOPE_EXPLICIT=1
      shift
      ;;
  esac
done

if [[ "$SCOPE" != "src/inet" && "$SCOPE" != src/inet/* ]]; then
  echo "error: scope must be src/inet or a subtree: '$SCOPE'" >&2
  exit 2
fi
if [ ! -d "$SCOPE" ]; then
  echo "error: '$SCOPE' not found (run from the INET repository root)" >&2
  exit 2
fi
REPOSITORY_ROOT="$(git rev-parse --show-toplevel 2>/dev/null || true)"
RESOLVED_SCOPE="$(cd "$SCOPE" && pwd -P)"
if [ -z "$REPOSITORY_ROOT" ] || { [ "$RESOLVED_SCOPE" != "$REPOSITORY_ROOT/src/inet" ] && \
                                  [[ "$RESOLVED_SCOPE" != "$REPOSITORY_ROOT"/src/inet/* ]]; }; then
  echo "error: scope resolves outside src/inet: '$SCOPE'" >&2
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
  echo "== NR-CI: a workflow file is build-*, *-tests, or check-* and lowercase/hyphenated =="
  workflow_violations=""
  while IFS= read -r workflow; do
    workflow_name="${workflow##*/}"
    if [[ ! "$workflow_name" =~ ^[a-z0-9]+(-[a-z0-9]+)*-tests\.yml$ && \
          ! "$workflow_name" =~ ^build-[a-z0-9]+(-[a-z0-9]+)*\.yml$ && \
          ! "$workflow_name" =~ ^check-[a-z0-9]+(-[a-z0-9]+)*\.yml$ ]]; then
      workflow_violations+="$workflow"$'\n'
    fi
  done < <(find .github/workflows -type f -name '*.yml' 2>/dev/null | sort)
  report "${workflow_violations%$'\n'}"
fi

echo
NED_MSG_GATE="doc/project/enforcement/check-ned-msg-naming.py"
if [ -n "$BASE" ]; then
  echo "== NR-NED/MSG: declarations changed since merge-base($BASE, HEAD) within $SCOPE =="
  NED_MSG_ARGS=(--base "$BASE" --scope "$SCOPE")
elif [ "$SCOPE_EXPLICIT" -eq 1 ]; then
  echo "== NR-NED/MSG: declaration names in $SCOPE =="
  NED_MSG_ARGS=("$SCOPE")
else
  echo "== NR-NED/MSG: changed declaration names in the working tree =="
  NED_MSG_ARGS=()
fi
if [ ! -f "$NED_MSG_GATE" ]; then
  echo "error: canonical gate missing: $NED_MSG_GATE" >&2
  status=2
elif ! command -v python3 >/dev/null 2>&1; then
  echo "error: python3 is required for canonical gate: $NED_MSG_GATE" >&2
  status=2
else
  python3 "$NED_MSG_GATE" "${NED_MSG_ARGS[@]}"
  gate_status=$?
  if [ "$gate_status" -eq 2 ]; then
    status=2
  elif [ "$gate_status" -ne 0 ] && [ "$status" -eq 0 ]; then
    status=1
  fi
fi

echo
if [ "$status" -eq 0 ]; then
  echo "PASS: naming checks clean."
elif [ "$status" -eq 2 ]; then
  echo "ERROR: naming gate could not run." >&2
else
  echo "FAIL: naming violations found. A hit already listed in"
  echo "      doc/project/audit/naming-exceptions.md is known; record a new one as an NV-* row."
fi
exit "$status"
