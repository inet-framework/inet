#!/bin/bash
# Env-gated INET fingerprint suite runner (avoids the INET_ROOT poisoning trap:
# both setenvs must run, omnetpp first, then the target INET worktree).
#
# Usage: runfingerprints.sh <inet-worktree-dir> <result-file> [extra opp_run_fingerprint_tests args...]
# Example:
#   runfingerprints.sh /home/levy/workspace/inet-fp-master /tmp/fp-master.json -f tcp
set -e
WORKTREE=$1
RESULT=$2
shift 2
cd /home/levy/workspace/omnetpp && source setenv -q
cd "$WORKTREE" && source setenv -q
exec opp_run_fingerprint_tests --load @opp -p inet --no-build -m release \
    --result-file "$RESULT" "$@"
