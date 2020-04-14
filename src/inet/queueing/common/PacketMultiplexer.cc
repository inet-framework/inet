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
            auto input = findConnectedModule<IActivePacketSource>(inputGate);
            inputGates.push_back(inputGate);
            producers.push_back(input);
        }
        outputGate = gate("out");
        consumer = findConnectedModule<IPassivePacketSink>(outputGate);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        for (int i = 0; i < (int)inputGates.size(); i++)
            checkPacketOperationSupport(inputGates[i]);
        checkPacketOperationSupport(outputGate);
    }
}

void PacketMultiplexer::checkPacketStreaming(Packet *packet)
{
    if (inProgressStreamId != -1 && (packet == nullptr || packet->getTreeId() != inProgressStreamId))
        throw cRuntimeError("Another packet streaming operation is already in progress");
}

void PacketMultiplexer::startPacketStreaming(Packet *packet)
{
    inProgressStreamId = packet->getTreeId();
}

void PacketMultiplexer::endPacketStreaming(Packet *packet)
{
    emit(packetPushedSignal, packet);
    handlePacketProcessed(packet);
    inProgressStreamId = -1;
}

void PacketMultiplexer::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    EV_INFO << "Forwarding pushed packet " << packet->getName() << "." << endl;
    handlePacketProcessed(packet);
    pushOrSendPacket(packet, outputGate, consumer);
    updateDisplayString();
}

void PacketMultiplexer::pushPacketStart(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacketStart");
    take(packet);
    EV_INFO << "Forwarding pushed packet " << packet->getName() << "." << endl;
    checkPacketStreaming(packet);
    startPacketStreaming(packet);
    pushOrSendPacketStart(packet, outputGate->getPathEndGate(), consumer);
    updateDisplayString();
}

void PacketMultiplexer::pushPacketEnd(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacketEnd");
    take(packet);
    EV_INFO << "Forwarding pushed packet " << packet->getName() << "." << endl;
    if (!isStreamingPacket())
        startPacketStreaming(packet);
    else
        checkPacketStreaming(packet);
    endPacketStreaming(packet);
    pushOrSendPacketEnd(packet, outputGate->getPathEndGate(), consumer);
    updateDisplayString();
}

void PacketMultiplexer::pushPacketProgress(Packet *packet, cGate *gate, b position, b extraProcessableLength)
{
    Enter_Method("pushPacketProgress");
    take(packet);
    EV_INFO << "Forwarding pushed packet " << packet->getName() << "." << endl;
    if (!isStreamingPacket())
        startPacketStreaming(packet);
    else
        checkPacketStreaming(packet);
    if (packet->getTotalLength() == position + extraProcessableLength)
        endPacketStreaming(packet);
    pushOrSendPacketProgress(packet, outputGate->getPathEndGate(), consumer, position, extraProcessableLength);
    updateDisplayString();
}

b PacketMultiplexer::getPushPacketProcessedLength(Packet *packet, cGate *gate)
{
    return consumer->getPushPacketProcessedLength(packet, outputGate->getPathEndGate());
}

void PacketMultiplexer::handleCanPushPacket(cGate *gate)
{
    Enter_Method("handleCanPushPacket");
    for (int i = 0; i < (int)inputGates.size(); i++)
        // NOTE: notifying a listener may prevent others from pushing
        if (producers[i] != nullptr && consumer->canPushSomePacket(outputGate))
            producers[i]->handleCanPushPacket(inputGates[i]->getPathStartGate());
}

void PacketMultiplexer::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
}

} // namespace queueing
} // namespace inet

