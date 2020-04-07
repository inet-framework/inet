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
#include "inet/queueing/base/PacketClassifierBase.h"

namespace inet {
namespace queueing {

void PacketClassifierBase::initialize(int stage)
{
    PassivePacketSinkBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        producer = findConnectedModule<IActivePacketSource>(inputGate);
        for (int i = 0; i < gateSize("out"); i++) {
            auto outputGate = gate("out", i);
            auto consumer = findConnectedModule<IPassivePacketSink>(outputGate);
            outputGates.push_back(outputGate);
            consumers.push_back(consumer);
        }
    }
    else if (stage == INITSTAGE_QUEUEING) {
        for (int i = 0; i < gateSize("out"); i++)
            checkPacketPushingSupport(outputGates[i]);
        checkPacketPushingSupport(inputGate);
    }
}

bool PacketClassifierBase::canPushSomePacket(cGate *gate) const
{
    for (int i = 0; i < (int)outputGates.size(); i++)
        if (!consumers[i]->canPushSomePacket(outputGates[i]->getPathEndGate()))
            return false;
    return true;
}

void PacketClassifierBase::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    emit(packetPushedSignal, packet);
    EV_INFO << "Classifying packet " << packet->getName() << ".\n";
    int index = classifyPacket(packet);
    if (index < 0 || static_cast<unsigned int>(index) >= outputGates.size())
        throw cRuntimeError("Classified packet to invalid output gate: %d", index);
    processedTotalLength += packet->getDataLength();
    pushOrSendPacket(packet, outputGates[index], consumers[index]);
    numProcessedPackets++;
    updateDisplayString();
}

void PacketClassifierBase::handleCanPushPacket(cGate *gate)
{
    Enter_Method("handleCanPushPacket");
    if (producer != nullptr)
        producer->handleCanPushPacket(inputGate);
}

} // namespace queueing
} // namespace inet

