//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <cstdio>
#include <fcntl.h>

#include "inet/common/NetworkNamespaceContext.h"
#include "inet/common/LinuxUtils.h"

namespace inet {

using namespace utils;

NetworkNamespaceContext::NetworkNamespaceContext(const char *networkNamespace)
{
    if (networkNamespace != nullptr && *networkNamespace != '\0') {
        ensureNamespaceExists(networkNamespace);
#ifdef __linux__
        oldNs = open("/proc/self/ns/net", O_RDONLY);
        std::string namespaceAsString = "/var/run/netns/";
        namespaceAsString += networkNamespace;
        newNs = open(namespaceAsString.c_str(), O_RDONLY);
        if (newNs == -1)
            throw cRuntimeError("Cannot open network namespace");
        if (setns(newNs, 0) == -1)
            throw cRuntimeError("Cannot change network namespace");
#else
    throw cRuntimeError("Network namespaces are only supported on Linux");
#endif
    }
}

bool NetworkNamespaceContext::checkNamespaceExists(const char *networkNamespace)
{
#ifdef __linux__
    return fileExists((std::string("/var/run/netns/") + networkNamespace).c_str());
#else
    throw cRuntimeError("Network namespaces are only supported on Linux");
#endif
}

void NetworkNamespaceContext::ensureNamespaceExists(const char *networkNamespace)
{
#ifdef __linux__
    if (!checkNamespaceExists(networkNamespace))
        execCommand({"ip", "netns", "add", networkNamespace}, nullptr, true, true);
#else
    throw cRuntimeError("Network namespaces are only supported on Linux");
#endif
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

