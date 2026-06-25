#!/bin/sh
# Run the Phase-0 observer demo and print the normalised packet-event trace.
set -e
cd "$(dirname "$0")"
INET_DIR="${INET_DIR:-$(cd ../../.. && pwd)}"

LIB="$(ls "$INET_DIR"/out/*/tests/protocol/lib/libprotocoltest.so 2>/dev/null | head -1)"
[ -n "$LIB" ] || { echo "libprotocoltest.so not found -- run ./build.sh first" >&2; exit 1; }

opp_run -u Cmdenv \
    -l "$INET_DIR/src/libINET.so" \
    -l "$LIB" \
    -n "$INET_DIR/src;." \
    -f omnetpp.ini "$@"
