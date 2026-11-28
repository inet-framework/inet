#!/usr/bin/env bash
#
# TR-REQ-REPORTS and TR-REQ-GREEN of doc/project/rule/testing.md over a range of commits: each required context
# of the ruleset of master still reports on a pull request after the change. A range that renames or
# removes a required job, or changes its matrix, needs a change of the ruleset by an admin at the same
# landing; otherwise each pull request waits for ever. The work is in required_contexts.py.
#
# Usage (from the INET repository root):
#   doc/project/enforcement/check-required-contexts.sh origin/master..HEAD
#   doc/project/enforcement/check-required-contexts.sh BASE..HEAD --required-file contexts.txt
#
# Exit status 0 = clean, 1 = violations, 2 = invalid usage or the gate cannot run.

set -uo pipefail
exec python3 "$(dirname "$0")/required_contexts.py" "$@"
