#!/bin/env bash

# This script runs the fingerprints job on GitHub.
# All arguments are passed to the fingerprinttest script.
#
# The following environment variables must be set when invoked:
#    MODE                   - must be "debug" or "release"
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

echo "::group::Run fingerprint tests"
echo -e "Additional arguments passed to fingerprint test script: " $@ "\n"

cd tests/fingerprint
if [ "$MODE" = "debug" ]; then
    ./fingerprinttest -d "$@" -a --cmdenv-express-mode=true
else
    ./fingerprinttest "$@" -a --cmdenv-express-mode=true
fi
echo "::endgroup::"