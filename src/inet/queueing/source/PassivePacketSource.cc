//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/source/PassivePacketSource.h"

namespace inet {
namespace queueing {

Define_Module(PassivePacketSource);

void PassivePacketSource::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        initialProvidingOffset = par("initialProvidingOffset");
        providingIntervalParameter = &par("providingInterval");
        providingTimer = new ClockEvent("ProvidingTimer");
        scheduleForAbsoluteTime = par("scheduleForAbsoluteTime");
        WATCH_PTR(nextPacket);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(outputGate);
        if (collector != nullptr)
            collector->handleCanPullPacketChanged(outputGate->getPathEndGate());
        if (!providingTimer->isScheduled() && initialProvidingOffset != 0)
            scheduleProvidingTimer(initialProvidingOffset);
    }
}

void PassivePacketSource::handleMessage(cMessage *message)
{
    if (message == providingTimer) {
        if (collector != nullptr)
            collector->handleCanPullPacketChanged(outputGate->getPathEndGate());
    }
    else
        throw cRuntimeError("Unknown message");
}

bool PassivePacketSource::canPullSomePacket(cGate *gate) const
{
    return getClockTime() >= initialProvidingOffset && !providingTimer->isScheduled();
}

Packet *PassivePacketSource::canPullPacket(cGate *gate) const
{
    Enter_Method("canPullPacket");
    if (getClockTime() < initialProvidingOffset || providingTimer->isScheduled())
        return nullptr;
    else {
        if (nextPacket == nullptr)
            // KLUDGE
            nextPacket = const_cast<PassivePacketSource *>(this)->createPacket();
        return nextPacket;
    }
}

void PassivePacketSource::scheduleProvidingTimer(clocktime_t delay)
{
    if (delay != 0 || providingTimer->getArrivalModule() == nullptr) {
        if (scheduleForAbsoluteTime)
            scheduleClockEventAt(getClockTime() + delay, providingTimer);
        else
            scheduleClockEventAfter(delay, providingTimer);
    }
}

Packet *PassivePacketSource::pullPacket(cGate *gate)
{
    Enter_Method("pullPacket");
    if (providingTimer->isScheduled() && providingTimer->getArrivalTime() > simTime())
        throw cRuntimeError("Another packet is already being provided");
    else {
        auto packet = providePacket(gate);
        animatePullPacket(packet, outputGate);
        emit(packetPulledSignal, packet);
        scheduleProvidingTimer(providingIntervalParameter->doubleValue());
        updateDisplayString();
        return packet;
    }
}

Packet *PassivePacketSource::providePacket(cGate *gate)
{
    Packet *packet;
    if (nextPacket == nullptr)
        packet = createPacket();
    else {
        packet = nextPacket;
        nextPacket = nullptr;
    }
    EV_INFO << "Providing packet" << EV_FIELD(packet) << EV_ENDL;
    updateDisplayString();
    return packet;
}

} // namespace queueing
} // namespace inet

