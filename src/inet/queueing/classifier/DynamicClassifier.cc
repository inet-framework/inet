//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/classifier/DynamicClassifier.h"

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/queueing/contract/IDynamicInputScheduler.h"

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
        spliceBranchSubmodules = par("spliceBranchSubmodules");
        if (!spliceBranchSubmodules && (submoduleName == nullptr || submoduleName[0] == '\0'))
            throw cRuntimeError("The submoduleName parameter must be set when spliceBranchSubmodules is false (it names the submodule vector that holds each branch)");
        if (!spliceBranchSubmodules && !getParentModule()->hasSubmoduleVector(submoduleName))
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
    cModule *parent = getParentModule();
    int index = gateSize("out");
    // grow this classifier's output gate vector
    setGateSize("out", index + 1);
    cGate *classifierOutputGate = gate("out", index);
    outputGates.push_back(classifierOutputGate);
    PassivePacketSinkRef consumer;
    consumer.reference(classifierOutputGate, false);
    consumers.push_back(consumer);
    ActivePacketSinkRef collector;
    collector.reference(classifierOutputGate, false);
    collectors.push_back(collector);
    // build the branch and collect the modules whose initialization is deferred until the
    // whole chain (including the aggregator connection) is wired
    std::vector<cModule *> modulesToInitialize;
    cGate *branchOutputGate = spliceBranchSubmodules ?
        spliceBranch(index, classifierOutputGate, modulesToInitialize) :
        createModuleBranch(index, classifierOutputGate, modulesToInitialize);
    // wire the branch output into the aggregator's next input gate
    cModule *aggregator = parent->getSubmodule(aggregatorSubmoduleName);
    aggregator->setGateSize("in", aggregator->gateSize("in") + 1);
    cGate *aggregatorInputGate = aggregator->gate("in", aggregator->gateSize("in") - 1);
    branchOutputGate->connectTo(aggregatorInputGate);
    for (auto module : modulesToInitialize)
        module->callInitialize();
    // if the aggregator is a pull scheduler with runtime-added inputs, register the new one
    if (auto scheduler = dynamic_cast<IDynamicInputScheduler *>(aggregator))
        scheduler->addInput(aggregatorInputGate);
    return index;
}

void DynamicClassifier::forwardMatchingParams(cModule *module)
{
    // copy the value of every parameter the created module shares (by name) with this
    // classifier's parent -- so e.g. a per-class compound picks up the enclosing queue's
    // configuration before it (and its own submodules) are finalized
    cModule *parent = getParentModule();
    for (int i = 0; i < module->getNumParams(); i++) {
        cPar& param = module->par(i);
        if (parent->hasPar(param.getName()))
            param = parent->par(param.getName());
    }
}

cGate *DynamicClassifier::createModuleBranch(int index, cGate *classifierOutputGate, std::vector<cModule *>& modulesToInitialize)
{
    cModule *parent = getParentModule();
    parent->setSubmoduleVectorSize(submoduleName, index + 1);
    cModule *module = moduleType->create(submoduleName, parent, index);
    classifierOutputGate->connectTo(module->gate("in"));
    module->finalizeParameters();
    module->buildInside();
    modulesToInitialize.push_back(module);
    return module->gate("out");
}

cGate *DynamicClassifier::spliceBranch(int index, cGate *classifierOutputGate, std::vector<cModule *>& modulesToInitialize)
{
    cModule *parent = getParentModule();
    // build the branch template compound
    cModule *compound = moduleType->create("splicetmp", parent);
    forwardMatchingParams(compound);
    compound->finalizeParameters();
    compound->buildInside();
    // walk the linear inner chain: compound.in -> chain[0].in, chain[i].out -> chain[i+1].in,
    // chain.back().out -> compound.out. Each chain module is expected to have a single in/out.
    std::vector<cModule *> chain;
    std::vector<std::string> names;
    for (cGate *g = compound->gate("in")->getNextGate(); g != nullptr && g->getOwnerModule() != compound; g = g->getOwnerModule()->gate("out")->getNextGate()) {
        chain.push_back(g->getOwnerModule());
        names.push_back(g->getOwnerModule()->getName());
    }
    if (chain.empty())
        throw cRuntimeError("spliceBranchSubmodules: compound '%s' has no linear in->out submodule chain", moduleType->getFullName());
    // disconnect all internal connections so the submodules can be reparented
    compound->gate("in")->disconnect();
    for (cModule *module : chain)
        if (module->gate("out")->isConnectedOutside())
            module->gate("out")->disconnect();
    // reparent each chain module into parent.<name>[index]: move the scalar under a temporary
    // unique name (so it does not clash with the target vector), then rename it into the slot
    for (size_t i = 0; i < chain.size(); i++) {
        cModule *module = chain[i];
        module->setName(("splicetmp_" + names[i]).c_str());
        parent->setSubmoduleVectorSize(names[i].c_str(), index + 1);
        module->changeParentTo(parent);
        module->setNameAndIndex(names[i].c_str(), index);
        modulesToInitialize.push_back(module);
    }
    compound->deleteModule();
    // rewire the chain at the branch level
    classifierOutputGate->connectTo(chain.front()->gate("in"));
    for (size_t i = 0; i + 1 < chain.size(); i++)
        chain[i]->gate("out")->connectTo(chain[i + 1]->gate("in"));
    return chain.back()->gate("out");
}

} // namespace queueing
} // namespace inet
