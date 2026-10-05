#!/usr/bin/env bash
#
# The pull-request description gate for INET — a T3 fitness function (see AR-QUAL-ENFORCED).
# It reads one description and gives notes for PR-REQ-STORY: no opening, evidence first, an opening
# that starts with actions or is a list of changes and names no problem, and a large series or a long
# description whose first 150 words name no problem. A note never fails the gate.
# The checks live in check_pr_description.py, and the word lists in message_opening.py.
#
# Usage (from the INET repository root):
#   doc/project/enforcement/check-pr-description.sh --pr <n> --range "$MB..refs/pr/<n>"
#   doc/project/enforcement/check-pr-description.sh --file description.md
#   gh pr view <n> --json body -q .body | doc/project/enforcement/check-pr-description.sh
#
# Exit status 0 = done, 2 = invalid usage or no description.

set -uo pipefail
exec python3 "$(dirname "$0")/check_pr_description.py" "$@"
