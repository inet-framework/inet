//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/classifier/DynamicClassifier.h"

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"

namespace inet {
namespace queueing {

Define_Module(DynamicClassifier);

void DynamicClassifier::initialize(int stage)
{
    PacketClassifier::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        submoduleName = par("submoduleName");
        moduleType = cModuleType::get(par("moduleType"));
        aggregatorSubmoduleName = par("aggregatorSubmoduleName");
        if (!getParentModule()->hasSubmoduleVector(submoduleName))
            throw cRuntimeError("The submodule vector '%s' is missing from %s", submoduleName, getParentModule()->getFullPath().c_str());
        if (getParentModule()->getSubmodule(aggregatorSubmoduleName) == nullptr)
            throw cRuntimeError("The aggregator submodule '%s' is missing from %s", aggregatorSubmoduleName, getParentModule()->getFullPath().c_str());
    }
}

int DynamicClassifier::classifyPacket(Packet *packet)
{
    int index = PacketClassifier::classifyPacket(packet);
    auto it = classIndexToGateItMap.find(index);
    if (it != classIndexToGateItMap.end())
        return it->second;
    int branchIndex = createBranch();
    classIndexToGateItMap[index] = branchIndex;
    return branchIndex;
}

int DynamicClassifier::createBranch()
{
    auto parentModule = getParentModule();
    int submoduleIndex = gateSize("out");
    int origVectorSize = parentModule->getSubmoduleVectorSize(submoduleName);
    parentModule->setSubmoduleVectorSize(submoduleName, std::max(origVectorSize, submoduleIndex + 1));
    auto module = moduleType->create(submoduleName, parentModule, submoduleIndex);
    auto moduleInputGate = module->gate("in");
    auto moduleOutputGate = module->gate("out");
    // Wire the branch output into the aggregator's next input gate. An aggregator that has
    // to take notice of a runtime-added input (a pull scheduler, for example) learns about
    // it from the model change notification of this very connection, so nothing here needs
    // to know what kind of aggregator it is.
    auto aggregator = parentModule->getSubmodule(aggregatorSubmoduleName);
    aggregator->setGateSize("in", aggregator->gateSize("in") + 1);
    auto aggregatorInputGate = aggregator->gate("in", aggregator->gateSize("in") - 1);
    setGateSize("out", submoduleIndex + 1);
    auto classifierOutputGate = gate("out", gateSize("out") - 1);
    classifierOutputGate->connectTo(moduleInputGate);
    outputGates.push_back(classifierOutputGate);
    PassivePacketSinkRef consumer;
    consumer.reference(classifierOutputGate, false);
    consumers.push_back(consumer);
    ActivePacketSinkRef collector;
    collector.reference(classifierOutputGate, false);
    collectors.push_back(collector);
    moduleOutputGate->connectTo(aggregatorInputGate);
    module->finalizeParameters();
    module->buildInside();
    module->callInitialize();
    return submoduleIndex;
}

} // namespace queueing
} // namespace inet
