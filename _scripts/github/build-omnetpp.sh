#!/bin/env bash

# This script runs the build job of GitHub Actions Workflows.
#
# The following environment variables must be set when invoked:
#    MODE                   - must be "debug" or "release"
#
#    GITHUB_WORKSPACE       - this is provided by GitHub, most likely has
#                             the value "/home/runner/work/inet/inet"

source "$(dirname "${BASH_SOURCE[0]}")/configure-omnetpp.sh"

echo "::group::Compiling omnetpp"
make MODE=$MODE -j $(nproc) base
echo "::endgroup::"
