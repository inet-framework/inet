#!/usr/bin/env bash
#
# The push check of PR-LAND-TRIVIAL in doc/project/rule/pull-request.md: is the change of a branch trivial, so
# that it can go to master with no pull request? The work is in trivial_change.py.
#
# Usage (from the INET repository root):
#   doc/project/enforcement/check-trivial-change.sh --base origin/master --head HEAD
#   doc/project/enforcement/check-trivial-change.sh --base <sha> --head <sha> --required-contexts-file contexts.txt
#
# Exit status 0 = trivial, 1 = not trivial (the change needs a pull request), 2 = the gate cannot run.

set -uo pipefail
exec python3 "$(dirname "$0")/trivial_change.py" "$@"
