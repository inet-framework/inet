//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INETWORK_H
#define __INET_INETWORK_H

#include "inet/common/INETDefs.h"

namespace inet {

class INetworkNode;

class INET_API INetwork
{
  public:
    virtual ~INetwork() {}

    virtual int getNumNetworkNodes() const = 0;
    virtual INetworkNode *getNetworkNode(int i) const = 0;

    virtual INetworkNode *createNetworkNode() const = 0;
    virtual void addNetworkNode(INetworkNode *networkNode) = 0;
    virtual void removeNetworkNode(INetworkNode *networkNode) = 0;

    virtual void connectNetworkNodes(INetworkNode *networkNode1, INetworkNode *networkNode2) = 0;
    virtual void disconnectNetworkNodes(INetworkNode *networkNode1, INetworkNode *networkNode2) = 0;

    virtual void connectNetworkInterfaces(INetworkInterface *networkInterface1, INetworkNode *networkInterface2) = 0;
    virtual void disconnectNetworkInterfaces(INetworkInterface *networkInterface1, INetworkNode *networkInterface2) = 0;

    virtual IApplication *findApplication() const = 0;
    virtual IPacketQueue *findPacketQueue() const = 0;
    virtual INetworkNode *findNetworkNode() const = 0;
    virtual INetworkInterface *findNetworkInterface() const = 0;

    virtual void mapApplications() const = 0;
    virtual void mapPacketQueues() const = 0;
    virtual void mapNetworkNodes() const = 0;
    virtual void mapNetworkInterfaces() const = 0;
};

} // namespace inet

#endif

