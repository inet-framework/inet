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
#   doc/project/enforcement/check-source-seals.sh --base <ref> [--head <ref>] [--ci-approved]
#
# Exit status:
#   0 = All target files are unsealed, or sealed files carry trusted CI approval
#   1 = One or more target files are SEALED (explicit approval required)
#   2 = Error (e.g. the canonical registry is unavailable)

set -euo pipefail

REPOSITORY_ROOT="$(git rev-parse --show-toplevel 2>/dev/null || true)"
STATUS_FILE="${REPOSITORY_ROOT}/doc/project/audit/seal-list.md"

if [ -z "$REPOSITORY_ROOT" ] || [ ! -f "$STATUS_FILE" ]; then
  echo "error: run from an INET checkout containing doc/project/audit/seal-list.md" >&2
  exit 2
fi

# Select input mode and reject option typos before collecting target files.
mode="paths"
base_ref=""
head_ref=""
ci_approved=0
if [ $# -eq 0 ]; then
  mode="diff"
elif [ "$1" = "--base" ]; then
  if [ $# -lt 2 ] || [[ "${2:-}" == --* ]]; then
    echo "error: --base requires a Git reference" >&2
    exit 2
  fi
  mode="base"
  base_ref="$2"
  shift 2
  while [ $# -gt 0 ]; do
    case "$1" in
      --head)
        if [ -n "$head_ref" ] || [ $# -lt 2 ] || [[ "${2:-}" == --* ]]; then
          echo "error: --head requires exactly one Git reference" >&2
          exit 2
        fi
        head_ref="$2"
        shift 2
        ;;
      --ci-approved)
        if [ "$ci_approved" -eq 1 ]; then
          echo "error: --ci-approved may be specified only once" >&2
          exit 2
        fi
        ci_approved=1
        shift
        ;;
      *)
        echo "error: unknown --base argument: '$1'" >&2
        exit 2
        ;;
    esac
  done
  if [ "$ci_approved" -eq 1 ] && [ -z "$head_ref" ]; then
    echo "error: --ci-approved requires an explicit --head reference" >&2
    exit 2
  fi
elif [[ "${1:-}" == --* ]]; then
  case "$1" in
    --diff)
      mode="diff"
      if [ $# -ne 1 ]; then
        echo "error: $1 does not accept file arguments" >&2
        exit 2
      fi
      ;;
    --staged)
      mode="staged"
      if [ $# -ne 1 ]; then
        echo "error: $1 does not accept file arguments" >&2
        exit 2
      fi
      ;;
    --head|--ci-approved)
      echo "error: $1 is valid only after --base" >&2
      exit 2
      ;;
    *)
      echo "error: unknown option: '$1'" >&2
      exit 2
      ;;
  esac
else
  for arg in "$@"; do
    if [[ "$arg" == --* ]]; then
      echo "error: unknown option in file list: '$arg'" >&2
      exit 2
    fi
  done
fi

# A committed branch is checked against the registry that governed its merge base. Reading the
# branch's registry would let the same branch remove a seal row and then modify the formerly sealed
# source without detection.
merge_base=""
base_commit=""
head_commit=""
status_label="$STATUS_FILE"
if [ "$mode" = "base" ]; then
  if ! base_commit="$(git rev-parse --verify "${base_ref}^{commit}" 2>/dev/null)" || [ -z "$base_commit" ]; then
    echo "error: invalid base reference: '$base_ref'" >&2
    exit 2
  fi
  effective_head_ref="${head_ref:-HEAD}"
  if ! head_commit="$(git rev-parse --verify "${effective_head_ref}^{commit}" 2>/dev/null)" || [ -z "$head_commit" ]; then
    echo "error: invalid head reference: '$effective_head_ref'" >&2
    exit 2
  fi
  if ! merge_base="$(git merge-base "$base_commit" "$head_commit" 2>/dev/null)" || [ -z "$merge_base" ]; then
    echo "error: '$base_ref' has no merge base with '$effective_head_ref'" >&2
    exit 2
  fi
  if ! status_text="$(git show "${merge_base}:doc/project/audit/seal-list.md" 2>/dev/null)"; then
    echo "error: canonical seal registry unavailable at merge base $merge_base" >&2
    exit 2
  fi
  status_label="doc/project/audit/seal-list.md at merge base $merge_base"
else
  status_text="$(<"$STATUS_FILE")"
fi

# Extract path cells only from the canonical "Sealed paths" section. Document-seal rows and
# generated index entries are deliberately outside this source-path guard. Markdown permits
# optional whitespace around table cells, so parse cells instead of matching one rendering.
trim_cell() {
  trimmed_cell="$1"
  trimmed_cell="${trimmed_cell#"${trimmed_cell%%[![:space:]]*}"}"
  trimmed_cell="${trimmed_cell%"${trimmed_cell##*[![:space:]]}"}"
}

strip_html_comments() {
  remaining_text="$1"
  visible_text=""
  while [ -n "$remaining_text" ]; do
    if [ "$in_comment" -eq 1 ]; then
      if [[ "$remaining_text" == *"-->"* ]]; then
        remaining_text="${remaining_text#*-->}"
        in_comment=0
      else
        remaining_text=""
      fi
    elif [[ "$remaining_text" == *"<!--"* ]]; then
      visible_text+="${remaining_text%%<!--*}"
      remaining_text="${remaining_text#*<!--}"
      in_comment=1
    else
      visible_text+="$remaining_text"
      remaining_text=""
    fi
  done
}

SEALED_PATTERNS=()
in_paths=0
in_comment=0
line_number=0
path_cell_pattern='^`([^`]+)`([[:space:]].*)?$'
while IFS= read -r line || [ -n "$line" ]; do
  line_number=$((line_number + 1))
  strip_html_comments "$line"
  trim_cell "$visible_text"
  trimmed_line="$trimmed_cell"

  if [ "$trimmed_line" = "## Sealed paths" ]; then
    in_paths=1
    continue
  elif [ "$trimmed_line" = "## Sealed documents" ]; then
    in_paths=0
    continue
  fi

  if [ "$in_paths" -ne 1 ] || [[ "$trimmed_line" != *"|"* ]]; then
    continue
  fi

  row="$trimmed_line"
  if [[ "$row" == \|* ]]; then
    row="${row:1}"
  fi
  if [[ "$row" == *\| ]]; then
    row="${row::-1}"
  fi
  IFS='|' read -r -a cells <<< "$row"
  trim_cell "${cells[0]:-}"
  if [ "$trimmed_cell" != "🔒" ]; then
    continue
  fi

  trim_cell "${cells[1]:-}"
  path_cell="$trimmed_cell"
  if [[ ! "$path_cell" =~ $path_cell_pattern ]] || [[ "${BASH_REMATCH[2]:-}" == *'`'* ]]; then
    echo "error: malformed active seal row in $status_label:$line_number: expected exactly one non-empty backtick path in the Path cell" >&2
    exit 2
  fi
  pattern="${BASH_REMATCH[1]}"
  trim_cell "$pattern"
  if [ "$pattern" != "$trimmed_cell" ] || [[ "$pattern" == /* || "$pattern" == "src/inet" || \
       "$pattern" == src/inet/* || \
       "$pattern" == *"//"* || "$pattern" == "." || "$pattern" == ".." || \
       "$pattern" == ./* || "$pattern" == ../* || "$pattern" == *"/./"* || \
       "$pattern" == */. || "$pattern" == *"/../"* || "$pattern" == */.. ]]; then
    echo "error: malformed active seal row in $status_label:$line_number: seal path must be normalized and relative to src/inet" >&2
    exit 2
  fi
  SEALED_PATTERNS+=("$pattern")
done <<< "$status_text"

if [ ${#SEALED_PATTERNS[@]} -eq 0 ]; then
  echo "info: No sealed paths found in $status_label. All files unsealed."
  exit 0
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
elif [ "$mode" = "base" ]; then
  while IFS= read -r -d '' f; do
    TARGET_FILES+=("$f")
  # Disable rename detection so both the old sealed path and new path are checked.
  done < <(git diff --no-ext-diff --no-renames --name-only -z "$merge_base" "$head_commit" -- src/inet 2>/dev/null)
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
  if [ "$ci_approved" -eq 1 ]; then
    echo "GUARD APPROVED: $sealed_hits sealed file(s) are authorized for this CI range."
    echo "MERGE BASE: $merge_base"
    echo "HEAD: $head_commit"
    exit 0
  else
    echo "GUARD VIOLATION: $sealed_hits file(s) are SEALED under src/inet/."
    echo "STOP: You must obtain explicit user permission in this conversation before modifying sealed files."
    exit 1
  fi
else
  echo "GUARD PASSED: All checked files are unsealed. Proceeding is safe."
  exit 0
fi
