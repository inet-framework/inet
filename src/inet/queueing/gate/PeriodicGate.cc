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
        isOpen_ = par("initiallyOpen");
        initialOffset = par("offset");
        durations = check_and_cast<cValueArray *>(par("durations").objectValue());
        scheduleForAbsoluteTime = par("scheduleForAbsoluteTime");
        changeTimer = new ClockEvent("ChangeTimer");
    }
    else if (stage == INITSTAGE_QUEUEING)
        initializeGating();
}

void PeriodicGate::handleParameterChange(const char *name)
{
    if (name != nullptr) {
        if (!strcmp(name, "offset"))
            initialOffset = par("offset");
        else if (!strcmp(name, "initiallyOpen"))
            isOpen_ = par("initiallyOpen");
        else if (!strcmp(name, "durations")) {
            durations = check_and_cast<cValueArray *>(par("durations").objectValue());
            initializeGating();
        }
    }
}

void PeriodicGate::handleMessage(cMessage *message)
{
    if (message == changeTimer) {
        processChangeTimer();
        scheduleChangeTimer();
    }
    else
        throw cRuntimeError("Unknown message");
}

void PeriodicGate::initializeGating()
{
    if (durations->size() % 2 != 0)
        throw cRuntimeError("The duration parameter must contain an even number of values");
    index = 0;
    offset = initialOffset;
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
    if (index < (int)durations->size()) {
        if (changeTimer->isScheduled())
            cancelClockEvent(changeTimer);
        scheduleChangeTimer();
    }
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
    if (std::isnan(bitrate.get()))
        return PacketGateBase::canPacketFlowThrough(packet);
    else if (packet == nullptr)
        return false;
    else {
        clocktime_t flowEndTime = getClockTime() + s((packet->getDataLength() + extraLength) / bitrate).get() + SIMTIME_AS_CLOCKTIME(extraDuration);
        return !changeTimer->isScheduled() || flowEndTime <= getArrivalClockTime(changeTimer);
    }
}

} // namespace queueing
} // namespace inet

