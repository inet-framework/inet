//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/gate/PeriodicGate.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace queueing {

Define_Module(PeriodicGate);

simsignal_t PeriodicGate::guardBandStateChangedSignal = registerSignal("guardBandStateChanged");

void PeriodicGate::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        initiallyOpen = par("initiallyOpen");
        initialOffset = par("offset");
        readDurationsPar();
        scheduleForAbsoluteTime = par("scheduleForAbsoluteTime");
        changeTimer = new ClockEvent("ChangeTimer");
        enableImplicitGuardBand = par("enableImplicitGuardBand");
        openSchedulingPriority = par("openSchedulingPriority");
        closeSchedulingPriority = par("closeSchedulingPriority");
        WATCH(isInGuardBand_);
    }
    else if (stage == INITSTAGE_QUEUEING)
        initializeGating();
    else if (stage == INITSTAGE_LAST)
        emit(guardBandStateChangedSignal, isInGuardBand_);
}

void PeriodicGate::finish()
{
    emit(guardBandStateChangedSignal, isInGuardBand_);
}

void PeriodicGate::handleParameterChange(const char *name)
{
    // NOTE: parameters are set from the gate schedule configurator modules
    if (!strcmp(name, "offset"))
        initialOffset = par("offset");
    else if (!strcmp(name, "initiallyOpen"))
        initiallyOpen = par("initiallyOpen");
    else if (!strcmp(name, "durations"))
        readDurationsPar();
    initializeGating();
}

void PeriodicGate::handleMessage(cMessage *message)
{
    if (message == changeTimer) {
        scheduleChangeTimer();
        processChangeTimer();
    }
    else
        throw cRuntimeError("Unknown message");
}

void PeriodicGate::readDurationsPar()
{
    auto durationsPar = check_and_cast<cValueArray *>(par("durations").objectValue());
    size_t size = durationsPar->size();
    if (size % 2 != 0)
        throw cRuntimeError("The duration parameter must contain an even number of values");
    totalDuration = CLOCKTIME_ZERO;
    durations.resize(size);
    for (int i = 0; i < size; i++) {
        clocktime_t duration = durationsPar->get(i).doubleValueInUnit("s");
        if (duration <= CLOCKTIME_ZERO)
            throw cRuntimeError("Unaccepted duration value (%s) at position %d", durationsPar->get(i).str().c_str(), i);
        durations[i] = duration;
        totalDuration += duration;
    }
}

void PeriodicGate::open()
{
    PacketGateBase::open();
    updateIsInGuardBand();
}

void PeriodicGate::close()
{
    PacketGateBase::close();
    updateIsInGuardBand();
}

void PeriodicGate::initializeGating()
{
    index = 0;
    isOpen_ = initiallyOpen;
    offset.setRaw(totalDuration != 0 ? initialOffset.raw() % totalDuration.raw() : 0);
    while (offset > CLOCKTIME_ZERO) {
        clocktime_t duration = durations[index];
        if (offset >= duration) {
            isOpen_ = !isOpen_;
            offset -= duration;
            index = (index + 1) % durations.size();
        }
        else
            break;
    }
    if (changeTimer->isScheduled())
        cancelClockEvent(changeTimer);
    if (durations.size() > 0)
        scheduleChangeTimer();
}

void PeriodicGate::scheduleChangeTimer()
{
    ASSERT(0 <= index && (size_t)index < durations.size());
    clocktime_t duration = durations[index];
    index = (index + 1) % durations.size();

    changeTimer->setSchedulingPriority(isOpen_ ? closeSchedulingPriority : openSchedulingPriority);
    if (scheduleForAbsoluteTime)
        scheduleClockEventAt(getClockTime() + duration - offset, changeTimer);
    else
        scheduleClockEventAfter(duration - offset, changeTimer);
    offset = 0;
}

void PeriodicGate::processChangeTimer()
{
    EV_INFO << "Processing change timer" << EV_ENDL;
    if (isOpen_)
        close();
    else
        open();
}

bool PeriodicGate::canPacketFlowThrough(Packet *packet) const
{
    ASSERT(isOpen_);
    if (std::isnan(bitrate.get()))
        return PacketGateBase::canPacketFlowThrough(packet);
    else if (packet == nullptr)
        return false;
    else {
        if (enableImplicitGuardBand) {
            clocktime_t flowEndTime = getClockTime() + s((packet->getDataLength() + extraLength) / bitrate).get() + SIMTIME_AS_CLOCKTIME(extraDuration);
            return !changeTimer->isScheduled() || flowEndTime <= getArrivalClockTime(changeTimer);
        }
        else
            return PacketGateBase::canPacketFlowThrough(packet);
    }
}

void PeriodicGate::updateIsInGuardBand()
{
    bool newIsInGuardBand = false;
    if (isOpen_) {
        auto packet = provider != nullptr ? provider->canPullPacket(inputGate->getPathStartGate()) : nullptr;
        newIsInGuardBand = packet != nullptr && !canPacketFlowThrough(packet);
    }
    if (isInGuardBand_ != newIsInGuardBand) {
        isInGuardBand_ = newIsInGuardBand;
        EV_INFO << "Changing guard band state" << EV_FIELD(isInGuardBand_) << EV_ENDL;
        emit(guardBandStateChangedSignal, isInGuardBand_);
    }
}

void PeriodicGate::handleCanPushPacketChanged(cGate *gate)
{
    PacketGateBase::handleCanPushPacketChanged(gate);
    updateIsInGuardBand();
}

void PeriodicGate::handleCanPullPacketChanged(cGate *gate)
{
    PacketGateBase::handleCanPullPacketChanged(gate);
    updateIsInGuardBand();
}

} // namespace queueing
} // namespace inet

