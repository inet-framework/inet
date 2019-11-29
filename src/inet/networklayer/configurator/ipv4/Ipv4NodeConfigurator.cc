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

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/configurator/ipv4/Ipv4NodeConfigurator.h"

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
    else if (stage == INITSTAGE_NETWORK_CONFIGURATION) {
        if (!nodeStatus || nodeStatus->getState() == NodeStatus::UP)
            prepareAllInterfaces();
    }
    else if (stage == INITSTAGE_NETWORK_ADDRESS_ASSIGNMENT) {
        if ((!nodeStatus || nodeStatus->getState() == NodeStatus::UP) && networkConfigurator)
            configureAllInterfaces();
    }
    else if (stage == INITSTAGE_STATIC_ROUTING) {
        if ((!nodeStatus || nodeStatus->getState() == NodeStatus::UP) && networkConfigurator) {
            configureRoutingTable();
            cModule *node = getContainingNode(this);
            // get a pointer to the host module and IInterfaceTable
            node->subscribe(interfaceCreatedSignal, this);
            node->subscribe(interfaceDeletedSignal, this);
            node->subscribe(interfaceStateChangedSignal, this);
        }
    }
}

bool Ipv4NodeConfigurator::handleOperationStage(LifecycleOperation *operation, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    int stage = operation->getCurrentStage();
    if (dynamic_cast<ModuleStartOperation *>(operation)) {
        if (static_cast<ModuleStartOperation::Stage>(stage) == ModuleStartOperation::STAGE_LINK_LAYER)
            prepareAllInterfaces();
        else if (static_cast<ModuleStartOperation::Stage>(stage) == ModuleStartOperation::STAGE_NETWORK_LAYER) {
            if (networkConfigurator != nullptr) {
                configureAllInterfaces();
                configureRoutingTable();
            }
            cModule *node = getContainingNode(this);
            node->subscribe(interfaceCreatedSignal, this);
            node->subscribe(interfaceDeletedSignal, this);
            node->subscribe(interfaceStateChangedSignal, this);
        }
    }
    else if (dynamic_cast<ModuleStopOperation *>(operation)) {
        if (static_cast<ModuleStopOperation::Stage>(stage) == ModuleStopOperation::STAGE_LOCAL) {
            cModule *node = getContainingNode(this);
            node->unsubscribe(interfaceCreatedSignal, this);
            node->unsubscribe(interfaceDeletedSignal, this);
            node->unsubscribe(interfaceStateChangedSignal, this);
        }
    }
    else if (dynamic_cast<ModuleCrashOperation *>(operation)) {
        cModule *node = getContainingNode(this);
        node->unsubscribe(interfaceCreatedSignal, this);
        node->unsubscribe(interfaceDeletedSignal, this);
        node->unsubscribe(interfaceStateChangedSignal, this);
    }
    else
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

void Ipv4NodeConfigurator::prepareAllInterfaces()
{
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++)
        prepareInterface(interfaceTable->getInterface(i));
}

void Ipv4NodeConfigurator::prepareInterface(InterfaceEntry *interfaceEntry)
{
    // ASSERT(!interfaceEntry->getProtocolData<Ipv4InterfaceData>());
    Ipv4InterfaceData *interfaceData = interfaceEntry->addProtocolData<Ipv4InterfaceData>();
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

void Ipv4NodeConfigurator::configureAllInterfaces()
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

void Ipv4NodeConfigurator::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    if (getSimulation()->getContextType() == CTX_INITIALIZE)
        return; // ignore notifications during initialize

    Enter_Method_Silent();
    printSignalBanner(signalID, obj, details);

    if (signalID == interfaceCreatedSignal) {
        auto *entry = check_and_cast<InterfaceEntry *>(obj);
        prepareInterface(entry);
        // TODO
    }
    else if (signalID == interfaceDeletedSignal) {
        // The RoutingTable deletes routing entries of interface
    }
    else if (signalID == interfaceStateChangedSignal) {
        const auto *ieChangeDetails = check_and_cast<const InterfaceEntryChangeDetails *>(obj);
        auto fieldId = ieChangeDetails->getFieldId();
        if (fieldId == InterfaceEntry::F_STATE || fieldId == InterfaceEntry::F_CARRIER) {
            auto *entry = ieChangeDetails->getInterfaceEntry();
            if (entry->isUp() && networkConfigurator) {
                networkConfigurator->configureInterface(entry);
                if (par("configureRoutingTable"))
                    networkConfigurator->configureRoutingTable(routingTable, entry);
            }
            // otherwise the RoutingTable deletes routing entries of interface entry
        }
    }
}

} // namespace inet

