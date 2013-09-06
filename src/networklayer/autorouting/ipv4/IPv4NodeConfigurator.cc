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

#include "IPv4NodeConfigurator.h"
#include "ModuleAccess.h"
#include "InterfaceTableAccess.h"
#include "IPv4RoutingTableAccess.h"
#include "NodeStatus.h"
#include "NodeOperations.h"
#include "opp_utils.h"   // for OPP_Global::getModuleByPath()

Define_Module(IPv4NodeConfigurator);

IPv4NodeConfigurator::IPv4NodeConfigurator()
{
    nodeStatus = NULL;
    interfaceTable = NULL;
    routingTable = NULL;
    networkConfigurator = NULL;
}

EXECUTE_ON_STARTUP(
    ASSERT(STAGE_DO_CONFIGURE_IP_ADDRESSES >= STAGE_DO_LOCAL);
    ASSERT(STAGE_DO_CONFIGURE_IP_ADDRESSES >= STAGE_DO_ADD_IP_PROTOCOLDATA_TO_INTERFACEENTRY);
);

int IPv4NodeConfigurator::numInitStages() const { return STAGE_DO_CONFIGURE_IP_ADDRESSES + 1; }

void IPv4NodeConfigurator::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == STAGE_DO_LOCAL)
    {
        cModule *node = findContainingNode(this);
        if (!node)
            throw cRuntimeError("The container @node module not found");
        const char *networkConfiguratorPath = par("networkConfiguratorModule");
        nodeStatus = dynamic_cast<NodeStatus *>(node->getSubmodule("status"));
        interfaceTable = InterfaceTableAccess().get();
        routingTable = AddressResolver().findIPv4RoutingTableOf(node);

        if (!networkConfiguratorPath[0])
            networkConfigurator = NULL;
        else {
#if OMNETPP_VERSION < 0x0403
            cModule *module = OPP_Global::getModuleByPath(this, networkConfiguratorPath);  // compatibility
#else
            cModule *module = getModuleByPath(networkConfiguratorPath);
#endif
            if (!module)
                throw cRuntimeError("Configurator module '%s' not found (check the 'networkConfiguratorModule' parameter)", networkConfiguratorPath);
            networkConfigurator = check_and_cast<IPv4NetworkConfigurator *>(module);
        }
    }
    if (stage == STAGE_DO_ADD_IP_PROTOCOLDATA_TO_INTERFACEENTRY)
    {
        ASSERT(stage >= STAGE_INTERFACEENTRY_REGISTERED);
        ASSERT(stage >= STAGE_NODESTATUS_AVAILABLE);

        if (!nodeStatus || nodeStatus->getState() == NodeStatus::UP)
            prepareNode();
    }
    if (stage == STAGE_DO_CONFIGURE_IP_ADDRESSES)
    {
        ASSERT(stage >= STAGE_NODESTATUS_AVAILABLE);
        ASSERT(stage > STAGE_DO_COMPUTE_IP_AUTOCONFIGURATION);

        if ((!nodeStatus || nodeStatus->getState() == NodeStatus::UP) && networkConfigurator)
            configureNode();
    }
}

bool IPv4NodeConfigurator::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (stage == NodeStartOperation::STAGE_LINK_LAYER)
            prepareNode();
        else if (stage == NodeStartOperation::STAGE_NETWORK_LAYER && networkConfigurator)
            configureNode();
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation))
        /*nothing to do*/;
    else if (dynamic_cast<NodeCrashOperation *>(operation))
        /*nothing to do*/;
    else
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

void IPv4NodeConfigurator::prepareNode()
{
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++)
        prepareInterface(interfaceTable->getInterface(i));
}

void IPv4NodeConfigurator::prepareInterface(InterfaceEntry *interfaceEntry)
{
    ASSERT(!interfaceEntry->ipv4Data());
    IPv4InterfaceData *interfaceData = new IPv4InterfaceData();
    interfaceEntry->setIPv4Data(interfaceData);
    if (interfaceEntry->isLoopback())
    {
        // we may reconfigure later it to be the routerId
        interfaceData->setIPAddress(IPv4Address::LOOPBACK_ADDRESS);
        interfaceData->setNetmask(IPv4Address::LOOPBACK_NETMASK);
        interfaceData->setMetric(1);
    }
    else
    {
        // metric: some hints: OSPF cost (2e9/bps value), MS KB article Q299540, ...
        interfaceData->setMetric((int)ceil(2e9/interfaceEntry->getDatarate())); // use OSPF cost as default
        if (interfaceEntry->isMulticast())
        {
            interfaceData->joinMulticastGroup(IPv4Address::ALL_HOSTS_MCAST);
            if (routingTable->isForwardingEnabled())
                interfaceData->joinMulticastGroup(IPv4Address::ALL_ROUTERS_MCAST);
        }
    }
}

void IPv4NodeConfigurator::configureNode()
{
    ASSERT(networkConfigurator);
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++)
        networkConfigurator->configureInterface(interfaceTable->getInterface(i));
    if (par("configureRoutingTable").boolValue())
        networkConfigurator->configureRoutingTable(routingTable);
}
