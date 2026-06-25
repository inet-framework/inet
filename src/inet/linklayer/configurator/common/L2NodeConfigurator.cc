//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/configurator/common/L2NodeConfigurator.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"

#ifdef INET_WITH_MRP
#include "inet/linklayer/mrp/MrpInterfaceData.h"
#endif

namespace inet {

Define_Module(L2NodeConfigurator);

L2NodeConfigurator::L2NodeConfigurator()
{
}

void L2NodeConfigurator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        cModule *host = getContainingNode(this);
        nodeStatus = dynamic_cast<NodeStatus *>(host->getSubmodule("status"));
        interfaceTable.reference(this, "interfaceTableModule", true);
        networkConfigurator.reference(this, "l2ConfiguratorModule", false);
        // The interfaceCreatedSignal is used to add protocol data to interfaces as
        // they are created (see receiveSignal()); subscribe early enough to catch the
        // interfaces created during initialization.
        host->subscribe(interfaceCreatedSignal, this);
    }
    else if (stage == INITSTAGE_NETWORK_CONFIGURATION) {
        // Apply the L2 configuration to all interfaces. This must happen at this stage
        // (not from the interfaceCreatedSignal during interface creation), because the
        // L2NetworkConfigurator extracts the network topology here, once every interface
        // of every node has been registered. Triggering it earlier would extract an
        // incomplete topology and leave most ports unconfigured.
        if ((!nodeStatus || nodeStatus->getState() == NodeStatus::UP) && networkConfigurator)
            configureNode();
    }
}

bool L2NodeConfigurator::handleOperationStage(LifecycleOperation *operation, IDoneCallback *doneCallback)
{
    Enter_Method("handleOperationStage");
    if (dynamic_cast<ModuleStartOperation *>(operation)) {
        if (static_cast<ModuleStartOperation::Stage>(operation->getCurrentStage()) == ModuleStartOperation::STAGE_LINK_LAYER) {
            prepareNode();
            configureNode();
        }
    }
    else if (dynamic_cast<ModuleStopOperation *>(operation))
        /*nothing to do*/;
    else if (dynamic_cast<ModuleCrashOperation *>(operation))
        /*nothing to do*/;
    else
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

void L2NodeConfigurator::prepareNode()
{
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++)
        prepareInterface(interfaceTable->getInterface(i));
}

void L2NodeConfigurator::prepareInterface(NetworkInterface *networkInterface)
{
    if (par("hasStp"))
        networkInterface->addProtocolData<Ieee8021dInterfaceData>();
#ifdef INET_WITH_MRP
    if (par("hasMrp"))
        networkInterface->addProtocolData<MrpInterfaceData>();
#endif
}

void L2NodeConfigurator::configureNode()
{
//    std::cout << "configureNode(): " << interfaceTable->getNumInterfaces() << endl;
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++)
        networkConfigurator->configureInterface(interfaceTable->getInterface(i));
}

void L2NodeConfigurator::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    if (nodeStatus && nodeStatus->getState() != NodeStatus::UP)
        return;

    if (signalID == interfaceCreatedSignal) {
        NetworkInterface *ie = check_and_cast<NetworkInterface *>(obj);
        prepareInterface(ie);
        // Interfaces created during initialization are configured later, in one pass,
        // from initialize(INITSTAGE_NETWORK_CONFIGURATION) -- once the configurator has
        // extracted the complete topology. Only configure interfaces that are created
        // dynamically at runtime here (the topology is already available by then).
#if OMNETPP_BUILDNUM < 2001
        bool initializing = getSimulation()->getSimulationStage() == STAGE(INITIALIZE);
#else
        bool initializing = getSimulation()->getStage() == STAGE(INITIALIZE);
#endif
        if (networkConfigurator && !initializing)
            networkConfigurator->configureInterface(ie);
    }
}

} // namespace inet

