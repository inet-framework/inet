//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/sink/ActivePacketSink.h"

namespace inet {
namespace queueing {

Define_Module(ActivePacketSink);

void ActivePacketSink::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        initialCollectionOffset = par("initialCollectionOffset");
        collectionIntervalParameter = &par("collectionInterval");
        collectionTimer = new ClockEvent("CollectionTimer");
        scheduleForAbsoluteTime = par("scheduleForAbsoluteTime");
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        if (!collectionTimer->isScheduled())
            scheduleCollectionTimerAndCollectPacket();
    }
}

void ActivePacketSink::handleMessage(cMessage *message)
{
    if (message == collectionTimer) {
        if (provider == nullptr || provider->canPullSomePacket(inputGate->getPathStartGate())) {
            scheduleCollectionTimer(collectionIntervalParameter->doubleValue());
            collectPacket();
        }
    }
    else
        throw cRuntimeError("Unknown message");
}

void ActivePacketSink::handleParameterChange(const char *name)
{
    if (!strcmp(name, "initialCollectionOffset"))
        initialCollectionOffset = par("initialCollectionOffset");
}

void ActivePacketSink::scheduleCollectionTimer(clocktime_t delay)
{
    if (scheduleForAbsoluteTime)
        scheduleClockEventAt(getClockTime() + delay, collectionTimer);
    else
        scheduleClockEventAfter(delay, collectionTimer);
}

void ActivePacketSink::scheduleCollectionTimerAndCollectPacket()
{
    if (!initialCollectionOffsetScheduled && initialCollectionOffset >= CLOCKTIME_ZERO) {
        scheduleCollectionTimer(initialCollectionOffset);
        initialCollectionOffsetScheduled = true;
    }
    else if (provider == nullptr || provider->canPullSomePacket(inputGate->getPathStartGate())) {
        scheduleCollectionTimer(collectionIntervalParameter->doubleValue());
        collectPacket();
    }
}

void ActivePacketSink::collectPacket()
{
    auto packet = provider->pullPacket(inputGate->getPathStartGate());
    take(packet);
    emit(packetPulledSignal, packet);
    EV_INFO << "Collecting packet" << EV_FIELD(packet) << EV_ENDL;
    numProcessedPackets++;
    processedTotalLength += packet->getDataLength();
    updateDisplayString();
    dropPacket(packet, OTHER_PACKET_DROP);
}

void ActivePacketSink::handleCanPullPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    if (!collectionTimer->isScheduled())
        scheduleCollectionTimerAndCollectPacket();
}

void ActivePacketSink::handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePullPacketProcessed");
}

} // namespace queueing
} // namespace inet

