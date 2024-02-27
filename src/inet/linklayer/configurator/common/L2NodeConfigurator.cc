//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/configurator/L2NodeConfigurator.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"

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
        host->subscribe(interfaceCreatedSignal, this);
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
//    ASSERT(!networkInterface->getProtocolData<Ieee8021dInterfaceData>());
    networkInterface->addProtocolData<Ieee8021dInterfaceData>();
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
        if (networkConfigurator)
            networkConfigurator->configureInterface(ie);
    }
}

} // namespace inet

