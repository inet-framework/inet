//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/server/PacketServer.h"

#include "inet/common/PacketEventTag.h"
#include "inet/common/Simsignals.h"
#include "inet/common/TimeTag.h"

namespace inet {
namespace queueing {

Define_Module(PacketServer);

PacketServer::~PacketServer()
{
    cancelAndDelete(serveTimer);
    cancelAndDeleteClockEvent(processingTimer);
    delete packet;
}

void PacketServer::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        int serveSchedulingPriority = par("serveSchedulingPriority");
        if (serveSchedulingPriority != -1) {
            serveTimer = new cMessage("ServeTimer");
            serveTimer->setSchedulingPriority(serveSchedulingPriority);
        }
        processingTimer = new ClockEvent("ProcessingTimer");
    }
}

void PacketServer::handleMessage(cMessage *message)
{
    if (message == serveTimer) {
        startProcessingPacket();
        scheduleProcessingTimer();
    }
    else if (message == processingTimer) {
        endProcessingPacket();
        if (canStartProcessingPacket()) {
            startProcessingPacket();
            scheduleProcessingTimer();
        }
        updateDisplayString();
    }
    else
        throw cRuntimeError("Unknown message");
}

void PacketServer::scheduleProcessingTimer()
{
    clocktime_t processingTime = par("processingTime");
    auto processingBitrate = bps(par("processingBitrate"));
    processingTime += (packet->getDataLength() / processingBitrate).get<s>();
    scheduleClockEventAfter(processingTime, processingTimer);
}

bool PacketServer::canStartProcessingPacket()
{
    auto packet = provider.canPullPacket();
    return packet != nullptr && consumer.canPushPacket(packet);
}

void PacketServer::startProcessingPacket()
{
    packet = provider.pullPacket();
    take(packet);
    emit(packetPulledSignal, packet);
    EV_INFO << "Processing packet started" << EV_FIELD(packet) << EV_ENDL;
}

void PacketServer::endProcessingPacket()
{
    EV_INFO << "Processing packet ended" << EV_FIELD(packet) << EV_ENDL;
    simtime_t packetProcessingTime = simTime() - processingTimer->getSendingTime();
    simtime_t bitProcessingTime = packetProcessingTime / packet->getBitLength();
    insertPacketEvent(this, packet, PEK_PROCESSED, bitProcessingTime, 0);
    increaseTimeTag<ProcessingTimeTag>(packet, bitProcessingTime, packetProcessingTime);
    processedTotalLength += packet->getDataLength();
    emit(packetPushedSignal, packet);
    pushOrSendPacket(packet, outputGate, consumer);
    numProcessedPackets++;
    packet = nullptr;
}

cGate *PacketServer::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

void PacketServer::handleCanPushPacketChanged(const cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (!processingTimer->isScheduled() && canStartProcessingPacket()) {
        if (serveTimer)
            rescheduleAt(simTime(), serveTimer);
        else {
            startProcessingPacket();
            scheduleProcessingTimer();
        }
    }
}

void PacketServer::handleCanPullPacketChanged(const cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    if (!processingTimer->isScheduled() && canStartProcessingPacket()) {
        if (serveTimer)
            rescheduleAt(simTime(), serveTimer);
        else {
            startProcessingPacket();
            scheduleProcessingTimer();
        }
    }
}

std::string PacketServer::resolveDirective(char directive) const
{
    switch (directive) {
        case 's':
            return processingTimer->isScheduled() ? "processing" : "";
        default:
            return PacketServerBase::resolveDirective(directive);
    }
}

} // namespace queueing
} // namespace inet

