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

# MEGA HACK: When cross-building to Windows, make complains about this
# being a missing executable. Let's make sure it exists, but it doesn't
# matter exactly what it does, as it's only used to translate
# OMNETPP_IMAGE_PATH, which we don't use for testing at all.
ln -s /usr/bin/echo /usr/local/bin/cygpath

# this is where the cloned INET repo is mounted into the container
cd $GITHUB_WORKSPACE

. setenv -f

# activating ccache
export PATH=/usr/lib/ccache:$PATH

echo "::group::Enable all features"
opp_featuretool enable all 2>&1 # redirecting stderr so it doesn't get out of sync with stdout
echo "::endgroup::"

if [ "$TARGET_PLATFORM" != "linux" ]; then
    # Disabling some features when cross-compiling, because:
    # - we don't [want to?] have cross-compiled ffmpeg
    #   (and leaving it enabled messes up the include paths)
    # - ExternalInterface is only supported on Linux
    # - lwIP and NSC does not seem to compile on at least Windows, oh well...
    echo "::group::Disable some features"
    opp_featuretool disable \
        VoipStream VoipStreamExamples \
        NetworkEmulationSupport NetworkEmulationExamples NetworkEmulationShowcases \
        TcpLwip VisualizationOsg VisualizationOsgShowcases 2>&1
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
    sed -i 's|  TARGET_FILES+= $(TARGET_DIR)/$(TARGET_IMPDEF) $(TARGET_DIR)/$(TARGET_IMPLIB)||g' src/Makefile
    sed -i 's|$O/$(TARGET) $O/$(TARGET_IMPDEF) $O/$(TARGET_IMPLIB) &: $(OBJS)|$O/$(TARGET) : $(OBJS)|g' src/Makefile
fi

echo "::group::Build"
# This is a magical "process substitution" for piping stderr into tee...
# Redirecting stderr will cost us the pretty colors, but we'll manage...
# Source: https://stackoverflow.com/a/692407/635587
 # the "| cat" is there to hide the exit code temporarily
make MODE=$MODE -j $(nproc) 2> >(tee make.err >&2) | cat # meow
#                          ^---- Everything from here on is only needed to make the pretty GitHub annotations. ----v
EXITCODE="${PIPESTATUS[0]}"
echo "::endgroup::"

ERRORS=$(cat make.err)
if [ -n "$ERRORS" ]
then
    # newline characters are replaced with '%0A' to make them appear as multiline on the web UI
    # Source: https://github.com/actions/starter-workflows/issues/68#issuecomment-581479448
    # (Also: https://github.com/mheap/phpunit-github-actions-printer/pull/14 )
    echo "::warning::${ERRORS//$'\n'/%0A}"
fi

exit $EXITCODE
