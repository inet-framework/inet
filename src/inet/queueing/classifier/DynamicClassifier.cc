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
    if (classIndexToGateItMap.find(index) == classIndexToGateItMap.end())
        classIndexToGateItMap[index] = createBranch();
}

int DynamicClassifier::createBranch()
{
    auto parentModule = getParentModule();
    int index = gateSize("out");
    setGateSize("out", index + 1);
    auto classifierOutputGate = gate("out", index);
    // the branch module is built but not initialized yet: its initialization is deferred until
    // the whole path, including the aggregator connection, is wired
    auto module = createBranchModule(index, classifierOutputGate);
    // An aggregator that has to take notice of a runtime-added input (a pull scheduler,
    // for example) learns about it from the model change notification of the connection,
    // so nothing here needs to know what kind of aggregator it is.
    auto aggregator = parentModule->getSubmodule(aggregatorSubmoduleName);
    aggregator->setGateSize("in", aggregator->gateSize("in") + 1);
    auto aggregatorInputGate = aggregator->gate("in", aggregator->gateSize("in") - 1);
    module->gate("out")->connectTo(aggregatorInputGate);
    // the sink references resolve the far end of the path eagerly, so they can only be taken
    // now that the whole branch, up to and including the aggregator, is connected
    outputGates.push_back(classifierOutputGate);
    PassivePacketSinkRef consumer;
    consumer.reference(classifierOutputGate, false);
    consumers.push_back(consumer);
    ActivePacketSinkRef collector;
    collector.reference(classifierOutputGate, false);
    collectors.push_back(collector);
    module->callInitialize();
    // the branch is a complete path only now, so this is the earliest point where the packet
    // operations of its modules can be checked, as the base class checks the gates wired in NED
    checkPacketOperationSupport(classifierOutputGate);
    return index;
}

cModule *DynamicClassifier::createBranchModule(int index, cGate *classifierOutputGate)
{
    auto parentModule = getParentModule();
    // the vector is only ever extended: it may have been declared larger in NED, and shrinking
    // one that still holds submodules is an error
    parentModule->setSubmoduleVectorSize(submoduleName, std::max(parentModule->getSubmoduleVectorSize(submoduleName), index + 1));
    // the branch is created with its final name and index, so that its parameters, its display
    // string and its result recording are all resolved for the module path it keeps
    auto module = moduleType->create(submoduleName, parentModule, index);
    classifierOutputGate->connectTo(module->gate("in"));
    module->finalizeParameters();
    module->buildInside();
    return module;
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

bool DynamicClassifier::canPushSomePacket(const cGate *gate) const
{
    // a packet of a class without a branch gets a new branch, which takes it
    return true;
}

void DynamicClassifier::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    createBranchIfNeeded(packet);
    PacketClassifier::pushPacket(packet, gate);
}

} // namespace queueing
} // namespace inet

