//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/common/UnsharedNamespaceInitializer.h"

#include <fcntl.h>
#include <fstream>

#ifdef __linux__
#include <sys/prctl.h>
#endif

namespace inet {

Register_GlobalConfigOption(CFGID_UNSHARE_NAMESPACES, "unshare-namespaces", CFG_BOOL, "false", "Unshares the user and network namespaces using the unshare() system call. The simulation continues running as the root user in the new user namespace. This allows creating new network namespaces and assign network resources without using sudo or setting capabilities for opp_run.");

UnsharedNamespaceInitializer UnsharedNamespaceInitializer::singleton;

#ifdef __linux__
#if OMNETPP_VERSION < 0x0700
EXECUTE_ON_STARTUP(getEnvir()->addLifecycleListener(&UnsharedNamespaceInitializer::singleton));
#endif
#endif

void UnsharedNamespaceInitializer::lifecycleEvent(SimulationLifecycleEventType eventType, cObject *details)
{
#if OMNETPP_VERSION < 0x0700
    // TODO KLUDGE LF_ON_STARTUP removed
    if (eventType == LF_ON_STARTUP) {
        auto config = cSimulation::getActiveEnvir()->getConfig();
        if (config->getAsBool(CFGID_UNSHARE_NAMESPACES)) {
            // unsharing the user namespace allows to switch to the root user
            // which in turn allows to freely create network namespaces for external processes, etc.
            unshareUserNamespace();
            // unsharing the network namespace allows switching back to the original network namespace
            // from additional network namespaces created later, also this network namespace is completely
            // isolated from the host OS
            unshareNetworkNamespace();
        }
    }
#endif
}

void UnsharedNamespaceInitializer::unshareUserNamespace()
{
#ifdef __linux__
    // NOTE:  setting the state of the "dumpable" attribute is not striclty needed for unsharing the user and network namespace
    //        but on Ubuntu 24.04 the system doesn't allow reading the /proc/self/ns/net file
    prctl(PR_SET_DUMPABLE, 1, 0, 0, 0);
    pid_t originalUid = getuid();
    pid_t originalGid = getgid();
    if (unshare(CLONE_NEWUSER) < 0)
        throw cRuntimeError("Failed to unshare user namespace");
    // set up UID mapping, mapping root in the namespace to the original user outside
    char uid_map[50];
    snprintf(uid_map, sizeof(uid_map), "0 %d 1", originalUid);
    writeMapping("/proc/self/uid_map", uid_map);
    // update /proc/self/setgroups to deny setgroups system call for safety
    writeMapping("/proc/self/setgroups", "deny");
    // set up GID mapping, mapping root in the namespace to the original user outside
    char gid_map[50];
    snprintf(gid_map, sizeof(gid_map), "0 %d 1", originalGid);
    writeMapping("/proc/self/gid_map", gid_map);
    // change effective user to root
    if (seteuid(0) < 0)
        throw cRuntimeError("Failed to switch to the root user");
#endif
}

void UnsharedNamespaceInitializer::unshareNetworkNamespace()
{
#ifdef __linux__
    if (unshare(CLONE_NEWNET) < 0)
        throw cRuntimeError("Failed to unshare network namespace");
    originalNetworkNamespaceFd = open("/proc/self/ns/net", O_RDONLY);
    if (originalNetworkNamespaceFd == -1)
        throw cRuntimeError("Cannot open current network namespace: errno=%d (%s)", errno, strerror(errno));
#endif
}

void UnsharedNamespaceInitializer::writeMapping(const char* path, const char* mapping)
{
    std::ofstream file(path);
    if (!file.is_open())
        throw cRuntimeError("Failed to open file: %s", path);
    file << mapping;
    if (file.fail())
        throw cRuntimeError("Failed to write mapping: %s", mapping);
    file.close();
}

} // namespace inet

