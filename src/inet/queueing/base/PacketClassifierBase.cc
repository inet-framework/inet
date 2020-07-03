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
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        reverseOrder = par("reverseOrder");
        inputGate = gate("in");
        producer = findConnectedModule<IActivePacketSource>(inputGate);
        provider = findConnectedModule<IPassivePacketSource>(inputGate);
        for (int i = 0; i < gateSize("out"); i++) {
            auto outputGate = gate("out", i);
            outputGates.push_back(outputGate);
            auto consumer = findConnectedModule<IPassivePacketSink>(outputGate);
            consumers.push_back(consumer);
            auto collector = findConnectedModule<IActivePacketSink>(outputGate);
            collectors.push_back(collector);
        }
    }
    else if (stage == INITSTAGE_QUEUEING) {
        for (int i = 0; i < gateSize("out"); i++)
            checkPacketOperationSupport(outputGates[i]);
        checkPacketOperationSupport(inputGate);
    }
}

void PacketClassifierBase::handleMessage(cMessage *message)
{
    auto packet = check_and_cast<Packet *>(message);
    pushPacket(packet, packet->getArrivalGate());
}

int PacketClassifierBase::callClassifyPacket(Packet *packet) const
{
    // KLUDGE:
    int index = const_cast<PacketClassifierBase *>(this)->classifyPacket(packet);
    if (index < 0 || static_cast<unsigned int>(index) >= outputGates.size())
        throw cRuntimeError("Classified packet to invalid output gate: %d", index);
    return index;
}

void PacketClassifierBase::checkPacketStreaming(Packet *packet)
{
    if (inProgressStreamId != -1 && (packet == nullptr || packet->getTreeId() != inProgressStreamId))
        throw cRuntimeError("Another packet streaming operation is already in progress");
}

void PacketClassifierBase::startPacketStreaming(Packet *packet)
{
    EV_INFO << "Classifying packet " << packet->getName() << ".\n";
    inProgressStreamId = packet->getTreeId();
    inProgressGateIndex = callClassifyPacket(packet);
}

void PacketClassifierBase::endPacketStreaming(Packet *packet)
{
    emit(packetPushedSignal, packet);
    handlePacketProcessed(packet);
    inProgressStreamId = -1;
    inProgressGateIndex = -1;
}

bool PacketClassifierBase::canPushSomePacket(cGate *gate) const
{
    for (int i = 0; i < (int)outputGates.size(); i++)
        if (!consumers[i]->canPushSomePacket(outputGates[i]->getPathEndGate()))
            return false;
    return true;
}

bool PacketClassifierBase::canPushPacket(Packet *packet, cGate *gate) const
{
    return canPushSomePacket(gate);
}

void PacketClassifierBase::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    checkPacketStreaming(nullptr);
    EV_INFO << "Classifying packet " << packet->getName() << ".\n";
    int index = callClassifyPacket(packet);
    handlePacketProcessed(packet);
    emit(packetPushedSignal, packet);
    pushOrSendPacket(packet, outputGates[index], consumers[index]);
    updateDisplayString();
}

void PacketClassifierBase::pushPacketStart(Packet *packet, cGate *gate, bps datarate)
{
    Enter_Method("pushPacketStart");
    take(packet);
    checkPacketStreaming(packet);
    startPacketStreaming(packet);
    pushOrSendPacketStart(packet, outputGates[inProgressGateIndex], consumers[inProgressGateIndex], datarate);
    updateDisplayString();
}

void PacketClassifierBase::pushPacketEnd(Packet *packet, cGate *gate, bps datarate)
{
    Enter_Method("pushPacketEnd");
    take(packet);
    if (!isStreamingPacket())
        startPacketStreaming(packet);
    else
        checkPacketStreaming(packet);
    auto outputGate = outputGates[inProgressGateIndex];
    auto consumer = consumers[inProgressGateIndex];
    endPacketStreaming(packet);
    pushOrSendPacketEnd(packet, outputGate, consumer, datarate);
    updateDisplayString();
}

void PacketClassifierBase::pushPacketProgress(Packet *packet,  cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    Enter_Method("pushPacketProgress");
    take(packet);
    if (!isStreamingPacket())
        startPacketStreaming(packet);
    else
        checkPacketStreaming(packet);
    auto outputGate = outputGates[inProgressGateIndex];
    auto consumer = consumers[inProgressGateIndex];
    if (packet->getTotalLength() == position + extraProcessableLength)
        endPacketStreaming(packet);
    pushOrSendPacketProgress(packet, outputGate, consumer, datarate, position, extraProcessableLength);
    updateDisplayString();
}

b PacketClassifierBase::getPushPacketProcessedLength(Packet *packet, cGate *gate)
{
    checkPacketStreaming(packet);
    auto outputGate = outputGates[inProgressGateIndex];
    auto consumer = consumers[inProgressGateIndex];
    return consumer->getPushPacketProcessedLength(packet, outputGate->getPathEndGate());
}

void PacketClassifierBase::handleCanPushPacket(cGate *gate)
{
    Enter_Method("handleCanPushPacket");
    if (producer != nullptr)
        producer->handleCanPushPacket(inputGate->getPathStartGate());
}

void PacketClassifierBase::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    producer->handlePushPacketProcessed(packet, inputGate->getPathStartGate(), successful);
}

bool PacketClassifierBase::canPullSomePacket(cGate *gate) const
{
    return canPullPacket(gate) != nullptr;
}

Packet *PacketClassifierBase::canPullPacket(cGate *gate) const
{
    auto packet = provider->canPullPacket(inputGate->getPathStartGate());
    if (packet == nullptr)
        return nullptr;
    else {
        int index = callClassifyPacket(packet);
        return index == gate->getIndex() ? packet : nullptr;
    }
}

Packet *PacketClassifierBase::pullPacket(cGate *gate)
{
    auto packet = provider->pullPacket(inputGate->getPathStartGate());
    int index = callClassifyPacket(packet);
    if (index != gate->getIndex())
        throw cRuntimeError("Classified packet to wrong output gate");
    return packet;
}

void PacketClassifierBase::handleCanPullPacket(cGate *gate)
{
    auto packet = provider->canPullPacket(inputGate->getPathStartGate());
    if (packet != nullptr) {
        int index = callClassifyPacket(packet);
        collectors[index]->handleCanPullPacket(outputGates[index]->getPathEndGate());
    }
}

} // namespace queueing
} // namespace inet

