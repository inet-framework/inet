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
        if (!getParentModule()->hasSubmoduleVector(submoduleName))
             throw cRuntimeError("The submodule vector '%s' missing from %s", submoduleName, getParentModule()->getFullPath().c_str());
     }
}

int DynamicClassifier::classifyPacket(Packet *packet)
{
    int index = PacketClassifier::classifyPacket(packet);
    auto it = classIndexToGateItMap.find(index);
    if (it == classIndexToGateItMap.end()) {
        auto parentModule = getParentModule();
        int submoduleIndex = gateSize("out");
        int origVectorSize = parentModule->getSubmoduleVectorSize(submoduleName);
        parentModule->setSubmoduleVectorSize(submoduleName, std::max(origVectorSize, submoduleIndex + 1));
        auto module = moduleType->create(submoduleName, parentModule, submoduleIndex);
        auto moduleInputGate = module->gate("in");
        auto moduleOutputGate = module->gate("out");
        auto multiplexer = parentModule->getSubmodule("multiplexer");
        multiplexer->setGateSize("in", multiplexer->gateSize("in") + 1);
        auto multiplexerInputGate = multiplexer->gate("in", multiplexer->gateSize("in") - 1);
        setGateSize("out", submoduleIndex + 1);
        auto classifierOutputGate = gate("out", gateSize("out") - 1);
        classifierOutputGate->connectTo(moduleInputGate);
        outputGates.push_back(classifierOutputGate);
        auto consumer = findConnectedModule<IPassivePacketSink>(classifierOutputGate);
        consumers.push_back(consumer);
        moduleOutputGate->connectTo(multiplexerInputGate);
        module->finalizeParameters();
        module->buildInside();
        module->callInitialize();
        classIndexToGateItMap[index] = submoduleIndex;
        return submoduleIndex;
    }
    else
        return it->second;
}

} // namespace queueing
} // namespace inet

