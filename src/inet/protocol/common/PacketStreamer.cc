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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "inet/common/ModuleAccess.h"
#include "inet/protocol/common/PacketStreamer.h"

namespace inet {

Define_Module(PacketStreamer);

PacketStreamer::~PacketStreamer()
{
    delete streamedPacket;
    cancelAndDelete(endStreamingTimer);
}

void PacketStreamer::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        datarate = bps(par("datarate"));
        inputGate = gate("in");
        outputGate = gate("out");
        producer = findConnectedModule<IActivePacketSource>(inputGate);
        provider = findConnectedModule<IPassivePacketSource>(inputGate);
        consumer = findConnectedModule<IPassivePacketSink>(outputGate);
        collector = findConnectedModule<IActivePacketSink>(outputGate);
        endStreamingTimer = new cMessage("EndStreamingTimer");
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        checkPacketOperationSupport(outputGate);
    }
}

void PacketStreamer::handleMessage(cMessage *message)
{
    if (message == endStreamingTimer)
        endStreaming();
    else {
        auto packet = check_and_cast<Packet *>(message);
        pushPacket(packet, packet->getArrivalGate());
    }
}

void PacketStreamer::endStreaming()
{
    auto packetLength = streamedPacket->getTotalLength();
    EV_INFO << "Ending streaming" << EV_FIELD(packet, *streamedPacket) << EV_ENDL;
    pushOrSendPacketEnd(streamedPacket, outputGate, consumer);
    streamDatarate = bps(NaN);
    streamedPacket = nullptr;
    numProcessedPackets++;
    processedTotalLength += packetLength;
    updateDisplayString();
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
    streamDatarate = datarate;
    streamedPacket = packet->dup();
    streamedPacket->setOrigPacketId(packet->getId());
    EV_INFO << "Starting streaming" << EV_FIELD(packet) << EV_ENDL;
    pushOrSendPacketStart(packet, outputGate, consumer, streamDatarate);
    if (std::isnan(streamDatarate.get()))
        endStreaming();
    else
        scheduleAfter(s(streamedPacket->getTotalLength() / streamDatarate).get(), endStreamingTimer);
}

void PacketStreamer::handleCanPushPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (producer != nullptr)
        producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
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
    auto packet = provider->pullPacket(inputGate->getPathStartGate());
    streamDatarate = datarate;
    streamedPacket = packet->dup();
    streamedPacket->setOrigPacketId(packet->getId());
    EV_INFO << "Starting streaming" << EV_FIELD(packet, *streamedPacket) << EV_ENDL;
    updateDisplayString();
    return packet;
}

Packet *PacketStreamer::pullPacketEnd(cGate *gate)
{
    Enter_Method("pullPacketEnd");
    EV_INFO << "Ending streaming" << EV_FIELD(packet, *streamedPacket) << EV_ENDL;
    auto packet = streamedPacket;
    streamDatarate = bps(NaN);
    streamedPacket = nullptr;
    numProcessedPackets++;
    processedTotalLength += packet->getTotalLength();
    updateDisplayString();
    return packet;
}

Packet *PacketStreamer::pullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    Enter_Method("pullPacketProgress");
    streamDatarate = datarate;
    EV_INFO << "Progressing streaming" << EV_FIELD(packet, *streamedPacket) << EV_ENDL;
    updateDisplayString();
    return streamedPacket->dup();
}

void PacketStreamer::handleCanPullPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    if (collector != nullptr && !isStreaming())
        collector->handleCanPullPacketChanged(outputGate->getPathEndGate());
}

void PacketStreamer::handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePullPacketConfirmation");
    if (collector != nullptr)
        collector->handlePullPacketProcessed(packet, gate, successful);
}

} // namespace inet

