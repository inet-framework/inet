#!/bin/env bash

# This script runs the queueingtests stage on Travis.
#
# The following environment variables must be set when invoked:
#    MODE                   - must be "debug" or "release"
#                             although ONLY "debug" does anything at the moment
#
#    TRAVIS_REPO_SLUG       - this is provided by Travis, most likely has the
#                             value "inet-framework/inet"


set -e # make the script exit with error if any executed command exits with error

echo -e "\nccache summary:\n"
ccache -s
echo -e ""

export PATH="/usr/lib/ccache:$PATH"

# this is where the cloned INET repo is mounted into the container (as prescribed in /.travis.yml)
cd /$TRAVIS_REPO_SLUG

cp -r /root/nsc-0.5.3 3rdparty

opp_featuretool enable all

# We have to explicitly enable diagnostics coloring to make ccache work,
# since we redirect stderr here, but not in the build stage.
# See https://github.com/ccache/ccache/issues/222
echo -e "CFLAGS += -fcolor-diagnostics\n\n$(cat src/makefrag)" > src/makefrag

echo -e "\nBuilding (silently)...\n"
make makefiles > /dev/null 2>&1
make MODE=$MODE USE_PRECOMPILED_HEADER=no -j $(nproc) > /dev/null 2>&1

echo -e "\nccache summary:\n"
ccache -s

echo -e "\nBuild finished, starting queueing tests..."

cd tests/queueing
if [ "$MODE" = "debug" ]; then
    ./runtest
fi

# TODO: add release mode when the runtest script can switch
# mode / executable based on command line arguments
