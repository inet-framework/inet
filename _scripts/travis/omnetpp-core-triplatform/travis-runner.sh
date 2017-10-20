#!/bin/env bash

# This script runs the jobs on Travis.
#
# The following environment variables must be set when invoked:
#    TARGET_PLATFORM       - must be one of "linux", "windows", "macosx"
#    MODE                  - must be "debug" or "release"
#    RUN_FINGERPRINT_TESTS - must be "yes" or "no"
#    TRAVIS_REPO_SLUG      - this is provided by Travis, most likely contains "inet-framework/inet"


set -e # make the script exit with error if any executed command exits with error


export PATH="/root/omnetpp-5.2-$TARGET_PLATFORM/bin:$PATH"

# this is where the cloned INET repo is mounted into the container (as prescribed in /.travis.yml)
cd /$TRAVIS_REPO_SLUG

make makefiles

make MODE=$MODE -j $(nproc)

POSTFIX=""
if [ "$MODE" = "debug" ]; then
    POSTFIX="_dbg"
fi

if [ "$RUN_FINGERPRINT_TESTS" = "yes" ]; then
    cd tests/fingerprint
    ./runDefaultTests.sh -e opp_run$POSTFIX
fi
