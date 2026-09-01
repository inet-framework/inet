#!/usr/bin/env bash
#
# Check whether modified or target source files overlap the canonical INET seal registry.
# This is the source-path companion to check-seals.sh, which checks document seal flags.
# Policy: doc/project/rule/sealing.md
# Registry: doc/project/audit/seal-list.md
#
# Usage (from the INET repository root):
#   doc/project/enforcement/check-source-seals.sh [file...]
#   doc/project/enforcement/check-source-seals.sh --diff
#   doc/project/enforcement/check-source-seals.sh --staged
#
# Exit status:
#   0 = All target files are unsealed (or no files provided)
#   1 = One or more target files are SEALED (explicit approval required)
#   2 = Error (e.g. the canonical registry is unavailable)

set -euo pipefail

REPOSITORY_ROOT="$(git rev-parse --show-toplevel 2>/dev/null || true)"
STATUS_FILE="${REPOSITORY_ROOT}/doc/project/audit/seal-list.md"

if [ -z "$REPOSITORY_ROOT" ] || [ ! -f "$STATUS_FILE" ]; then
  echo "error: run from an INET checkout containing doc/project/audit/seal-list.md" >&2
  exit 2
fi

# Extract path cells only from the canonical "Sealed paths" section. Document-seal rows and
# generated index entries are deliberately outside this source-path guard.
SEALED_PATTERNS=()
while IFS= read -r line; do
  if [[ "$line" =~ \`([^\`]+)\` ]]; then
    pattern="${BASH_REMATCH[1]}"
    SEALED_PATTERNS+=("$pattern")
  fi
done < <(awk '
  /^## Sealed paths$/ { in_paths = 1; next }
  /^## Sealed documents$/ { in_paths = 0 }
  /<!--/ { in_comment = 1 }
  /-->/ { in_comment = 0; next }
  in_paths && !in_comment && /^\| 🔒 \|/ { print }
' "$STATUS_FILE")

if [ ${#SEALED_PATTERNS[@]} -eq 0 ]; then
  echo "info: No sealed paths found in $STATUS_FILE. All files unsealed."
  exit 0
fi

# Select input mode and reject option typos before collecting target files.
mode="paths"
if [ $# -eq 0 ]; then
  mode="diff"
elif [[ "${1:-}" == --* ]]; then
  case "$1" in
    --diff)
      mode="diff"
      ;;
    --staged)
      mode="staged"
      ;;
    *)
      echo "error: unknown option: '$1'" >&2
      exit 2
      ;;
  esac
  if [ $# -ne 1 ]; then
    echo "error: $1 does not accept file arguments" >&2
    exit 2
  fi
else
  for arg in "$@"; do
    if [[ "$arg" == --* ]]; then
      echo "error: unknown option in file list: '$arg'" >&2
      exit 2
    fi
  done
fi

# Collect target files.
TARGET_FILES=()

if [ "$mode" = "diff" ]; then
  if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    while IFS= read -r -d '' f; do
      TARGET_FILES+=("$f")
    # Disable rename detection so both the old sealed path and new path are checked.
    done < <(git diff --no-renames --name-only -z HEAD -- src/inet 2>/dev/null)
    while IFS= read -r -d '' f; do
      TARGET_FILES+=("$f")
    done < <(git ls-files --others --exclude-standard -z -- src/inet 2>/dev/null)
  fi
elif [ "$mode" = "staged" ]; then
  if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    while IFS= read -r -d '' f; do
      TARGET_FILES+=("$f")
    # Disable rename detection so both the old sealed path and new path are checked.
    done < <(git diff --no-renames --name-only -z --cached -- src/inet 2>/dev/null)
  fi
else
  for arg in "$@"; do
    normalized="${arg#./}"
    if [[ "$arg" == /* ]]; then
      if [[ "$arg" == "$REPOSITORY_ROOT"/src/inet/* ]]; then
        normalized="${arg#"$REPOSITORY_ROOT"/}"
      else
        echo "error: source path is outside this INET checkout: '$arg'" >&2
        exit 2
      fi
    fi
    if [[ "$normalized" != src/inet/* || "$normalized" == *"//"* || \
          "$normalized" == *"/./"* || "$normalized" == */. || \
          "$normalized" == *"/../"* || "$normalized" == */.. ]]; then
      echo "error: source path must be under src/inet: '$arg'" >&2
      exit 2
    fi
    TARGET_FILES+=("$normalized")
  done
fi

if [ ${#TARGET_FILES[@]} -eq 0 ]; then
  echo "info: No files to check."
  exit 0
fi

sealed_hits=0

for file in "${TARGET_FILES[@]}"; do
  # Normalize path relative to src/inet/
  norm_file="$file"
  if [[ "$norm_file" == src/inet/* ]]; then
    norm_file="${norm_file#src/inet/}"
  elif [[ "$norm_file" == */src/inet/* ]]; then
    norm_file="${norm_file#*/src/inet/}"
  fi

  # An exact .msg seal also covers its generated C++ siblings even when the generated file is
  # passed directly. Directory seals already cover both source and generated paths.
  source_msg=""
  if [[ "$norm_file" == *_m.h ]]; then
    source_msg="${norm_file%_m.h}.msg"
  elif [[ "$norm_file" == *_m.cc ]]; then
    source_msg="${norm_file%_m.cc}.msg"
  fi

  for pattern in "${SEALED_PATTERNS[@]}"; do
    if [[ "$pattern" == */ ]]; then
      # Directory pattern (recursive)
      dir_prefix="${pattern%/}"
      if [[ "$norm_file" == "$dir_prefix"/* ]] || [[ "$norm_file" == "$dir_prefix" ]]; then
        echo "🔒 SEALED: '$file' matches sealed directory '$pattern'"
        sealed_hits=$((sealed_hits + 1))
        break
      fi
    else
      # Exact file pattern
      if [[ "$norm_file" == "$pattern" || ( -n "$source_msg" && "$source_msg" == "$pattern" ) ]]; then
        if [[ "$norm_file" == "$pattern" ]]; then
          echo "🔒 SEALED: '$file' matches sealed file '$pattern'"
        else
          echo "🔒 SEALED: '$file' is generated from sealed message file '$pattern'"
        fi
        sealed_hits=$((sealed_hits + 1))
        break
      fi
    fi
  done
done

echo
if [ "$sealed_hits" -gt 0 ]; then
  echo "GUARD VIOLATION: $sealed_hits file(s) are SEALED under src/inet/."
  echo "STOP: You must obtain explicit user permission in this conversation before modifying sealed files."
  exit 1
else
  echo "GUARD PASSED: All checked files are unsealed. Proceeding is safe."
  exit 0
fi
