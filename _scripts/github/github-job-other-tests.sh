#!/bin/env bash

# This script runs the "other tests" job on GitHub.
#
# The following environment variables must be set when invoked:
#    TESTDIR                - must be the name of a subfolder in the tests directory
#                             in which all tests can be run simply as: ./runtest
#                             (such as: module, packet, queueing, unit, ...)
#
#    GITHUB_WORKSPACE       - this is provided by GitHub, most likely has
#                             the value "/home/runner/work/inet/inet"


set -e # make the script exit with error if any executed command exits with error


export PATH="/root/omnetpp-6.0pre9-linux/bin:$PATH"

# this is where the cloned INET repo is mounted into the container
cd $GITHUB_WORKSPACE

. setenv -f

cp -r /root/nsc-0.5.3 3rdparty

echo "::group::Enable all features"
opp_featuretool enable all
echo "::endgroup::"

echo "::group::Make Makefiles"
make makefiles
echo "::endgroup::"

echo "::group::Make message headers"
make -C src msgheaders smheaders
echo "::endgroup::"

echo "::group::Run $TESTDIR tests"
cd tests/$TESTDIR
./runtest
echo "::endgroup::"

