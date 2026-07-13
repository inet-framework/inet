#!/bin/bash
# Run one INET simulation with verified environment.
# Usage: runsim.sh <workdir-relative-to-INET_ROOT> [extra opp_run args...]
cd /home/levy/workspace/omnetpp || exit 1
source setenv -q
cd /home/levy/workspace/inet-infrastructure || exit 1
source setenv -q
if [ "$INET_ROOT" != "/home/levy/workspace/inet-infrastructure" ]; then
    echo "FATAL: wrong INET_ROOT=$INET_ROOT" >&2
    exit 1
fi
if [ "$BUILD" = "1" ]; then
    make MODE=debug -j"$(nproc)" 2>&1 | tail -2 || exit 1
fi
WORKDIR="$1"; shift
cd "$INET_ROOT/$WORKDIR" || exit 1
exec inet_dbg -u Cmdenv "$@"
