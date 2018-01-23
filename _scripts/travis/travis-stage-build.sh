#!/bin/env bash

# This script runs the jobs on Travis.
#
# The following environment variables must be set when invoked:
#    TARGET_PLATFORM        - must be one of "linux", "windows", "macosx"
#    MODE                   - must be "debug" or "release"
#
#    TRAVIS_REPO_SLUG       - this is provided by Travis, most likely has the
#                             value "inet-framework/inet"


set -e # make the script exit with error if any executed command exits with error

if [ "$TARGET_PLATFORM" = "windows" ]; then
    # we have to use ccache across builds when compiling to windows, because it is
    # just so darn slow that otherwise we would get intermittent job timeouts...
    echo -e "\nccache summary:\n"
    ccache -s
    echo -e "\n----\n"
else
    # on linux and macosx, we don't use the old cache contents for builds,
    # only populate it for the fingerprint stage (on linux that is)
    ccache -cC
fi

export PATH="/root/omnetpp-5.3p2-$TARGET_PLATFORM/bin:/usr/lib/ccache:$PATH"

# this is where the cloned INET repo is mounted into the container (as prescribed in /.travis.yml)
cd /$TRAVIS_REPO_SLUG

cp -r /root/nsc-0.5.3 3rdparty

# only enabling some features only with native compilation, because we don't [want to?] have cross-compiled ffmpeg and NSC
if [ "$TARGET_PLATFORM" = "linux" ]; then
    opp_featuretool enable VoIPStream VoIPStream_examples TCP_NSC TCP_lwIP
fi

make makefiles
make MODE=$MODE USE_PRECOMPILED_HEADER=no -j $(nproc)

echo -e "\nccache summary:\n"
ccache -s
echo -e "\n----\n"
