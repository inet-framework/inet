#!/bin/bash
# Test-runner preamble with verified environment. Usage: runtests.sh '<filter-regex>'
cd /home/levy/workspace/omnetpp || exit 1
source setenv -q
cd /home/levy/workspace/inet-infrastructure || exit 1
source setenv -q
echo "INET_ROOT=$INET_ROOT"
if [ "$INET_ROOT" != "/home/levy/workspace/inet-infrastructure" ]; then
    echo "FATAL: wrong INET_ROOT" >&2
    exit 1
fi
if [ "$BUILD" = "1" ]; then
    make MODE=debug -j"$(nproc)" 2>&1 | tail -2 || exit 1
fi
opp_run_module_tests --load @opp --no-build ${1:+--filter "$1"} 2>&1 | sed 's/\x1b\[[0-9;]*m//g' | grep -E 'PASS|FAIL|TOTAL|◉'
