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
// Authors: Levente Meszaros
//

#ifndef __INET_IPV4NODECONFIGURATOR_H
#define __INET_IPV4NODECONFIGURATOR_H

#include "inet/common/INETDefs.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/networklayer/configurator/ipv4/IPv4NetworkConfigurator.h"

namespace inet {

/**
 * This module provides the static configuration for the IPv4RoutingTable and
 * the IPv4 network interfaces of a particular node in the network.
 *
 * For more info please see the NED file.
 */
class IPv4NodeConfigurator : public cSimpleModule, public ILifecycle
{
  protected:
    NodeStatus *nodeStatus;
    IInterfaceTable *interfaceTable;
    IIPv4RoutingTable *routingTable;
    IPv4NetworkConfigurator *networkConfigurator;

  public:
    IPv4NodeConfigurator();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("this module doesn't handle messages, it runs only in initialize()"); }
    virtual void initialize(int stage) override;
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;
    virtual void prepareNode();
    virtual void prepareInterface(InterfaceEntry *interfaceEntry);
    virtual void configureNode();
};

} // namespace inet

#endif // ifndef __INET_IPV4NODECONFIGURATOR_H

