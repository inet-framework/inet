#!/bin/sh
# Build the self-contained WiFi conformance suite into a single `wifitests` executable.
#
# Links the framework (../lib/libprotocoltest.so) and INET (../../../src/libINET.so) from
# the surrounding inet-protocoltest checkout -- both must already be built (run
# ../lib/build.sh first if libprotocoltest.so is missing). opp_makemake --deep collects
# every generation subfolder's .cc, so all tests compile into the one binary; the
# ProtocolTester's `testName` parameter selects which one runs.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
set -e
cd "$(dirname "$0")"

# INET source + libINET.so live at the inet-protocoltest root (tests/protocol/wifi -> ../../..).
INET_DIR="${INET_DIR:-$(cd ../../.. && pwd)}"
LIB_DIR="$(cd ../lib && pwd)"

# rpath so the produced executable finds libINET.so and libprotocoltest.so at runtime.
printf 'LIBS += -Wl,-rpath,%s/src -Wl,-rpath,%s\n' "$INET_DIR" "$LIB_DIR" > makefrag

opp_makemake -f --deep -o wifitests \
    -I"$INET_DIR/src" -I"$LIB_DIR" \
    -L"$INET_DIR/src" -lINET \
    -L"$LIB_DIR" -lprotocoltest

make MODE="${MODE:-release}" -j"$(nproc)"
