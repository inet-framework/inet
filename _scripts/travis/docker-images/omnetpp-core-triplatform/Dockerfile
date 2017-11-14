FROM omnetpp-compilers

WORKDIR /root

RUN wget https://omnetpp.org/omnetpp/send/30-omnet-releases/2319-omnetpp-5-2-core \
        --referer=https://omnetpp.org/omnetpp -O omnetpp-5.2-src-core.tgz --progress=dot:giga && \
    tar xf omnetpp-5.2-src-core.tgz && rm omnetpp-5.2-src-core.tgz

WORKDIR /root/omnetpp-5.2

# these are optional now
RUN rm -rf samples

# fetching and applying the patch for INET 4
RUN wget https://raw.githubusercontent.com/inet-framework/inet/integration/misc/patch/inet-4.0-omnetpp-5.1_or_5.2.patch
RUN patch -p 1 -i inet-4.0-omnetpp-5.1_or_5.2.patch

WORKDIR /root

RUN cp -r omnetpp-5.2 omnetpp-5.2-macosx
RUN cp -r omnetpp-5.2 omnetpp-5.2-windows
RUN mv omnetpp-5.2 omnetpp-5.2-linux


#### building the linux version

WORKDIR omnetpp-5.2-linux

# Removing the default "-march native" options, as those are not practical in a Docker image.
# We have to echo into configure.user because assignments with spaces in the value can't be passed as command line
# arguments becauseof a bug in configure.in, and the environment variables are overridden by configure.user.
RUN echo "CFLAGS_RELEASE='-O3 -DNDEBUG=1 -D_XOPEN_SOURCE'" >> configure.user
# Adding O1 because without it, the INET fingerprint tests timed out on travis on a debug build, and we don't actually want to debug.
RUN echo "CFLAGS_DEBUG='-O1 -ggdb -Wall -Wextra -Wno-unused-parameter'" >> configure.user

ENV PATH /root/omnetpp-5.2-linux/bin:$PATH
RUN ./configure WITH_TKENV=no WITH_QTENV=no WITH_OSG=no WITH_OSGEARTH=no
RUN make -j $(nproc)


#### building the mac version using some linux tools

WORKDIR /root/omnetpp-5.2-macosx

# The _XOPEN_SOURCE macro has to be defined otherwise the deprecated context switching functions are not available.
# Also we remove the default "-march native" options, as those are not practical in a Docker image.
# We have to echo into configure.user because assignments with spaces in the value can't be passed as command line
# arguments becauseof a bug in configure.in, and the environment variables are overridden by configure.user.
RUN echo "CFLAGS_RELEASE='-O3 -DNDEBUG=1 -D_XOPEN_SOURCE'" >> configure.user
# Adding O1 because without it, the INET fingerprint tests timed out on travis on a debug build, and we don't actually want to debug.
RUN echo "CFLAGS_DEBUG='-O1 -ggdb -Wall -Wextra -Wno-unused-parameter'" >> configure.user

ENV PATH /root/omnetpp-5.2-macosx/bin:$PATH
# yes, msgc is nedtool too, and we have to use that because it redirects using $PATH, so it would pick up the macosx one
RUN ./configure WITH_TKENV=no WITH_QTENV=no WITH_OSG=no WITH_OSGEARTH=no --host="x86_64-apple-darwin15" \
        CXX="x86_64-apple-darwin15-clang++-libc++" CC="x86_64-apple-darwin15-clang" \
        MSGC="/root/omnetpp-5.2-linux/bin/nedtool" NEDTOOL="/root/omnetpp-5.2-linux/bin/nedtool"
RUN make -j $(nproc)


#### building the windows version using some linux tools

# ugly workaround for incorrect header name in sim/gettime.cc
RUN ln -s /usr/x86_64-w64-mingw32/include/windows.h /usr/x86_64-w64-mingw32/include/Windows.h

WORKDIR /root/omnetpp-5.2-windows

# Removing the default "-march native" options, as those are not practical in a Docker image.
# We have to echo into configure.user because assignments with spaces in the value can't be passed as command line
# arguments becauseof a bug in configure.in, and the environment variables are overridden by configure.user.
RUN echo "CFLAGS_RELEASE='-O3 -DNDEBUG=1'" >> configure.user
# Adding O1 because without it, the INET fingerprint tests timed out on travis on a debug build, and we don't actually want to debug.
RUN echo "CFLAGS_DEBUG='-O1 -ggdb -Wall -Wextra -Wno-unused-parameter'" >> configure.user

ENV PATH /root/omnetpp-5.2-windows/bin:$PATH
# yes, msgc is nedtool too, and we have to use that because it redirects using $PATH, so it would pick up the macosx one
RUN ./configure WITH_TKENV=no WITH_QTENV=no WITH_OSG=no WITH_OSGEARTH=no --host=x86_64-w64-mingw32 \
        MSGC="/root/omnetpp-5.2-linux/bin/nedtool" NEDTOOL="/root/omnetpp-5.2-linux/bin/nedtool"
RUN make -j $(nproc)

# switch back to using the linux tools
ENV PATH /root/omnetpp-5.2-linux/bin:$PATH

WORKDIR /root

