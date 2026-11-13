#!/usr/bin/env bash
#
# TR-BASELINE-PIN, TR-BASELINE-PROVENANCE, TR-BASELINE-ATTRIBUTION and TR-BASELINE-PARENT over a range
# of commits: each commit pins the commit of each baseline repository in inet.opp, a pin moves only
# to a direct child that is in the baseline repository, and each baseline commit that a pin moves
# to has its record. It reads only git objects, of this repository and of the baseline
# repositories, so the pull-request workflow can run it on untrusted content. The work is in
# check_baselines.py.
#
# Usage (from the INET repository root):
#   doc/project/enforcement/check-baselines.sh origin/master..HEAD
#   doc/project/enforcement/check-baselines.sh origin/master..HEAD --baseline-repository statistical=statistics
#
# Exit status 0 = clean, 1 = violations, 2 = invalid usage or the gate cannot run.

set -uo pipefail
if [ $# -lt 1 ]; then
  echo "usage: $0 BASE..HEAD [--baseline-repository KIND=PATH]..." >&2
  exit 2
fi
exec python3 "$(dirname "$0")/check_baselines.py" "$@"
