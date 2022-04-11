//
// Copyright (C) 2005 Eric Wu
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPV6FLATNETWORKCONFIGURATOR_H
#define __INET_IPV6FLATNETWORKCONFIGURATOR_H

#include "inet/common/INETDefs.h"
#include "inet/common/Topology.h"

namespace inet {

/**
 * Configures Ipv6 addresses and routing tables for a "flat" network,
 * "flat" meaning that all hosts and routers will have the same
 * network address.
 *
 * For more info please see the NED file.
 */
class INET_API Ipv6FlatNetworkConfigurator : public cSimpleModule
{
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    virtual void configureAdvPrefixes(Topology& topo);
    virtual void addOwnAdvPrefixRoutes(Topology& topo);
    virtual void addStaticRoutes(Topology& topo);

    virtual void setDisplayString(int numIPNodes, int numNonIPNodes);
    virtual bool isIPNode(Topology::Node *node);
};

} // namespace inet

#endif

