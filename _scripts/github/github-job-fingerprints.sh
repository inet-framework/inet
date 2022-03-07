#!/bin/env bash

# This script runs the fingerprints job on GitHub.
#
# The following environment variables must be set when invoked:
#    MODE                   - must be "debug" or "release"
#
#    GITHUB_WORKSPACE       - this is provided by GitHub, most likely has
#                             the value "/home/runner/work/inet/inet"
# Optionally:
#    SPLIT_N                - the number of partitions to split the tests into (default 1)
#    SPLIT_I                - the index of the partition to run in this invocation (default 0)

set -e # make the script exit with error if any executed command exits with error

# this is where the cloned INET repo is mounted into the container
cd $GITHUB_WORKSPACE

. setenv -f

echo "::group::Enable all features"
opp_featuretool enable all 2>&1 # redirecting stderr so it doesn't get out of sync with stdout
echo "::endgroup::"

SPLIT_N="${SPLIT_N:-1}"
SPLIT_I="${SPLIT_I:-0}"

echo "::group::Run fingerprint tests"
cd tests/fingerprint
# this indirectly calls the script named simply "inet", which handles the MODE envvar internally
./fingerprinttest -n $SPLIT_N -i $SPLIT_I -f 'tplx' -f '~tNl' | tee fingerprinttest.out
./fingerprinttest -n $SPLIT_N -i $SPLIT_I -f 'tplx' -f '~tND' | tee fingerprinttest.out
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
