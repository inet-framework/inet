#!/bin/env bash

# This script configures INET for a job of GitHub Actions Workflows, without the compile, so that a
# test job can select its tests before it builds (TR-CI-SELECTION of doc/project/rule/testing.md).
# build-inet.sh sources it, and then compiles.
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

echo "::group::Making inet makefiles"
make makefiles
echo "::endgroup::"
