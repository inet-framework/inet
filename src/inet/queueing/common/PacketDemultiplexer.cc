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
        checkPopPacketSupport(inputGate);
        for (auto outputGate : outputGates)
            checkPopPacketSupport(outputGate);
    }
}

Packet *PacketDemultiplexer::popPacket(cGate *gate)
{
    Enter_Method("popPacket");
    auto packet = provider->popPacket(inputGate->getPathStartGate());
    EV_INFO << "Forwarding popped packet " << packet->getName() << "." << endl;
    animateSend(packet, gate);
    numProcessedPackets++;
    processedTotalLength += packet->getDataLength();
    updateDisplayString();
    return packet;
}

void PacketDemultiplexer::handleCanPopPacket(cGate *gate)
{
    Enter_Method("handleCanPopPacket");
    for (int i = 0; i < (int)outputGates.size(); i++)
        // NOTE: notifying a listener may prevent others from popping
        if (provider->canPopSomePacket(inputGate->getPathStartGate()))
            collectors[i]->handleCanPopPacket(outputGates[i]);
}

} // namespace queueing
} // namespace inet

