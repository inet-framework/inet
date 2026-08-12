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

int DynamicClassifier::getClassIndex(Packet *packet) const
{
    // the class of the packet, with no side effect -- unlike classifyPacket() below, which
    // creates the branch of a class that is seen for the first time. Note that the class index
    // is taken as it is, and not mapped through getOutputGateIndex(): that mapping depends on
    // the number of output gates, which grows with each branch, so the same class would end up
    // under a different key over time, and get a second branch.
    return packetClassifierFunction->classifyPacket(packet);
}

int DynamicClassifier::classifyPacket(Packet *packet)
{
    int index = getClassIndex(packet);
    auto it = classIndexToGateItMap.find(index);
    if (it != classIndexToGateItMap.end())
        return it->second;
    int branchIndex = createBranch();
    classIndexToGateItMap[index] = branchIndex;
    return branchIndex;
}

int DynamicClassifier::createBranch()
{
    cModule *parent = getParentModule();
    int index = gateSize("out");
    // grow this classifier's output gate vector
    setGateSize("out", index + 1);
    cGate *classifierOutputGate = gate("out", index);
    // build the branch and collect the modules whose initialization is deferred until the
    // whole chain (including the aggregator connection) is wired
    std::vector<cModule *> modulesToInitialize;
    cGate *branchOutputGate = createModuleBranch(index, classifierOutputGate, modulesToInitialize);
    // Wire the branch output into the aggregator's next input gate. An aggregator that has
    // to take notice of a runtime-added input (a pull scheduler, for example) learns about
    // it from the model change notification of this very connection, so nothing here needs
    // to know what kind of aggregator it is.
    cModule *aggregator = parent->getSubmodule(aggregatorSubmoduleName);
    aggregator->setGateSize("in", aggregator->gateSize("in") + 1);
    cGate *aggregatorInputGate = aggregator->gate("in", aggregator->gateSize("in") - 1);
    branchOutputGate->connectTo(aggregatorInputGate);
    // the sink references resolve the far end of the path eagerly, so they can only be taken
    // now that the whole branch, up to and including the aggregator, is connected
    outputGates.push_back(classifierOutputGate);
    PassivePacketSinkRef consumer;
    consumer.reference(classifierOutputGate, false);
    consumers.push_back(consumer);
    ActivePacketSinkRef collector;
    collector.reference(classifierOutputGate, false);
    collectors.push_back(collector);
    for (auto module : modulesToInitialize)
        module->callInitialize();
    return index;
}

cGate *DynamicClassifier::createModuleBranch(int index, cGate *classifierOutputGate, std::vector<cModule *>& modulesToInitialize)
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
    modulesToInitialize.push_back(module);
    return module->gate("out");
}

} // namespace queueing
} // namespace inet
