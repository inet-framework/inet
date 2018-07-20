#!/bin/env bash

# This script runs the build stage of the Travis builds.
#
# The following environment variables must be set when invoked:
#    TARGET_PLATFORM        - must be one of "linux", "windows", "macosx"
#    MODE                   - must be "debug" or "release"
#
#    TRAVIS_REPO_SLUG       - this is provided by Travis, most likely has the
#                             value "inet-framework/inet"


set -e # make the script exit with error if any executed command exits with error

ccache -cCz # we don't use ccache across builds, only across stages within each build

export PATH="/root/omnetpp-5.4.1-$TARGET_PLATFORM/bin:/usr/lib/ccache:$PATH"

# this is where the cloned INET repo is mounted into the container (as prescribed in /.travis.yml)
cd /$TRAVIS_REPO_SLUG

cp -r /root/nsc-0.5.3 3rdparty

# enabling some features only with native compilation, because we don't [want to?] have cross-compiled ffmpeg and NSC
if [ "$TARGET_PLATFORM" = "linux" ]; then
    opp_featuretool enable VoIPStream VoIPStream_examples TCP_NSC TCP_lwIP

    # In the fingerprints stage we have to force enable diagnostics coloring
    # to make ccache work well, see https://github.com/ccache/ccache/issues/222
    # We do it here as well to make the compiler arguments match.
    # Only when compiling to linux though, as for cross-compiling we use GCC,
    # and it has a different flag for this. And we only need this for linux anyway.
    echo -e "CFLAGS += -fcolor-diagnostics\n\n$(cat src/makefrag)" > src/makefrag

    # On linux we can't use precompiled headers, because ccache can't work with them,
    # and we need ccache, but we are fine without precompiled headers (on linux that is).
    PCH=no
else
    # When compiling to windows, we have to enable precompiled headers, otherwise the
    # debug builds take too long, sometimes exceeding 50 minutes.
    # And we don't need ccache here anyway (only the linux build is used in the second stage).
    # When compiling to macos, it doesn't really matter, but let's enable them just in case.
    PCH=yes
fi

if [ "$TARGET_PLATFORM" = "windows" ]; then
    # The --no-keep-memory flag decreased linking time by ~10 percent in debug builds
    # (a bit more in release), which is needed because otherwise it sometimes took over 10 minutes,
    # triggering the other kind of Travis timeout. The --stats option is just to check, and it
    # produces some output before the command terminates (before the .dll file is written to disk),
    # which also helps. We only need these with mingw, and they don't work with clang (for the macos builds).
    echo -e "LDFLAGS += -Wl,--stats -Wl,--no-keep-memory\n\n$(cat src/makefrag)" > src/makefrag
fi

make makefiles
make MODE=$MODE USE_PRECOMPILED_HEADER=$PCH -j $(nproc)

echo -e "\nccache summary:\n"
ccache -s
echo -e "\n----\n"
