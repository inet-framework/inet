#!/bin/env bash

# This script runs the fingerprints stage on Travis.
# All arguments are passed to the fingerprinttest script.
#
# The following environment variables must be set when invoked:
#    TARGET_PLATFORM        - must be one of "linux", "windows", "macosx"
#    MODE                   - must be "debug" or "release"
#
#    TRAVIS_REPO_SLUG       - this is provided by Travis, most likely has the
#                             value "inet-framework/inet"


set -e # make the script exit with error if any executed command exits with error

echo -e "\nccache summary:\n"
ccache -s
echo -e ""

export PATH="/root/omnetpp-5.3p2-$TARGET_PLATFORM/bin:/usr/lib/ccache:$PATH"

# this is where the cloned INET repo is mounted into the container (as prescribed in /.travis.yml)
cd /$TRAVIS_REPO_SLUG

cp -r /root/nsc-0.5.3 3rdparty

# only enabling some features only with native compilation, because we don't [want to?] have cross-compiled ffmpeg and NSC
if [ "$TARGET_PLATFORM" = "linux" ]; then
    opp_featuretool enable VoIPStream VoIPStream_examples TCP_NSC TCP_lwIP
fi

echo -e "\nBuilding...\n"
make makefiles > /dev/null
make MODE=$MODE USE_PRECOMPILED_HEADER=no -j $(nproc) > /dev/null
echo -e "\nccache summary:\n"
ccache -s

echo -e "\nBuild finished, starting fingerprint tests..."
echo -e "Additional arguments passed to fingerprint test script: " $@ "\n"

cd tests/fingerprint
if [ "$MODE" = "debug" ]; then
    ./fingerprints -e opp_run_dbg $@
else
    ./fingerprints -e opp_run_release $@
fi
