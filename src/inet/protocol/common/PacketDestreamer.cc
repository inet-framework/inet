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
#include "inet/protocol/common/PacketDestreamer.h"

namespace inet {

Define_Module(PacketDestreamer);

void PacketDestreamer::initialize(int stage)
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

void PacketDestreamer::handleMessage(cMessage *message)
{
    auto packet = check_and_cast<Packet *>(message);
    pushPacket(packet, packet->getArrivalGate());
}

bool PacketDestreamer::canPushSomePacket(cGate *gate) const
{
    return !isStreaming() && consumer->canPushSomePacket(outputGate->getPathEndGate());
}

bool PacketDestreamer::canPushPacket(Packet *packet, cGate *gate) const
{
    return !isStreaming() && consumer->canPushPacket(packet, outputGate->getPathEndGate());
}

void PacketDestreamer::pushPacketStart(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacketStart");
    take(packet);
    streamedPacket = packet;
    EV_INFO << "Starting destreaming packet " << streamedPacket->getName() << "." << std::endl;
}

void PacketDestreamer::pushPacketEnd(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacketEnd");
    take(packet);
    delete streamedPacket;
    streamedPacket = packet;
    auto packetLength = streamedPacket->getTotalLength();
    EV_INFO << "Ending destreaming packet " << streamedPacket->getName() << "." << std::endl;
    pushOrSendPacket(streamedPacket, outputGate, consumer);
    streamedPacket = nullptr;
    numProcessedPackets++;
    processedTotalLength += packetLength;
    updateDisplayString();
}

void PacketDestreamer::pushPacketProgress(Packet *packet, cGate *gate, b position, b extraProcessableLength)
{
    Enter_Method("pushPacketProgress");
    take(packet);
    delete streamedPacket;
    streamedPacket = packet;
    EV_INFO << "Progressing destreaming packet " << streamedPacket->getName() << "." << std::endl;
}

b PacketDestreamer::getPushPacketProcessedLength(Packet *packet, cGate *gate)
{
    return packet->getTotalLength();
}

void PacketDestreamer::handleCanPushPacket(cGate *gate)
{
    Enter_Method("handleCanPushPacket");
    if (producer != nullptr)
        producer->handleCanPushPacket(inputGate->getPathStartGate());
}

void PacketDestreamer::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
    if (producer != nullptr)
        producer->handlePushPacketProcessed(packet, inputGate->getPathStartGate(), successful);
}

bool PacketDestreamer::canPullSomePacket(cGate *gate) const
{
    return !isStreaming() && provider->canPullSomePacket(inputGate->getPathStartGate());
}

Packet *PacketDestreamer::canPullPacket(cGate *gate) const
{
    return isStreaming() ? nullptr : provider->canPullPacket(inputGate->getPathStartGate());
}

Packet *PacketDestreamer::pullPacket(cGate *gate)
{
    Enter_Method("pullPacket");
    ASSERT(!isStreaming());
    auto packet = provider->pullPacketStart(inputGate->getPathStartGate(), datarate);
    EV_INFO << "Starting destreaming packet " << packet->getName() << "." << std::endl;
    take(packet);
    streamedPacket = packet;
    packet = provider->pullPacketEnd(inputGate->getPathStartGate(), datarate);
    EV_INFO << "Ending destreaming packet " << packet->getName() << "." << std::endl;
    take(packet);
    delete streamedPacket;
    streamedPacket = packet;
    numProcessedPackets++;
    processedTotalLength += packet->getTotalLength();
    animateSend(packet, outputGate);
    updateDisplayString();
    streamedPacket = nullptr;
    return packet;
}

void PacketDestreamer::handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePullPacketConfirmation");
    if (collector != nullptr)
        collector->handlePullPacketProcessed(packet, gate, successful);
}

void PacketDestreamer::handleCanPullPacket(cGate *gate)
{
    Enter_Method("handleCanPullPacket");
    if (collector != nullptr)
        collector->handleCanPullPacket(outputGate->getPathEndGate());
}

} // namespace inet

