//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/protocol/classifier/DynamicClassifier.h"

namespace inet {

Define_Module(DynamicClassifier);

void DynamicClassifier::initialize(int stage)
{
    PacketClassifier::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        submoduleName = par("submoduleName");
        moduleType = cModuleType::get(par("moduleType"));
    }
}

int DynamicClassifier::classifyPacket(Packet *packet)
{
    int index = PacketClassifier::classifyPacket(packet);
    auto it = classIndexToGateItMap.find(index);
    if (it == classIndexToGateItMap.end()) {
        auto parentModule = getParentModule();
        int submoduleIndex = gateSize("out");
        auto module = moduleType->create(submoduleName, parentModule, submoduleIndex + 1, submoduleIndex);
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

} // namespace inet

