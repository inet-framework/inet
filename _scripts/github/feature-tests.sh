#!/bin/env bash

# This script runs the featuretest job on GitHub.
#
# The following environment variables must be set when invoked:
#    SKIPPED_FEATURES
#    SPLIT_INDEX
#    SPLIT_TOTAL
#
#    GITHUB_WORKSPACE       - this is provided by GitHub, most likely has
#                             the value "/home/runner/work/inet/inet"


set -e # make the script exit with error if any executed command exits with error

# this is where the cloned INET repo is mounted into the container
cd $GITHUB_WORKSPACE

. setenv -f

echo "::group::Run feature tests"
cd tests/features
./featuretest | tee featuretest.out
#             ^---- Everything from here on is only needed to make the pretty GitHub annotations. ----v
EXITCODE="${PIPESTATUS[0]}"
echo "::endgroup::"

# grep returns 1 if no lines were selected, we have to suppress that with "|| true"
FAILS=$(grep -P ": FAIL" featuretest.out || true)

if [ -n "$FAILS" ]
then
    # newline characters are replaced with '%0A' to make them appear as multiline on the web UI
    # Source: https://github.com/actions/starter-workflows/issues/68#issuecomment-581479448
    # (Also: https://github.com/mheap/phpunit-github-actions-printer/pull/14 )
    echo "::warning::${FAILS//$'\n'/%0A}"
fi

exit $EXITCODE
