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
#include "inet/queueing/common/PacketMultiplexer.h"

namespace inet {
namespace queueing {

Define_Module(PacketMultiplexer);

void PacketMultiplexer::initialize(int stage)
{
    PassivePacketSinkBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        for (int i = 0; i < gateSize("in"); i++) {
            auto inputGate = gate("in", i);
            auto input = getConnectedModule<IActivePacketSource>(inputGate);
            inputGates.push_back(inputGate);
            producers.push_back(input);
        }
        outputGate = gate("out");
        consumer = findConnectedModule<IPassivePacketSink>(outputGate);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        for (auto inputGate : inputGates)
            checkPushPacketSupport(inputGate);
        checkPushPacketSupport(outputGate);
    }
}

void PacketMultiplexer::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    EV_INFO << "Forwarding pushed packet " << packet->getName() << "." << endl;
    processedTotalLength += packet->getDataLength();
    pushOrSendPacket(packet, outputGate, consumer);
    numProcessedPackets++;
    updateDisplayString();
}

void PacketMultiplexer::handleCanPushPacket(cGate *gate)
{
    Enter_Method("handleCanPushPacket");
    for (int i = 0; i < (int)inputGates.size(); i++)
        // NOTE: notifying a listener may prevent others from pushing
        if (consumer->canPushSomePacket(outputGate))
            producers[i]->handleCanPushPacket(inputGates[i]);
}

} // namespace queueing
} // namespace inet

