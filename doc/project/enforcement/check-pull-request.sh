#!/usr/bin/env bash
#
# The mechanical pull-request audit for INET: the series gates of review-a-pull-request.md, steps 2
# and 3, over one pull request, with one exit status. CI runs it on every pull request to master
# (.github/workflows/check-pull-request.yml), and a reviewer runs it by hand. The work is in
# check_pull_request.py.
#
# Usage (from the INET repository root):
#   doc/project/enforcement/check-pull-request.sh --base origin/master --head refs/pr/<n> --pr <n>
#   doc/project/enforcement/check-pull-request.sh --base <sha> --head <sha> --description-file body.md \
#     --markdown report.md
#
# Exit status 0 = clean, 1 = violations, 2 = invalid usage or a gate could not run.

set -uo pipefail
exec python3 "$(dirname "$0")/check_pull_request.py" "$@"
