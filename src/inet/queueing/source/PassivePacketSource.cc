//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/source/PassivePacketSource.h"

#include "inet/common/Simsignals.h"

namespace inet {
namespace queueing {

Define_Module(PassivePacketSource);

void PassivePacketSource::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        providingIntervalParameter = &par("providingInterval");
        providingTimer = new ClockEvent("ProvidingTimer");
        scheduleForAbsoluteTime = par("scheduleForAbsoluteTime");
        WATCH_PTR(nextPacket);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        if (collector != nullptr)
            collector->handleCanPullPacketChanged(outputGate->getPathEndGate());
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

void PassivePacketSource::scheduleProvidingTimer()
{
    clocktime_t interval = providingIntervalParameter->doubleValue();
    if (interval != 0 || providingTimer->getArrivalModule() == nullptr) {
        if (scheduleForAbsoluteTime)
            scheduleClockEventAt(getClockTime() + interval, providingTimer);
        else
            scheduleClockEventAfter(interval, providingTimer);
    }
}

Packet *PassivePacketSource::canPullPacket(cGate *gate) const
{
    Enter_Method("canPullPacket");
    if (providingTimer->isScheduled())
        return nullptr;
    else {
        if (nextPacket == nullptr)
            // KLUDGE
            nextPacket = const_cast<PassivePacketSource *>(this)->createPacket();
        return nextPacket;
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
        scheduleProvidingTimer();
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

