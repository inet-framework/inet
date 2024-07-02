//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/NetworkNamespaceContext.h"

#include "inet/common/IPrintableObject.h"
#include "inet/common/UnsharedNamespaceInitializer.h"

#include <cstdio>
#include <fcntl.h>

#ifdef __linux__
#include <sys/mount.h>
#endif

namespace inet {

std::map<std::string, int> localNetworkNamespaces; // network namespace which are not bound in the /proc filesystem

void createNetworkNamespace(const char *name, bool global)
{
#ifdef __linux__
    if (global) {
        std::string fullPath = std::string("/var/run/netns/") + name;
        int fd = open(fullPath.c_str(), O_RDONLY | O_CREAT | O_EXCL, 0);
        if (fd < 0)
            throw cRuntimeError("Cannot open file: %s", fullPath.c_str());
        close(fd);
        if (mount("/proc/self/ns/net", fullPath.c_str(), "none", MS_BIND, nullptr) < 0)
            throw cRuntimeError("Cannot mount: %s", fullPath.c_str());
    }
    else {
        int oldFd = open("/proc/self/ns/net", O_RDONLY);
        if (unshare(CLONE_NEWNET) < 0)
            throw cRuntimeError("Cannot unshare network namespace");
        int newFd = open("/proc/self/ns/net", O_RDONLY);
        if (newFd < 0)
            throw cRuntimeError("Cannot open file: /proc/self/ns/net");
        localNetworkNamespaces[name] = newFd;
        // switch back to the original namespace that was used before unshare
        setns(oldFd, 0);
    }
#else
    throw cRuntimeError("Network namespaces are only supported on Linux");
#endif
}

bool existsNetworkNamespace(const char *name)
{
#ifdef __linux__
    auto it = localNetworkNamespaces.find(name);
    if (it != localNetworkNamespaces.end())
        return true;
    else {
        std::string path = std::string("/var/run/netns/") + name;
        int fd = open(path.c_str(), O_RDONLY);
        if (fd >= 0) {
            close(fd);
            return true;
        }
    }
    return false;
#else
    throw cRuntimeError("Network namespaces are only supported on Linux");
#endif
}

void deleteNetworkNamespace(const char *name) {
#ifdef __linux__
    auto it = localNetworkNamespaces.find(name);
    if (it != localNetworkNamespaces.end()) {
        auto it = localNetworkNamespaces.find(name);
        close(it->second);
        localNetworkNamespaces.erase(it);
    }
    else {
        std::string path = std::string("/var/run/netns/") + name;
        if (umount2(path.c_str(), MNT_DETACH) != 0)
            throw cRuntimeError("Cannot umount file: %s", path.c_str());
        if (unlink(path.c_str()) != 0)
            throw cRuntimeError("Cannot unlink file: %s", path.c_str());
    }
#else
    throw cRuntimeError("Network namespaces are only supported on Linux");
#endif
}

NetworkNamespaceContext::NetworkNamespaceContext(const char *name)
{
    this->name = name;
    if (!opp_isempty(name)) {
#ifdef __linux__
        auto it = localNetworkNamespaces.find(name);
        if (it != localNetworkNamespaces.end()) {
            oldFd = UnsharedNamespaceInitializer::singleton.originalNetworkNamespaceFd;
            newFd = it->second;
            global = false;
        }
        else {
            oldFd = open("/proc/self/ns/net", O_RDONLY);
            if (oldFd == -1)
                throw cRuntimeError("Cannot open current network namespace: errno=%d (%s)", errno, strerror(errno));
            std::string path = std::string("/var/run/netns/") + name;
            newFd = open(path.c_str(), O_RDONLY);
            if (newFd == -1)
                throw cRuntimeError("Cannot open network namespace: %s, errno=%d (%s)", name, errno, strerror(errno));
            global = true;
        }
        EV_TRACE << "Switching to network namespace" << EV_FIELD(name) << EV_FIELD(newFd) << std::endl;
        if (setns(newFd, 0) != 0)
            throw cRuntimeError("Cannot switch to network namespace: %s, errno=%d (%s)", name, errno, strerror(errno));
#else
        throw cRuntimeError("Network namespaces are only supported on Linux");
#endif
    }
}

NetworkNamespaceContext::~NetworkNamespaceContext()
{
    if (newFd != -1) {
#ifdef __linux__
        EV_TRACE << "Switching back to network namespace" << EV_FIELD(name) << EV_FIELD(oldFd) << std::endl;
        if (setns(oldFd, 0) != 0)
            EV_FATAL << "Cannot switch to network namespace: " << name << ", errno=" << errno << " (" << strerror(errno) << ")" << EV_ENDL;
        if (global) {
            close(oldFd);
            close(newFd);
        }
        oldFd = -1;
        newFd = -1;
#endif
    }
}

} // namespace inet

