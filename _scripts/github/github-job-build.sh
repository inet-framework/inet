#!/bin/env bash

# This script runs the build job of the GitHub Actions Workflow.
#
# The following environment variables must be set when invoked:
#    TARGET_PLATFORM        - must be one of "linux", "windows", "macosx"
#    MODE                   - must be "debug" or "release"
#
#    GITHUB_WORKSPACE       - this is provided by GitHub, most likely has
#                             the value "/home/runner/work/inet/inet"

set -e # make the script exit with error if any executed command exits with error

export PATH="/root/omnetpp-6.0pre9-$TARGET_PLATFORM/bin:$PATH"

# this is where the cloned INET repo is mounted into the container
cd $GITHUB_WORKSPACE

. setenv -f

cp -r /root/nsc-0.5.3 3rdparty

echo "::group::Enable all features"
opp_featuretool enable all
echo "::endgroup::"

if [ "$TARGET_PLATFORM" != "linux" ]; then
    # Disabling some features when cross-compiling, because:
    # - we don't [want to?] have cross-compiled ffmpeg
    #   (and leaving it enabled messes up the include paths)
    # - ExternalInterface is only supported on Linux
    # - lwIP and NSC does not seem to compile on at least Windows, oh well...
    echo "::group::Disable some features"
    opp_featuretool disable VoIPStream VoIPStream_examples ExternalInterface ExternalInterface_examples emulation_showcases TCP_lwIP TCP_NSC
    echo "::endgroup::"
fi

echo "::group::Make Makefiles"
make makefiles
echo "::endgroup::"

if [ "$TARGET_PLATFORM" = "windows" ]; then
    # This is here to stop make from invoking the final .dll linker twice, seeing that
    # both the .dll and the import lib for it (.dll.a) are targets, that have to be
    # made the same way. This is a problem because when cross-compiling to mingw, in
    # debug mode, one linker needs 5GB+ RAM, and it won't fit twice on the CI machines.
    # This workaround will not be necessary once the tester Docker image includes:
    # - A newer opp_makemake that generates a group target for the .dll and .dll.a files
    # - GNU make 4.3 that supports group targets (this is in ubuntu:20.10)
    sed -i 's|  TARGET_FILES+= $(TARGET_DIR)/$(TARGET_IMPLIB)||g' src/Makefile
    sed -i 's|$O/$(TARGET) $O/$(TARGET_IMPLIB): $(OBJS)|$O/$(TARGET): $(OBJS)|g' src/Makefile
fi

echo "::group::Build"
make MODE=$MODE -j $(nproc)
echo "::endgroup::"