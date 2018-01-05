#!/bin/env bash

# This script runs the jobs on Travis.
#
# The following environment variables must be set when invoked:
#    TARGET_PLATFORM       - must be one of "linux", "windows", "macosx"
#    MODE                  - must be "debug" or "release"
#    RUN_FINGERPRINT_TESTS - must be "yes" or "no"

#    TRAVIS_REPO_SLUG      - this is provided by Travis, most likely contains "inet-framework/inet"


set -e # make the script exit with error if any executed command exits with error

echo -e "\nccache summary:\n"
ccache -s
echo -e "\n----\n"

export PATH="/root/omnetpp-5.3p2-$TARGET_PLATFORM/bin:$PATH"

# this is where the cloned INET repo is mounted into the container (as prescribed in /.travis.yml)
cd /$TRAVIS_REPO_SLUG

cp -r /root/nsc-0.5.3 3rdparty

# only enabling some features only with native compilation, because we don't [want to?] have cross-compiled ffmpeg and NSC
if [ "$TARGET_PLATFORM" = "linux" ]; then
    opp_featuretool enable VoIPStream VoIPStream_examples TCP_NSC TCP_lwIP
fi

make makefiles

if [ "$RUN_FINGERPRINT_TESTS" = "yes" ]; then
    export PATH=/usr/lib/ccache:$PATH
    make MODE=$MODE USE_PRECOMPILED_HEADER=no -j $(nproc)

    echo -e "\n---- build finished, starting fingerprint tests ----\n"

    cd tests/fingerprint
    if [ "$MODE" = "debug" ]; then
        ./fingerprinttest -d
    else
        ./fingerprinttest
    fi
else
    make MODE=$MODE -j $(nproc)
fi

echo -e "\nccache summary:\n"
ccache -s
echo -e "\n----\n"
