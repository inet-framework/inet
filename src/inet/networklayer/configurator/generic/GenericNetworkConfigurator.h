//
// Copyright (C) 2012 Opensim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
//
// Authors: Levente Meszaros (primary author), Andras Varga, Tamas Borbely
//

#ifndef __INET_GENERICNETWORKCONFIGURATOR_H
#define __INET_GENERICNETWORKCONFIGURATOR_H

#include "inet/networklayer/configurator/base/NetworkConfiguratorBase.h"

namespace inet {

class PatternMatcher;

/**
 * This module configures generic routing tables for a network.
 *
 * For more info please see the NED file.
 */
class INET_API GenericNetworkConfigurator : public NetworkConfiguratorBase
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

#endif // ifndef __INET_GENERICNETWORKCONFIGURATOR_H

