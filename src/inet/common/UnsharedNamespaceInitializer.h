//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_UNSHARENAMESPACELISTENER_H
#define __INET_UNSHARENAMESPACELISTENER_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * This class uses the unshare() system call to create isolated user and network
 * namespaces. This is useful for many emulation scenarios including running
 * external networked processes, using virtual network interfaces, host OS
 * routing tables, etc. See also ExternalApp, ExternalProcess, ExternalEnvironment
 * modules.
 */
class UnsharedNamespaceInitializer : public omnetpp::cISimulationLifecycleListener
{
  public:
    static UnsharedNamespaceInitializer singleton;

    int originalNetworkNamespaceFd = -1;

  public:
    virtual void lifecycleEvent(SimulationLifecycleEventType eventType, cObject *details) override;

  private:
    void unshareUserNamespace();
    void unshareNetworkNamespace();

    void writeMapping(const char* path, const char* mapping);
};

}

#endif
