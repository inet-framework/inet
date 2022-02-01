//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPV4FLATNETWORKCONFIGURATOR_H
#define __INET_IPV4FLATNETWORKCONFIGURATOR_H

#include "inet/networklayer/contract/ipv4/Ipv4Address.h"

namespace inet {

class IInterfaceTable;
class IIpv4RoutingTable;

/**
 * Configures Ipv4 addresses and routing tables for a "flat" network,
 * "flat" meaning that all hosts and routers will have the same
 * network address.
 *
 * For more info please see the NED file.
 */
class INET_API Ipv4FlatNetworkConfigurator : public cSimpleModule
{
  protected:
    class NodeInfo {
      public:
        bool isIPNode = false;
        IInterfaceTable *ift = nullptr;
        IIpv4RoutingTable *rt = nullptr;
        Ipv4Address address;
        bool usesDefaultRoute = false;
        bool ipForwardEnabled = false;
    };
    typedef std::vector<NodeInfo> NodeInfoVector;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    virtual void extractTopology(cTopology& topo, NodeInfoVector& nodeInfo);
    virtual void assignAddresses(cTopology& topo, NodeInfoVector& nodeInfo);
    virtual void addDefaultRoutes(cTopology& topo, NodeInfoVector& nodeInfo);
    virtual void fillRoutingTables(cTopology& topo, NodeInfoVector& nodeInfo);

    virtual void setDisplayString(cTopology& topo, NodeInfoVector& nodeInfo);
};

} // namespace inet

#endif

