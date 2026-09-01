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
        if (reverseOrder)
            throw cRuntimeError("The reverseOrder parameter is not supported: branches are created in the order the classes of the packets first appear");
    }
}

int DynamicClassifier::getClassIndex(Packet *packet) const
{
    // The class of the packet, taken as the classifier function returns it, and not mapped
    // through getOutputGateIndex(): that mapping depends on the number of output gates, which
    // grows with each branch, so the same class would end up under a different key over time,
    // and get a second branch.
    return packetClassifierFunction->classifyPacket(packet);
}

int DynamicClassifier::classifyPacket(Packet *packet)
{
    // a plain lookup, free of side effects: the branch of a class that is seen for the first
    // time is created before the base class classifies, on the delivery path only
    auto it = classIndexToBranchIndex.find(getClassIndex(packet));
    return it != classIndexToBranchIndex.end() ? it->second : -1;
}

void DynamicClassifier::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    // usually a no-op: a source that asked canPushPacket() first already had it created
    createBranchIfAbsent(packet);
    PacketClassifier::pushPacket(packet, gate);
}

void DynamicClassifier::startPacketStreaming(Packet *packet)
{
    // the one place all three streaming push operations classify through
    createBranchIfAbsent(packet);
    PacketClassifier::startPacketStreaming(packet);
}

bool DynamicClassifier::canPushPacket(Packet *packet, const cGate *gate) const
{
    // The answer has to hold for this very packet, because a source that asks may push exactly
    // it next. Answering yes for a class that has no branch would promise on behalf of a branch
    // that does not exist and may refuse its first packet, so the branch is created here and
    // asked. This is the one query whose answer decides the fate of a packet, and it therefore
    // has to build the thing that decides it; classification itself stays a pure lookup.
    // KLUDGE the query is const, the model change it needs is not
    const_cast<DynamicClassifier *>(this)->createBranchIfAbsent(packet);
    return consumers[classIndexToBranchIndex.at(getClassIndex(packet))].canPushPacket(packet);
}

void DynamicClassifier::createBranchIfAbsent(Packet *packet)
{
    int classIndex = getClassIndex(packet);
    if (classIndexToBranchIndex.find(classIndex) == classIndexToBranchIndex.end())
        classIndexToBranchIndex[classIndex] = createBranch();
}

int DynamicClassifier::createBranch()
{
    cModule *parent = getParentModule();
    int index = gateSize("out");
    // grow this classifier's output gate vector
    setGateSize("out", index + 1);
    cGate *classifierOutputGate = gate("out", index);
    // the branch module is built but not initialized yet: its initialization is deferred until
    // the whole chain, including the aggregator connection, is wired
    cModule *branch = createBranchModule(index, classifierOutputGate);
    // Wire the branch output into the aggregator's next input gate. An aggregator that has
    // to take notice of a runtime-added input (a pull scheduler, for example) learns about
    // it from the model change notification of this very connection, so nothing here needs
    // to know what kind of aggregator it is.
    cModule *aggregator = parent->getSubmodule(aggregatorSubmoduleName);
    aggregator->setGateSize("in", aggregator->gateSize("in") + 1);
    cGate *aggregatorInputGate = aggregator->gate("in", aggregator->gateSize("in") - 1);
    branch->gate("out")->connectTo(aggregatorInputGate);
    // the sink references resolve the far end of the path eagerly, so they can only be taken
    // now that the whole branch, up to and including the aggregator, is connected
    outputGates.push_back(classifierOutputGate);
    PassivePacketSinkRef consumer;
    consumer.reference(classifierOutputGate, false);
    consumers.push_back(consumer);
    ActivePacketSinkRef collector;
    collector.reference(classifierOutputGate, false);
    collectors.push_back(collector);
    branch->callInitialize();
    // the branch is a complete path only now, so this is the earliest point where the packet
    // operations of its modules can be checked, the way the base class checks the gates that
    // are wired in NED
    checkPacketOperationSupport(classifierOutputGate);
    return index;
}

cModule *DynamicClassifier::createBranchModule(int index, cGate *classifierOutputGate)
{
    cModule *parent = getParentModule();
    // the vector is only ever extended: it may have been declared larger in NED, and shrinking
    // one that still holds submodules is an error
    parent->setSubmoduleVectorSize(submoduleName, std::max(parent->getSubmoduleVectorSize(submoduleName), index + 1));
    // the branch is created with its final name and index, so that its parameters (from the
    // enclosing NED declaration and from the ini file), its display string and its result
    // recording are all resolved for the module path it keeps
    cModule *module = moduleType->create(submoduleName, parent, index);
    classifierOutputGate->connectTo(module->gate("in"));
    module->finalizeParameters();
    module->buildInside();
    return module;
}

} // namespace queueing
} // namespace inet
