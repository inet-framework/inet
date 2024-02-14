//
// Copyright (C) 2008 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INETWORKNODE_H
#define __INET_INETWORKNODE_H

#include "inet/common/INETDefs.h"

namespace inet {

class INetwork;
class IInterfaceTable;
class IRoutingTable;

class INET_API INetworkNode
{
  public:
    virtual ~INetworkNode() {}

    virtual INetwork *getNetwork() const = 0;

    virtual IInterfaceTable *getInterfaceTable() const = 0;

    virtual IRoutingTable *getRoutingTable() const = 0;
    virtual IRoutingTable *getIpv4RoutingTable() const = 0;
    virtual IRoutingTable *getIpv6RoutingTable() const = 0;

    virtual void startup() const = 0;
    virtual void shutdown() const = 0;
    virtual void crash() const = 0;
};

} // namespace inet

#endif

