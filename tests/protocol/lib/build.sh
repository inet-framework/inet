#!/bin/sh
# Build the protocol-test framework library.
#
# The Makefile is in git and uses relative paths, so this script only runs make. It stays
# because the run records of the standards passes name it, and because MODE defaults to
# release here while the test harness builds debug.
#
# Add or remove a source file and the Makefile needs regenerating. The command is in its
# header; run it from this folder.
set -e
cd "$(dirname "$0")"
make MODE="${MODE:-release}" -j"$(nproc)"
