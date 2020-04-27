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
#include "inet/protocol/common/PacketStreamer.h"

namespace inet {

Define_Module(PacketStreamer);

void PacketStreamer::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        outputGate = gate("out");
        producer = findConnectedModule<IActivePacketSource>(inputGate);
        provider = findConnectedModule<IPassivePacketSource>(inputGate);
        consumer = findConnectedModule<IPassivePacketSink>(outputGate);
        collector = findConnectedModule<IActivePacketSink>(outputGate);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        checkPacketOperationSupport(outputGate);
    }
}

void PacketStreamer::handleMessage(cMessage *message)
{
    auto packet = check_and_cast<Packet *>(message);
    pushPacket(packet, packet->getArrivalGate());
}

bool PacketStreamer::canPushSomePacket(cGate *gate) const
{
    return !isStreaming() && consumer->canPushSomePacket(outputGate->getPathEndGate());
}

bool PacketStreamer::canPushPacket(Packet *packet, cGate *gate) const
{
    return !isStreaming() && consumer->canPushPacket(packet, outputGate->getPathEndGate());
}

void PacketStreamer::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    ASSERT(!isStreaming());
    take(packet);
    streamedPacket = packet;
    auto packetLength = packet->getTotalLength();
    EV_INFO << "Starting streaming packet " << packet->getName() << "." << std::endl;
    pushOrSendPacketStart(packet->dup(), outputGate, consumer, datarate);
    EV_INFO << "Ending streaming packet " << packet->getName() << "." << std::endl;
    pushOrSendPacketEnd(streamedPacket, outputGate, consumer, datarate);
    streamedPacket = nullptr;
    numProcessedPackets++;
    processedTotalLength += packetLength;
    updateDisplayString();
}

void PacketStreamer::handleCanPushPacket(cGate *gate)
{
    Enter_Method("handleCanPushPacket");
    if (producer != nullptr)
        producer->handleCanPushPacket(inputGate->getPathStartGate());
}

void PacketStreamer::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
    if (producer != nullptr)
        producer->handlePushPacketProcessed(packet, inputGate->getPathStartGate(), successful);
}

bool PacketStreamer::canPullSomePacket(cGate *gate) const
{
    return !isStreaming() && provider->canPullSomePacket(inputGate->getPathStartGate());
}

Packet *PacketStreamer::canPullPacket(cGate *gate) const
{
    return isStreaming() ? nullptr : provider->canPullPacket(inputGate->getPathStartGate());
}

Packet *PacketStreamer::pullPacketStart(cGate *gate, bps datarate)
{
    Enter_Method("pullPacketStart");
    streamedPacket = provider->pullPacket(inputGate->getPathStartGate());
    EV_INFO << "Starting streaming packet " << streamedPacket->getName() << "." << std::endl;
    updateDisplayString();
    return streamedPacket->dup();
}

Packet *PacketStreamer::pullPacketEnd(cGate *gate, bps datarate)
{
    Enter_Method("pullPacketEnd");
    EV_INFO << "Ending streaming packet " << streamedPacket->getName() << "." << std::endl;
    auto packet = streamedPacket;
    streamedPacket = nullptr;
    numProcessedPackets++;
    processedTotalLength += packet->getTotalLength();
    updateDisplayString();
    return packet;
}

Packet *PacketStreamer::pullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    Enter_Method("pullPacketProgress");
    EV_INFO << "Progressing streaming packet " << streamedPacket->getName() << "." << std::endl;
    updateDisplayString();
    return streamedPacket->dup();
}

b PacketStreamer::getPullPacketProcessedLength(Packet *packet, cGate *gate)
{
    return streamedPacket->getTotalLength();
}

void PacketStreamer::handleCanPullPacket(cGate *gate)
{
    Enter_Method("handleCanPullPacket");
    if (collector != nullptr)
        collector->handleCanPullPacket(outputGate->getPathEndGate());
}

void PacketStreamer::handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePullPacketConfirmation");
    if (collector != nullptr)
        collector->handlePullPacketProcessed(packet, gate, successful);
}

} // namespace inet

