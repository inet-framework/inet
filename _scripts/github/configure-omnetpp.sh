#!/bin/env bash

# This script configures omnetpp for a job of GitHub Actions Workflows, without the compile, so that a
# test job can select its tests before it builds (rule T8 of plan/pending/master-guards.md).
# build-omnetpp.sh sources it, and then compiles.
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

echo "::group::Installing the omnetpp tools"
# make copies opp_featuretool, opp_makemake and the other tools of src/utils into bin/, which
# configure-inet.sh needs before the compile
make MODE=$MODE utils
echo "::endgroup::"
