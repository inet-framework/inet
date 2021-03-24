//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/protocolelement/common/PacketStreamer.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

Define_Module(PacketStreamer);

PacketStreamer::~PacketStreamer()
{
    delete streamedPacket;
    cancelAndDeleteClockEvent(endStreamingTimer);
}

void PacketStreamer::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        datarate = bps(par("datarate"));
        inputGate = gate("in");
        outputGate = gate("out");
        producer = findConnectedModule<IActivePacketSource>(inputGate);
        provider = findConnectedModule<IPassivePacketSource>(inputGate);
        consumer = findConnectedModule<IPassivePacketSink>(outputGate);
        collector = findConnectedModule<IActivePacketSink>(outputGate);
        endStreamingTimer = new ClockEvent("EndStreamingTimer");
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
    EV_INFO << "Ending streaming packet" << EV_FIELD(packet, *streamedPacket) << EV_ENDL;
    pushOrSendPacketEnd(streamedPacket, outputGate, consumer, streamedPacket->getId());
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
    streamedPacket = packet;
    EV_INFO << "Starting streaming packet" << EV_FIELD(packet) << EV_ENDL;
    pushOrSendPacketStart(streamedPacket->dup(), outputGate, consumer, streamDatarate, streamedPacket->getId());
    if (std::isnan(streamDatarate.get()))
        endStreaming();
    else
        scheduleClockEventAfter(s(streamedPacket->getTotalLength() / streamDatarate).get(), endStreamingTimer);
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
    streamDatarate = datarate;
    streamedPacket = provider->pullPacket(inputGate->getPathStartGate());
    auto packet = streamedPacket->dup();
    EV_INFO << "Starting streaming packet" << EV_FIELD(packet) << EV_ENDL;
    animateSendPacketStart(packet, outputGate, streamDatarate, SendOptions().transmissionId(streamedPacket->getId()));
    updateDisplayString();
    return packet;
}

Packet *PacketStreamer::pullPacketEnd(cGate *gate)
{
    Enter_Method("pullPacketEnd");
    EV_INFO << "Ending streaming packet" << EV_FIELD(packet, *streamedPacket) << EV_ENDL;
    auto packet = streamedPacket;
    handlePacketProcessed(packet);
    animateSendPacketEnd(packet, outputGate, SendOptions().transmissionId(streamedPacket->getId()));
    streamDatarate = bps(NaN);
    streamedPacket = nullptr;
    updateDisplayString();
    return packet;
}

Packet *PacketStreamer::pullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    Enter_Method("pullPacketProgress");
    streamDatarate = datarate;
    EV_INFO << "Progressing streaming" << EV_FIELD(packet, *streamedPacket) << EV_ENDL;
    auto packet = streamedPacket->dup();
    animateSendPacketProgress(packet, outputGate, streamDatarate, position, extraProcessableLength, SendOptions().transmissionId(streamedPacket->getId()));
    updateDisplayString();
    return packet;
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

