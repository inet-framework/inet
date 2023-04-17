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

echo "::group::Running omnetpp setenv"
cd $GITHUB_WORKSPACE/omnetpp
cp configure.user.dist configure.user
. setenv -f
echo "::endgroup::"

echo "::group::Configuring omnetpp"
./configure WITH_LIBXML=yes WITH_QTENV=no WITH_OSG=yes WITH_OSGEARTH=no
echo "::endgroup::"
 
echo "::group::Compiling omnetpp"
make MODE=$MODE -j $(nproc) base
echo "::endgroup::"
