#!/bin/env bash

# This script runs the jobs on Travis.
#
# The following environment variables must be set when invoked:
#    TARGET_PLATFORM       - must be one of "linux", "windows", "macosx"
#    MODE                  - must be "debug" or "release"
#    RUN_FINGERPRINT_TESTS - must be "yes" or "no"

#    TRAVIS_REPO_SLUG      - this is provided by Travis, most likely contains "inet-framework/inet"


set -e # make the script exit with error if any executed command exits with error

ccache -s

export PATH="/root/omnetpp-5.2-$TARGET_PLATFORM/bin:$PATH"

# this is where the cloned INET repo is mounted into the container (as prescribed in /.travis.yml)
cd /$TRAVIS_REPO_SLUG

# only enabling VoIP on native compilation, because we don't [want to?] have cross-compiled ffmpeg
if [ "$TARGET_PLATFORM" = "linux" ]; then
    opp_featuretool enable VoIPStream
    opp_featuretool enable VoIPStream_examples
fi

make makefiles


if [ "$RUN_FINGERPRINT_TESTS" = "yes" ]; then
    export PATH=/usr/lib/ccache:$PATH
    make MODE=$MODE USE_PRECOMPILED_HEADER=no -j $(nproc)

    POSTFIX=""
    if [ "$MODE" = "debug" ]; then
        POSTFIX="_dbg"
    fi

    cd tests/fingerprint
    ./fingerprinttest -e opp_run$POSTFIX
else
    make MODE=$MODE -j $(nproc)
fi

ccache -s
