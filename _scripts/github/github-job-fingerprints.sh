#!/bin/env bash

# This script runs the fingerprints job on GitHub.
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
opp_featuretool enable all 2>&1 # redirecting stderr so it doesn't get out of sync with stdout
echo "::endgroup::"

echo "::group::Run fingerprint tests"
cd tests/fingerprint
# this indirectly calls the script named simply "inet", which handles the MODE envvar internally
./fingerprinttest | tee fingerprinttest.out
#                ^---- Everything from here on is only needed to make the pretty GitHub annotations. ----v
EXITCODE="${PIPESTATUS[0]}"
echo "::endgroup::"

# grep returns 1 if no lines were selected, we have to suppress that with "|| true"
FAILS=$(grep -P "PASS \\(unexpected\\)|FAILED \\(should be|ERROR \\(should be" fingerprinttest.out || true)

if [ -n "$FAILS" ]
then
    # newline characters are replaced with '%0A' to make them appear as multiline on the web UI
    # Source: https://github.com/actions/starter-workflows/issues/68#issuecomment-581479448
    # (Also: https://github.com/mheap/phpunit-github-actions-printer/pull/14 )
    echo "::warning::${FAILS//$'\n'/%0A}"
fi

exit $EXITCODE