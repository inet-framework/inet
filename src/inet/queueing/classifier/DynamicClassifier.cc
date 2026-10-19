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
        // the output gates are created in the order in which the classes appear
        if (reverseOrder)
            throw cRuntimeError("The reverseOrder parameter is not supported");
        submoduleName = par("submoduleName");
        moduleType = cModuleType::get(par("moduleType"));
        aggregatorSubmoduleName = par("aggregatorSubmoduleName");
        if (!getParentModule()->hasSubmoduleVector(submoduleName))
            throw cRuntimeError("The submodule vector '%s' is missing from %s", submoduleName, getParentModule()->getFullPath().c_str());
        if (getParentModule()->getSubmodule(aggregatorSubmoduleName) == nullptr)
            throw cRuntimeError("The aggregator submodule '%s' is missing from %s", aggregatorSubmoduleName, getParentModule()->getFullPath().c_str());
    }
}

int DynamicClassifier::getClassIndex(Packet *packet) const
{
    return packetClassifierFunction->classifyPacket(packet);
}

int DynamicClassifier::classifyPacket(Packet *packet) const
{
    auto it = classIndexToGateItMap.find(getClassIndex(packet));
    return it != classIndexToGateItMap.end() ? it->second : -1;
}

void DynamicClassifier::createBranchIfNeeded(Packet *packet)
{
    int index = getClassIndex(packet);
    auto it = classIndexToGateItMap.find(index);
    if (it == classIndexToGateItMap.end()) {
        auto parentModule = getParentModule();
        int submoduleIndex = gateSize("out");
        int origVectorSize = parentModule->getSubmoduleVectorSize(submoduleName);
        parentModule->setSubmoduleVectorSize(submoduleName, std::max(origVectorSize, submoduleIndex + 1));
        auto module = moduleType->create(submoduleName, parentModule, submoduleIndex);
        auto moduleInputGate = module->gate("in");
        auto moduleOutputGate = module->gate("out");
        // An aggregator that has to take notice of a runtime-added input (a pull scheduler,
        // for example) learns about it from the model change notification of the connection,
        // so nothing here needs to know what kind of aggregator it is.
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
        classIndexToGateItMap[index] = submoduleIndex;
    }
}

void DynamicClassifier::startPacketStreaming(Packet *packet)
{
    createBranchIfNeeded(packet);
    PacketClassifier::startPacketStreaming(packet);
}

bool DynamicClassifier::canPushPacket(Packet *packet, const cGate *gate) const
{
    // KLUDGE: the answer must hold for this packet, and it depends on the branch
    // of its class, so this query creates the branch if it does not exist yet
    const_cast<DynamicClassifier *>(this)->createBranchIfNeeded(packet);
    return PacketClassifier::canPushPacket(packet, gate);
}

void DynamicClassifier::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    createBranchIfNeeded(packet);
    PacketClassifier::pushPacket(packet, gate);
}

} // namespace queueing
} // namespace inet

