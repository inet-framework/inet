//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/source/ActivePacketSource.h"

namespace inet {
namespace queueing {

Define_Module(ActivePacketSource);

void ActivePacketSource::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        initialProductionOffset = par("initialProductionOffset");
        productionIntervalParameter = &par("productionInterval");
        productionTimer = new ClockEvent("ProductionTimer");
        scheduleForAbsoluteTime = par("scheduleForAbsoluteTime");
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(outputGate);
        if (!productionTimer->isScheduled())
            scheduleProductionTimerAndProducePacket();
    }
}

void ActivePacketSource::handleMessage(cMessage *message)
{
    if (message == productionTimer) {
        if (consumer == nullptr || consumer->canPushSomePacket(outputGate->getPathEndGate())) {
            scheduleProductionTimer(productionIntervalParameter->doubleValue());
            producePacket();
        }
    }
    else
        throw cRuntimeError("Unknown message");
}

void ActivePacketSource::handleParameterChange(const char *name)
{
    if (!strcmp(name, "initialProductionOffset"))
        initialProductionOffset = par("initialProductionOffset");
}

void ActivePacketSource::scheduleProductionTimer(clocktime_t delay)
{
    if (scheduleForAbsoluteTime)
        scheduleClockEventAt(getClockTime() + delay, productionTimer);
    else
        scheduleClockEventAfter(delay, productionTimer);
}

void ActivePacketSource::scheduleProductionTimerAndProducePacket()
{
    if (!initialProductionOffsetScheduled && initialProductionOffset >= CLOCKTIME_ZERO) {
        scheduleProductionTimer(initialProductionOffset);
        initialProductionOffsetScheduled = true;
    }
    else if (consumer == nullptr || consumer->canPushSomePacket(outputGate->getPathEndGate())) {
        scheduleProductionTimer(productionIntervalParameter->doubleValue());
        producePacket();
    }
}

void ActivePacketSource::producePacket()
{
    auto packet = createPacket();
    EV_INFO << "Producing packet" << EV_FIELD(packet) << EV_ENDL;
    emit(packetPushedSignal, packet);
    pushOrSendPacket(packet, outputGate, consumer);
    updateDisplayString();
}

void ActivePacketSource::handleCanPushPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (!productionTimer->isScheduled())
        scheduleProductionTimerAndProducePacket();
}

void ActivePacketSource::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
}

} // namespace queueing
} // namespace inet

