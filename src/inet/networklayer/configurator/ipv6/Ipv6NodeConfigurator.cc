//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/configurator/ipv6/Ipv6NodeConfigurator.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"

namespace inet {

Define_Module(Ipv6NodeConfigurator);

Ipv6NodeConfigurator::Ipv6NodeConfigurator()
{
}

void Ipv6NodeConfigurator::initialize(int stage)
{
    SimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        _configureRoutingTable = par("configureRoutingTable");
        cModule *node = getContainingNode(this);
        nodeStatus = dynamic_cast<NodeStatus *>(node->getSubmodule("status"));

        interfaceTable.reference(this, "interfaceTableModule", true);
        routingTable.reference(this, "routingTableModule", true);
        networkConfigurator.reference(this, "networkConfiguratorModule", false);
        if (networkConfigurator != nullptr)
            networkConfigurator->subscribe(Ipv6NetworkConfigurator::networkConfigurationChangedSignal, this);
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
            node->subscribe(interfaceCreatedSignal, this);
            node->subscribe(interfaceDeletedSignal, this);
            node->subscribe(interfaceStateChangedSignal, this);
        }
    }
}

bool Ipv6NodeConfigurator::handleOperationStage(LifecycleOperation *operation, IDoneCallback *doneCallback)
{
    Enter_Method("handleOperationStage");
    int stage = operation->getCurrentStage();
    if (dynamic_cast<ModuleStartOperation *>(operation)) {
        if (static_cast<ModuleStartOperation::Stage>(stage) == ModuleStartOperation::STAGE_LOCAL)
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

void Ipv6NodeConfigurator::prepareAllInterfaces()
{
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++)
        prepareInterface(interfaceTable->getInterface(i));
}

void Ipv6NodeConfigurator::prepareInterface(NetworkInterface *networkInterface)
{
    // Ipv6InterfaceData and link-local address are handled by Ipv6RoutingTable::configureInterfaceForIpv6()
    // Nothing extra needed here currently
}

void Ipv6NodeConfigurator::configureAllInterfaces()
{
    ASSERT(networkConfigurator);
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++)
        networkConfigurator->configureInterface(interfaceTable->getInterface(i));
}

void Ipv6NodeConfigurator::configureRoutingTable()
{
    ASSERT(networkConfigurator);
    if (_configureRoutingTable)
        networkConfigurator->configureRoutingTable(routingTable);
}

void Ipv6NodeConfigurator::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
#if OMNETPP_BUILDNUM < 2001
    if (getSimulation()->getSimulationStage() == STAGE(INITIALIZE))
#else
    if (getSimulation()->getStage() == STAGE(INITIALIZE))
#endif
        return; // ignore notifications during initialize

    Enter_Method("%s", cComponent::getSignalName(signalID));

    printSignalBanner(signalID, obj, details);

    if (signalID == interfaceCreatedSignal) {
        auto *entry = check_and_cast<NetworkInterface *>(obj);
        prepareInterface(entry);
    }
    else if (signalID == interfaceDeletedSignal) {
        // The RoutingTable deletes routing entries of interface
    }
    else if (signalID == interfaceStateChangedSignal) {
        const auto *networkInterfaceChangeDetails = check_and_cast<const NetworkInterfaceChangeDetails *>(obj);
        auto fieldId = networkInterfaceChangeDetails->getFieldId();
        if (fieldId == NetworkInterface::F_STATE || fieldId == NetworkInterface::F_CARRIER) {
            auto networkInterface = networkInterfaceChangeDetails->getNetworkInterface();
            if (networkConfigurator != nullptr) {
                if (networkInterface->isUp() && networkInterface->hasCarrier()) {
                    networkConfigurator->configureInterface(networkInterface);
                }
            }
        }
    }
    else if (signalID == Ipv6NetworkConfigurator::networkConfigurationChangedSignal) {
        if ((!nodeStatus || nodeStatus->getState() == NodeStatus::UP) && networkConfigurator) {
            configureAllInterfaces();
            configureRoutingTable();
        }
    }
}

} // namespace inet
