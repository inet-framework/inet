#!/usr/bin/env bash
#
# RR-WHATSNEW-FILE over a range of commits: no commit adds or changes an entry in the section of the
# version in development of WHATSNEW, and each entry file that a commit touches has the folder, the
# name and the format of doc/whatsnew/README.md. It reads only git objects, so the pull-request
# workflow can run it on untrusted content. The work is in whatsnew.py.
#
# Usage (from the INET repository root):
#   doc/project/enforcement/check-whatsnew.sh origin/master..HEAD
#
# Exit status 0 = clean, 1 = violations, 2 = invalid usage.

set -uo pipefail
if [ $# -ne 1 ]; then
  echo "usage: $0 BASE..HEAD" >&2
  exit 2
fi
exec python3 "$(dirname "$0")/whatsnew.py" check --range "$1"
