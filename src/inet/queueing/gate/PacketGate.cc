//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/gate/PacketGate.h"

#include <algorithm>

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace queueing {

Define_Module(PacketGate);

void PacketGate::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        isOpen_ = par("initiallyOpen");
        openTime = par("openTime");
        closeTime = par("closeTime");
        changeTimer = new ClockEvent("ChangeTimer");
    }
    else if (stage == INITSTAGE_QUEUEING)
        scheduleChangeTimer();
}

void PacketGate::handleMessage(cMessage *message)
{
    if (message == changeTimer) {
        processChangeTimer();
        scheduleChangeTimer();
    }
    else
        throw cRuntimeError("Unknown message");
}

void PacketGate::scheduleChangeTimer()
{
    clocktime_t changeTime;
    if (isOpen_)
        changeTime = closeTime;
    else
        changeTime = openTime;
    if (changeTime > getClockTime())
        scheduleClockEventAt(changeTime, changeTimer);
}

void PacketGate::processChangeTimer()
{
    if (isOpen_)
        close();
    else
        open();
}

} // namespace queueing
} // namespace inet

