#!/bin/bash
# Build + run the WiFi conformance suite via opp_test.
#
# Each test is a .test file whose `%file: <Name>.cc` carries a named
# Define_ProtocolTest program; opp_test extracts them all into ./work and a single
# --deep build links them (plus the shared WifiTestSupport.h + ../lib/libprotocoltest.so
# + INET) into ONE `wifitests` binary. Each test's %inifile selects its program via
# *.tester.testName and %contains asserts the verdict (CONFORMS/DEVIATES/NOT-MODELED
# all surface as PASS/FAIL here).
#
# Usage:  ./run-tests.sh [test-file ...]   (default: all *.test under this folder)
#
# SPDX-License-Identifier: LGPL-3.0-or-later
set -e
cd "$(dirname "$0")"
. /home/levy/workspace/omnetpp/setenv -q 2>/dev/null || true

INET_DIR="${INET_DIR:-$(cd ../../.. && pwd)}"
LIB_DIR="${LIB_DIR:-$(cd ../lib && pwd)}"
WIFI_DIR="$(pwd)"
TESTS="${*:-$(find . -name '*.test' | sort)}"

rm -rf work
mkdir -p work
opp_test gen $TESTS

cd work
printf 'LIBS += -Wl,-rpath,%s/src -Wl,-rpath,%s\n' "$INET_DIR" "$LIB_DIR" > makefrag
opp_makemake -f --deep -o wifitests \
    -I"$WIFI_DIR" -I"$LIB_DIR" -I"$INET_DIR/src" \
    -L"$INET_DIR/src" -lINET -L"$LIB_DIR" -lprotocoltest >/dev/null
make MODE="${MODE:-release}" -j"$(nproc)" >/dev/null
# opp_makemake drops the executable under out/<config>/; expose it as ./wifitests so
# `opp_test run -p wifitests` finds it.
BIN="$(find "$PWD" -name wifitests -type f -perm -u+x | head -1)"
[ -n "$BIN" ] && [ "$BIN" != "$PWD/wifitests" ] && ln -sf "$BIN" wifitests
cd ..

# NED path for the run (test inis carry no ned-path; they rely on this).
export NEDPATH="$WIFI_DIR/ned:$LIB_DIR:$INET_DIR/src"
# Run opp_test (each test asserts the honest 'PROTOCOLTEST <name>: PASS' line, so opp_test's
# per-test result IS the true conformance outcome) and delegate the verdict to the opp_repl-backed
# reporter, which pairs each result with its '%# expected-result' declaration: a NOT-MODELED test
# reads 'FAIL (expected)' (green, not a fake PASS) and a closed gap surfaces as 'PASS (unexpected)'.
# opp_test's own aggregate exit is expected to be non-zero (the 35 NOT-MODELED tests fail on
# purpose); the reporter recomputes the real pass/fail from the expected-vs-actual comparison.
set +e
opp_test run -p wifitests $TESTS > work/opp_test.out 2>&1
set -e
python3 "$WIFI_DIR/report_expected_results.py" work/opp_test.out $TESTS
