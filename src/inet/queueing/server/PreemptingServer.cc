//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/server/PreemptingServer.h"

namespace inet {
namespace queueing {

Define_Module(PreemptingServer);

void PreemptingServer::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        datarate = bps(par("datarate"));
        timer = new ClockEvent("Timer");
    }
}

void PreemptingServer::handleMessage(cMessage *message)
{
    if (message == timer)
        endStreaming();
    else
        PacketServerBase::handleMessage(message);
}

bool PreemptingServer::canStartStreaming() const
{
    return provider.canPullSomePacket() && consumer.canPushSomePacket();
}

void PreemptingServer::startStreaming()
{
    auto packet = provider.pullPacketStart(datarate);
    take(packet);
    EV_INFO << "Starting streaming packet" << EV_FIELD(packet) << EV_ENDL;
    streamedPacket = packet;
    pushOrSendPacketStart(streamedPacket->dup(), outputGate, consumer, datarate, packet->getTransmissionId());
    scheduleClockEventAfter(s(streamedPacket->getDataLength() / datarate).get(), timer);
    handlePacketProcessed(streamedPacket);
    updateDisplayString();
}

void PreemptingServer::endStreaming()
{
    auto packet = provider.pullPacketEnd();
    take(packet);
    EV_INFO << "Ending streaming packet" << EV_FIELD(packet) << EV_ENDL;
    delete streamedPacket;
    streamedPacket = packet;
    EV_INFO << "Ending streaming packet" << EV_FIELD(packet, *streamedPacket) << EV_ENDL;
    pushOrSendPacketEnd(streamedPacket, outputGate, consumer, packet->getTransmissionId());
    streamedPacket = nullptr;
    updateDisplayString();
}

void PreemptingServer::handleCanPushPacketChanged(const cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    EV_DEBUG << "Checking if packet streaming should be started" << EV_ENDL;
    if (!isStreaming() && canStartStreaming())
        startStreaming();
}

void PreemptingServer::handleCanPullPacketChanged(const cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    EV_DEBUG << "Checking if packet streaming should be started" << EV_ENDL;
    if (!isStreaming() && canStartStreaming())
        startStreaming();
}

void PreemptingServer::handlePushPacketProcessed(Packet *packet, const cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
    if (isStreaming()) {
        delete streamedPacket;
        streamedPacket = provider.pullPacketEnd();
        take(streamedPacket);
        EV_INFO << "Ending streaming packet" << EV_FIELD(packet, *streamedPacket) << EV_ENDL;
        delete streamedPacket;
        streamedPacket = nullptr;
    }
}

void PreemptingServer::pushPacketEnd(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacketEnd");
    ASSERT(isStreaming());
    EV_INFO << "Ending packet streaming, requested by packet producer" << EV_FIELD(packet) << EV_ENDL;
    take(packet);
    consumer.pushPacketEnd(packet);
    cancelEvent(timer);
    delete streamedPacket;
    streamedPacket = nullptr;
    updateDisplayString();
}

} // namespace queueing
} // namespace inet

