//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/sink/PassivePacketSink.h"

namespace inet {
namespace queueing {

Define_Module(PassivePacketSink);

void PassivePacketSink::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        initialConsumptionOffset = par("initialConsumptionOffset");
        consumptionIntervalParameter = &par("consumptionInterval");
        consumptionTimer = new ClockEvent("ConsumptionTimer");
        scheduleForAbsoluteTime = par("scheduleForAbsoluteTime");
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        if (producer != nullptr)
            producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
        if (!consumptionTimer->isScheduled() && initialConsumptionOffset != 0)
            scheduleConsumptionTimer(initialConsumptionOffset);
    }
}

void PassivePacketSink::handleMessage(cMessage *message)
{
    if (message == consumptionTimer) {
        if (producer != nullptr)
            producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
    }
    else
        PassivePacketSinkBase::handleMessage(message);
}

bool PassivePacketSink::canPushSomePacket(cGate *gate) const
{
    return getClockTime() >= initialConsumptionOffset && !consumptionTimer->isScheduled() && PassivePacketSinkBase::canPushSomePacket(gate);
}

bool PassivePacketSink::canPushPacket(Packet *packet, cGate *gate) const
{
    return canPushSomePacket(gate);
}

void PassivePacketSink::scheduleConsumptionTimer(clocktime_t delay)
{
    if (delay != 0 || consumptionTimer->getArrivalModule() == nullptr) {
        if (scheduleForAbsoluteTime)
            scheduleClockEventAt(getClockTime() + delay, consumptionTimer);
        else
            scheduleClockEventAfter(delay, consumptionTimer);
    }
}

void PassivePacketSink::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    if (consumptionTimer->isScheduled() && consumptionTimer->getArrivalTime() > simTime())
        throw cRuntimeError("Another packet is already being consumed");
    else {
        emit(packetPushedSignal, packet);
        consumePacket(packet);
        scheduleConsumptionTimer(consumptionIntervalParameter->doubleValue());
    }
}

void PassivePacketSink::consumePacket(Packet *packet)
{
    EV_INFO << "Consuming packet" << EV_FIELD(packet) << EV_ENDL;
    numProcessedPackets++;
    processedTotalLength += packet->getDataLength();
    updateDisplayString();
    dropPacket(packet, OTHER_PACKET_DROP);
}

} // namespace queueing
} // namespace inet

