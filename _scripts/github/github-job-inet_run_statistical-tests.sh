#!/bin/env bash

# This script runs the "more tests" job on GitHub.
#
# The following environment variables must be set when invoked:
#    GITHUB_WORKSPACE       - this is provided by GitHub, most likely has
#                             the value "/home/runner/work/inet/inet"


set -e # make the script exit with error if any executed command exits with error

# this is where the cloned INET repo is mounted into the container
cd $GITHUB_WORKSPACE

echo "::group::Installing pip"
apt install -y python3-pip
echo "::endgroup::"

echo "::group::Installing Python packages"
python3 -m pip install matplotlib numpy pandas scipy ipython cppyy sewar dask distributed IPython sqlalchemy optimparallel
echo "::endgroup::"

mkdir /root/workspace
export WORKSPACE_ROOT=/root/workspace

# NOTE: `ln -s` can't be used here because it confuses both
# OMNeT++ and INET when they are sometimes resolved to real paths.
cp -r /root/omnetpp-6.0.1-linux /root/workspace/omnetpp
cp -r $GITHUB_WORKSPACE /root/workspace/inet

cd /root/workspace
git clone https://github.com/ACassimiro/TSNsched
cd /root/workspace/TSNsched
git checkout 3f3bf663d196ec6c03e81a1e1392d4aefd158e3e
export TSNSCHED_ROOT=/root/workspace/TSNsched

. /root/workspace/omnetpp/setenv -f

cd /root/workspace/inet
. setenv -f

git clone https://github.com/inet-framework/statistics.git

echo "::group::Enable all features"
opp_featuretool enable all  2>&1 # redirecting stderr so it doesn't get out of sync with stdout
echo "::endgroup::"

echo "::group::Make Makefiles"
make makefiles
echo "::endgroup::"

echo "::group::Make message headers"
make -C src msgheaders smheaders
echo "::endgroup::"

echo "::group::Run tests"
# This is a magical "process substitution" for piping stderr into tee...
# Source: https://stackoverflow.com/a/692407/635587
 # the "| cat" is there to hide the exit code temporarily
inet_run_statistical_tests -m release --exclude-filter /osg/ > >(tee runtest.out) 2> >(tee runtest.err >&2) | cat
#                                                            ^---- Everything from here on is only needed to make the pretty GitHub annotations. ----v
EXITCODE="${PIPESTATUS[0]}"
echo "::endgroup::"

cp -r /root/workspace/inet $GITHUB_WORKSPACE # copy _new.png and _diff.png files back

ERRORS=$(cat runtest.err)
if [ -n "$ERRORS" ]
then
    # newline characters are replaced with '%0A' to make them appear as multiline on the web UI
    # Source: https://github.com/actions/starter-workflows/issues/68#issuecomment-581479448
    # (Also: https://github.com/mheap/phpunit-github-actions-printer/pull/14 )
    echo "::warning::${ERRORS//$'\n'/%0A}"
fi

DIFFPATTERN="([^\n]*\n[?] *\^\n)" # matches a line that looks like "?     ^" and the line before it

FAILPATTERN="test: FAIL |FAILED tests:|FAIL: [^0]"
UNRESOLVEDPATTERN="test: UNRESOLVED |UNRESOLVED tests: |UNRESOLVED: [^0]"
LINEPATTERN="([^\n]*($FAILPATTERN|$UNRESOLVEDPATTERN)[^\n]*\n)"

# the "| tr -d" is there only to get rid of the terminating NUL which is there because
# of "grep -z" which is needed to make the entire input look like one long line
FAILS=$(grep -P -zo "$DIFFPATTERN|$LINEPATTERN" runtest.out | tr -d '\0')

if [ -n "$FAILS" ]
then
    # newline characters are replaced with '%0A' to make them appear as multiline on the web UI
    # Source: https://github.com/actions/starter-workflows/issues/68#issuecomment-581479448
    # (Also: https://github.com/mheap/phpunit-github-actions-printer/pull/14 )
    echo "::warning::${FAILS//$'\n'/%0A}"
fi

exit $EXITCODE
