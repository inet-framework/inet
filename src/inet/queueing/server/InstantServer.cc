//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/server/InstantServer.h"

#include "inet/common/Simsignals.h"

namespace inet {
namespace queueing {

Define_Module(InstantServer);

InstantServer::~InstantServer()
{
    cancelAndDelete(serveTimer);
}

void InstantServer::initialize(int stage)
{
    PacketServerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        int serveSchedulingPriority = par("serveSchedulingPriority");
        if (serveSchedulingPriority != -1) {
            serveTimer = new cMessage("ServeTimer");
            serveTimer->setSchedulingPriority(serveSchedulingPriority);
        }
    }
}

void InstantServer::handleMessage(cMessage *message)
{
    if (message == serveTimer)
        processPackets();
    else
        PacketServerBase::handleMessage(message);
}

bool InstantServer::canProcessPacket()
{
    if (provider.canPullSomePacket() && consumer.canPushSomePacket()) {
        auto packet = provider.canPullPacket();
        return packet != nullptr && consumer.canPushPacket(packet);
    }
    else
        return false;
}

void InstantServer::processPacket()
{
    auto packet = provider.pullPacket();
    take(packet);
    emit(packetPulledSignal, packet);
    std::string packetName = packet->getName();
    auto packetLength = packet->getDataLength();
    EV_INFO << "Processing packet started" << EV_FIELD(packet) << EV_ENDL;
    emit(packetPushedSignal, packet);
    pushOrSendPacket(packet, outputGate, consumer);
    processedTotalLength += packetLength;
    numProcessedPackets++;
    EV_INFO << "Processing packet ended" << EV_ENDL;
}

void InstantServer::handleCanPushPacketChanged(const cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (serveTimer)
        rescheduleAt(simTime(), serveTimer);
    else
        processPackets();
}

void InstantServer::handleCanPullPacketChanged(const cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    if (serveTimer)
        rescheduleAt(simTime(), serveTimer);
    else
        processPackets();
}

void InstantServer::processPackets()
{
    if (!isProcessing) {
        isProcessing = true;
        while (canProcessPacket())
            processPacket();
        isProcessing = false;
    }
}

} // namespace queueing
} // namespace inet

