//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/NetworkNamespaceContext.h"

#include <fcntl.h>

#include <cstdio>

namespace inet {

NetworkNamespaceContext::NetworkNamespaceContext(const char *networkNamespace)
{
    if (networkNamespace != nullptr && *networkNamespace != '\0') {
#ifdef __linux__
        oldNs = open("/proc/self/ns/net", O_RDONLY);
        std::string namespaceAsString = "/var/run/netns/";
        namespaceAsString += networkNamespace;
        newNs = open(namespaceAsString.c_str(), O_RDONLY);
        if (newNs == -1)
            throw cRuntimeError("Cannot open network namespace: errno=%d (%s)", errno, strerror(errno));
        if (setns(newNs, 0) == -1)
            throw cRuntimeError("Cannot change network namespace: errno=%d (%s)", errno, strerror(errno));
#else
        throw cRuntimeError("Network namespaces are only supported on Linux");
#endif
    }
}

NetworkNamespaceContext::~NetworkNamespaceContext()
{
    if (newNs != -1) {
#ifdef __linux__
        setns(oldNs, 0);
        close(oldNs);
        close(newNs);
        oldNs = -1;
        newNs = -1;
#endif
    }
}

} // namespace inet

