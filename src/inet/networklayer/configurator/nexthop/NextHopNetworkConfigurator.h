//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NEXTHOPNETWORKCONFIGURATOR_H
#define __INET_NEXTHOPNETWORKCONFIGURATOR_H

#include "inet/networklayer/configurator/base/L3NetworkConfiguratorBase.h"

namespace inet {

class PatternMatcher;

/**
 * This module configures NextHopRoutingTable modules for a network.
 *
 * For more info please see the NED file.
 */
class INET_API NextHopNetworkConfigurator : public L3NetworkConfiguratorBase
{
  protected:
    // parameters
    bool addStaticRoutesParameter;

    // internal state
    Topology topology;

  protected:
    virtual void initialize(int stage) override;

    virtual IRoutingTable *findRoutingTable(Node *node) override;

    /**
     * Adds static routes to all routing tables in the network.
     * The algorithm uses Dijkstra's weighted shortest path algorithm.
     * May add default routes and subnet routes if possible and requested.
     */
    virtual void addStaticRoutes(Topology& topology);

    virtual void dumpRoutes(Topology& topology);
};

} // namespace inet

#endif

