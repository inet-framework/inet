//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
    auto packet = streamedPacket;
    auto packetLength = packet->getTotalLength();
    EV_INFO << "Ending streaming packet" << EV_FIELD(packet) << EV_ENDL;
    streamDatarate = bps(NaN);
    streamedPacket = nullptr;
    pushOrSendPacketEnd(packet, outputGate, consumer, packet->getId());
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
    if (!std::isnan(streamDatarate.get()))
        scheduleClockEventAfter(s(streamedPacket->getTotalLength() / streamDatarate).get(), endStreamingTimer);
    pushOrSendPacketStart(streamedPacket->dup(), outputGate, consumer, streamDatarate, streamedPacket->getId());
    if (std::isnan(streamDatarate.get()))
        endStreaming();
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
    animatePullPacketStart(packet, outputGate, streamDatarate, streamedPacket->getId());
    updateDisplayString();
    return packet;
}

Packet *PacketStreamer::pullPacketEnd(cGate *gate)
{
    Enter_Method("pullPacketEnd");
    EV_INFO << "Ending streaming packet" << EV_FIELD(packet, *streamedPacket) << EV_ENDL;
    auto packet = streamedPacket;
    handlePacketProcessed(packet);
    animatePullPacketEnd(packet, outputGate, streamedPacket->getId());
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
    animatePullPacketProgress(packet, outputGate, streamDatarate, position, extraProcessableLength, streamedPacket->getId());
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

