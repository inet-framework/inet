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

#include "inet/networklayer/configurator/ipv4/Ipv4NodeConfigurator.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/lifecycle/NodeOperations.h"

namespace inet {

Define_Module(Ipv4NodeConfigurator);

Ipv4NodeConfigurator::Ipv4NodeConfigurator()
{
    nodeStatus = nullptr;
    interfaceTable = nullptr;
    routingTable = nullptr;
    networkConfigurator = nullptr;
}

void Ipv4NodeConfigurator::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        cModule *node = getContainingNode(this);
        if (!node)
            throw cRuntimeError("The container @networkNode module not found");
        const char *networkConfiguratorPath = par("networkConfiguratorModule");
        nodeStatus = dynamic_cast<NodeStatus *>(node->getSubmodule("status"));
        interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        routingTable = getModuleFromPar<IIpv4RoutingTable>(par("routingTableModule"), this);

        if (!networkConfiguratorPath[0])
            networkConfigurator = nullptr;
        else {
            cModule *module = getModuleByPath(networkConfiguratorPath);
            if (!module)
                throw cRuntimeError("Configurator module '%s' not found (check the 'networkConfiguratorModule' parameter)", networkConfiguratorPath);
            networkConfigurator = check_and_cast<Ipv4NetworkConfigurator *>(module);
        }
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        if (!nodeStatus || nodeStatus->getState() == NodeStatus::UP)
            prepareNode();
    }
    else if (stage == INITSTAGE_NETWORK_LAYER_2) {
        if ((!nodeStatus || nodeStatus->getState() == NodeStatus::UP) && networkConfigurator)
            configureInterface();
    }
    else if (stage == INITSTAGE_NETWORK_LAYER_3) {
        if ((!nodeStatus || nodeStatus->getState() == NodeStatus::UP) && networkConfigurator)
            configureRoutingTable();
    }
}

bool Ipv4NodeConfigurator::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (static_cast<NodeStartOperation::Stage>(stage) == NodeStartOperation::STAGE_LINK_LAYER)
            prepareNode();
        else if (static_cast<NodeStartOperation::Stage>(stage) == NodeStartOperation::STAGE_NETWORK_LAYER && networkConfigurator) {
            configureInterface();
            configureRoutingTable();
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {    /*nothing to do*/
        ;
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {    /*nothing to do*/
        ;
    }
    else
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

void Ipv4NodeConfigurator::prepareNode()
{
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++)
        prepareInterface(interfaceTable->getInterface(i));
}

void Ipv4NodeConfigurator::prepareInterface(InterfaceEntry *interfaceEntry)
{
    ASSERT(!interfaceEntry->ipv4Data());
    Ipv4InterfaceData *interfaceData = new Ipv4InterfaceData();
    interfaceEntry->setIpv4Data(interfaceData);
    if (interfaceEntry->isLoopback()) {
        // we may reconfigure later it to be the routerId
        interfaceData->setIPAddress(Ipv4Address::LOOPBACK_ADDRESS);
        interfaceData->setNetmask(Ipv4Address::LOOPBACK_NETMASK);
        interfaceData->setMetric(1);
    }
    else {
        auto datarate = interfaceEntry->getDatarate();
        // TODO: KLUDGE: how do we set the metric correctly for both wired and wireless interfaces even if datarate is unknown
        if (datarate == 0)
            interfaceData->setMetric(1);
        else
            // metric: some hints: OSPF cost (2e9/bps value), MS KB article Q299540, ...
            interfaceData->setMetric((int)ceil(2e9 / datarate));    // use OSPF cost as default
        if (interfaceEntry->isMulticast()) {
            interfaceData->joinMulticastGroup(Ipv4Address::ALL_HOSTS_MCAST);
            if (routingTable->isForwardingEnabled())
                interfaceData->joinMulticastGroup(Ipv4Address::ALL_ROUTERS_MCAST);
        }
    }
}

void Ipv4NodeConfigurator::configureInterface()
{
    ASSERT(networkConfigurator);
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++)
        networkConfigurator->configureInterface(interfaceTable->getInterface(i));
}

void Ipv4NodeConfigurator::configureRoutingTable()
{
    ASSERT(networkConfigurator);
    if (par("configureRoutingTable"))
        networkConfigurator->configureRoutingTable(routingTable);
}

} // namespace inet

