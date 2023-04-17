#!/bin/env bash

# This script runs the build job of GitHub Actions Workflows.
#
# The following environment variables must be set when invoked:
#    MODE                   - must be "debug" or "release"
#
#    GITHUB_WORKSPACE       - this is provided by GitHub, most likely has
#                             the value "/home/runner/work/inet/inet"

echo "::group::Configuring ccache"
export PATH=/usr/lib/ccache:$PATH
export CCACHE_DIR=/home/runner/work/ccache
echo "::endgroup::"

echo "::group::Running inet setenv"
cd $GITHUB_WORKSPACE/inet
. setenv -f
echo "::endgroup::"

echo "::group::Enabling all inet features"
opp_featuretool enable all
echo "::endgroup::"

echo "::group::Disabling some inet features"
opp_featuretool disable SelfDoc
echo "::endgroup::"

echo "::group::Making inet makefiles"
make makefiles
echo "::endgroup::"

echo "::group::Compiling inet"
make MODE=$MODE -j $(nproc)
echo "::endgroup::"
