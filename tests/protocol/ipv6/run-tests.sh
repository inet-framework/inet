#!/bin/bash
# Build + run the IPv6 conformance suite (Neighbor Discovery RFC 4861, DAD/SLAAC
# RFC 4862) via opp_test.
#
# Each test is a .test file whose `%file: <Name>.cc` carries a named
# Define_ProtocolTest program; opp_test extracts them all into ./work and a single
# --deep build links them (plus the shared Ipv6TestSupport.h + ../lib/libprotocoltest.so
# + INET) into ONE `ipv6tests` binary. Each test's %inifile selects its program via
# *.tester.testName and %contains asserts the verdict (CONFORMS -> PASS,
# NOT-MODELED -> the faithful spec assertion FAILs on its deadline).
#
# Usage:  ./run-tests.sh [test-file ...]   (default: all *.test under this folder)
#
# Requires ../lib/libprotocoltest.so (build it first: ../lib/build.sh) and a built INET
# (src/libINET.so). The omnetpp tools must be on PATH; if not, `source <inet>/setenv`.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
set -e
cd "$(dirname "$0")"

INET_DIR="${INET_DIR:-$(cd ../../.. && pwd)}"
LIB_DIR="$(cd ../lib && pwd)"
IPV6_DIR="$(pwd)"
TESTS="${*:-$(find . -name '*.test' | sort)}"

# Make sure the omnetpp tools are usable; fall back to the repo-local setenv.
command -v opp_test >/dev/null 2>&1 || . "$INET_DIR/setenv" -q 2>/dev/null || true

rm -rf work
mkdir -p work
opp_test gen $TESTS

cd work
printf 'LIBS += -Wl,-rpath,%s/src -Wl,-rpath,%s\n' "$INET_DIR" "$LIB_DIR" > makefrag
opp_makemake -f --deep -o ipv6tests \
    -I"$IPV6_DIR" -I"$LIB_DIR" -I"$INET_DIR/src" \
    -L"$INET_DIR/src" -lINET -L"$LIB_DIR" -lprotocoltest >/dev/null
make MODE="${MODE:-release}" -j"$(nproc)" >/dev/null
# opp_makemake drops the executable under out/<config>/; expose it as ./ipv6tests so
# `opp_test run -p ipv6tests` finds it.
BIN="$(find "$PWD" -name ipv6tests -type f -perm -u+x | head -1)"
[ -n "$BIN" ] && [ "$BIN" != "$PWD/ipv6tests" ] && ln -sf "$BIN" ipv6tests
cd ..

# NED path for the run (test inis carry no ned-path; they rely on this).
export NEDPATH="$IPV6_DIR/ned:$LIB_DIR:$INET_DIR/src"
opp_test run -p ipv6tests $TESTS
