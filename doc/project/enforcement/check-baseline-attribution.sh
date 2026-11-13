#!/usr/bin/env bash
#
# The attribution run: for each commit of a range that moves a baseline pin over a result, it runs
# the moved tests at the parent commit, at the commit, and with the reduction patch, and checks that
# each run gives the result that the Attribution of the baseline commit claims
# (TR-BASELINE-ATTRIBUTION, TR-BASELINE-PARENT). The work is in baseline_attribution.py.
#
# "plan" reads git objects only. "run" checks out, builds and runs content of the range in this
# work tree: use a checkout that is only for this purpose, never the trusted pull-request audit.
#
# Usage (from the INET repository root):
#   doc/project/enforcement/check-baseline-attribution.sh plan origin/master..HEAD
#   doc/project/enforcement/check-baseline-attribution.sh run origin/master..HEAD
#
# Exit status 0 = each run passes, 1 = a run fails, 2 = invalid usage or the tool cannot run.

set -uo pipefail
if [ $# -lt 2 ]; then
  echo "usage: $0 plan|run BASE..HEAD [options]" >&2
  exit 2
fi
exec python3 "$(dirname "$0")/baseline_attribution.py" "$@"
