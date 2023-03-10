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

void PeriodicGate::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        initiallyOpen = par("initiallyOpen");
        initialOffset = par("offset");
        durations = check_and_cast<cValueArray *>(par("durations").objectValue());
        scheduleForAbsoluteTime = par("scheduleForAbsoluteTime");
        changeTimer = new ClockEvent("ChangeTimer");
        enableImplicitGuardBand = par("enableImplicitGuardBand");
        openSchedulingPriority = par("openSchedulingPriority");
        closeSchedulingPriority = par("closeSchedulingPriority");
    }
    else if (stage == INITSTAGE_QUEUEING)
        initializeGating();
}

void PeriodicGate::handleParameterChange(const char *name)
{
    // NOTE: parameters are set from the gate schedule configurator modules
    if (!strcmp(name, "offset"))
        initialOffset = par("offset");
    else if (!strcmp(name, "initiallyOpen"))
        initiallyOpen = par("initiallyOpen");
    else if (!strcmp(name, "durations"))
        durations = check_and_cast<cValueArray *>(par("durations").objectValue());
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

void PeriodicGate::initializeGating()
{
    if (durations->size() % 2 != 0)
        throw cRuntimeError("The duration parameter must contain an even number of values");
    clocktime_t totalDuration = 0;
    for (int i = 0; i < durations->size(); i++)
        totalDuration += durations->get(i).doubleValueInUnit("s");
    index = 0;
    isOpen_ = initiallyOpen;
    offset.setRaw(totalDuration != 0 ? initialOffset.raw() % totalDuration.raw() : 0);
    while (offset > 0) {
        clocktime_t duration = durations->get(index).doubleValueInUnit("s");
        if (offset > duration) {
            isOpen_ = !isOpen_;
            offset -= duration;
            index = (index + 1) % durations->size();
        }
        else
            break;
    }
    if (changeTimer->isScheduled())
        cancelClockEvent(changeTimer);
    if (durations->size() > 0)
        scheduleChangeTimer();
}

void PeriodicGate::scheduleChangeTimer()
{
    ASSERT(0 <= index && index < (int)durations->size());
    clocktime_t duration = durations->get(index).doubleValueInUnit("s");
    index = (index + 1) % durations->size();
    // skip trailing zero for wrap around, length is divisible by 2 so the expected state is the same
    if (durations->get(index).doubleValueInUnit("s") == 0) {
        index = (index + 1) % durations->size();
        duration += durations->get(index).doubleValueInUnit("s");
        index = (index + 1) % durations->size();
    }
    //std::cout << getFullPath() << " " << duration << std::endl;
    changeTimer->setSchedulingPriority(isOpen_ ? closeSchedulingPriority : openSchedulingPriority);
    if (scheduleForAbsoluteTime)
        scheduleClockEventAt(getClockTime() + duration - offset, changeTimer);
    else
        scheduleClockEventAfter(duration - offset, changeTimer);
    offset = 0;
}

void PeriodicGate::processChangeTimer()
{
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

} // namespace queueing
} // namespace inet

