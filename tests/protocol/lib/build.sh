#!/bin/sh
# Build the protocol-test framework library.
#
# Phase 0 links against a pre-built INET checkout (headers + libINET.so) at the
# same commit, to avoid a full INET rebuild in this worktree. Override INET_DIR if
# your built INET lives elsewhere. (Proper in-tree build comes with the Phase 7
# harness integration.)
set -e
INET_DIR="${INET_DIR:-/home/levy/workspace/inet}"
cd "$(dirname "$0")"

printf 'LIBS += -Wl,-rpath,%s/src\n' "$INET_DIR" > makefrag
opp_makemake -f --deep -s -o protocoltest -I"$INET_DIR/src" -L"$INET_DIR/src" -lINET
make MODE="${MODE:-release}" -j"$(nproc)"
