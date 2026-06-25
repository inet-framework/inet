#!/bin/sh
# Run the Phase-0 observer demo and print the normalised packet-event trace.
set -e
INET_DIR="${INET_DIR:-/home/levy/workspace/inet}"
cd "$(dirname "$0")"

opp_run -u Cmdenv \
    -l "$INET_DIR/src/libINET.so" \
    -l ./out/clang-release/libprotocoltest.so \
    -n "$INET_DIR/src;." \
    -f omnetpp.ini "$@"
