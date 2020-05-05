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

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/queueing/common/PacketDemultiplexer.h"

namespace inet {
namespace queueing {

Define_Module(PacketDemultiplexer);

void PacketDemultiplexer::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        for (int i = 0; i < gateSize("out"); i++) {
            auto outputGate = gate("out", i);
            auto output = getConnectedModule<IActivePacketSink>(outputGate);
            outputGates.push_back(outputGate);
            collectors.push_back(output);
        }
        provider = findConnectedModule<IPassivePacketSource>(inputGate);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        for (auto outputGate : outputGates)
            checkPacketOperationSupport(outputGate);
    }
}

Packet *PacketDemultiplexer::pullPacket(cGate *gate)
{
    Enter_Method("pullPacket");
    auto packet = provider->pullPacket(inputGate->getPathStartGate());
    take(packet);
    EV_INFO << "Forwarding pulled packet " << packet->getName() << "." << endl;
    animateSendPacket(packet, gate);
    numProcessedPackets++;
    processedTotalLength += packet->getDataLength();
    updateDisplayString();
    return packet;
}

void PacketDemultiplexer::handleCanPullPacket(cGate *gate)
{
    Enter_Method("handleCanPullPacket");
    for (int i = 0; i < (int)outputGates.size(); i++)
        // NOTE: notifying a listener may prevent others from pulling
        if (collectors[i] != nullptr && provider->canPullSomePacket(inputGate->getPathStartGate()))
            collectors[i]->handleCanPullPacket(outputGates[i]->getPathEndGate());
}

void PacketDemultiplexer::handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePullPacketProcessed");
    throw cRuntimeError("Invalid operation");
}

} // namespace queueing
} // namespace inet

