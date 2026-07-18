#!/bin/bash
# Build + run the protocolelement atomic conformance suite via opp_test.
#
# Each .test is fully SELF-CONTAINED: it carries its own tiny network
# (%file: test.ned, a source -> element(s) -> sink chain plus a ProtocolTester),
# its ProtocolTest program (%file: <Name>.cc), its config (%inifile) and the
# expected verdict (%contains: "PROTOCOLTEST <name>: PASS"). opp_test extracts
# them all into ./work and a single --deep build links them (+ ../lib/libprotocoltest.so
# + INET) into one `petests` binary; each test's %inifile selects its program via
# *.tester.testName.
#
# Usage:  ./run-tests.sh [test-file ...]     (default: all *.test in this folder)
#
# SPDX-License-Identifier: LGPL-3.0-or-later
set -e
cd "$(dirname "$0")"
. /home/levy/workspace/omnetpp/setenv -q 2>/dev/null || true

INET_DIR="${INET_DIR:-$(cd ../../.. && pwd)}"
LIB_DIR="${LIB_DIR:-$(cd ../lib && pwd)}"
HERE="$(pwd)"
TESTS="${*:-$(find . -maxdepth 1 -name '*.test' | sort)}"

rm -rf work
mkdir -p work
opp_test gen $TESTS

cd work
printf 'LIBS += -Wl,-rpath,%s/src -Wl,-rpath,%s\n' "$INET_DIR" "$LIB_DIR" > makefrag
opp_makemake -f --deep -o petests \
    -I"$HERE" -I"$LIB_DIR" -I"$INET_DIR/src" \
    -L"$INET_DIR/src" -lINET -L"$LIB_DIR" -lprotocoltest >/dev/null
make MODE="${MODE:-release}" -j"$(nproc)" >/dev/null
BIN="$(find "$PWD" -name petests -type f -perm -u+x | head -1)"
[ -n "$BIN" ] && [ "$BIN" != "$PWD/petests" ] && ln -sf "$BIN" petests
cd ..

# opp_test runs each test with its cwd = work/<name>/, so "." resolves to that test's own
# folder (its generated test.ned + package.ned). Pointing at the shared parent work/ instead
# would clash on the per-test package.ned files.
export NEDPATH=".:$LIB_DIR:$INET_DIR/src"
opp_test run -p petests $TESTS
