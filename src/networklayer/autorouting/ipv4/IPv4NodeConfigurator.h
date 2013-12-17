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

#include "INETDefs.h"
#include "ILifecycle.h"
#include "NodeStatus.h"
#include "IInterfaceTable.h"
#include "IRoutingTable.h"
#include "IPv4NetworkConfigurator.h"

/**
 TODO:
Have 3 stages for network configuration?
 - in stage N + 0: network configurator computes the configuration for the whole network
 - in stage N + 1: node configurators configure their interfaces
 - in stage N + 2: node configurators configure their routing table (must be after interface configuration to allow manual routes using names)

functions:
 - set IP address
 - find IP address by name

*/

/**
 * This module provides the static configuration for the IPv4RoutingTable and
 * the IPv4 network interfaces of a particular node in the network.
 *
 * For more info please see the NED file.
 */
class IPv4NodeConfigurator : public cSimpleModule, public ILifecycle {
    protected:
        NodeStatus *nodeStatus;
        IInterfaceTable *interfaceTable;
        IRoutingTable *routingTable;
        IPv4NetworkConfigurator *networkConfigurator;

    public:
        IPv4NodeConfigurator();

    protected:
        virtual int numInitStages() const { return 4; }
        virtual void handleMessage(cMessage *msg) { throw cRuntimeError("this module doesn't handle messages, it runs only in initialize()"); }
        virtual void initialize(int stage);
        virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);
        virtual void prepareNode();
        virtual void prepareInterface(InterfaceEntry *interfaceEntry);
        virtual void configureNode();
};

#endif
