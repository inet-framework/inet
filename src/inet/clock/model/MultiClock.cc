//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/clock/model/MultiClock.h"

#include "inet/clock/base/ClockBase.h"

namespace inet {

Define_Module(MultiClock);

void MultiClock::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        activeClock = check_and_cast<IClock *>(getSubmodule("clock", par("activeClockIndex")));
        subscribe(ClockBase::timeChangedSignal, this);
    }
}

void MultiClock::handleParameterChange(const char *name)
{
    if (!strcmp(name, "activeClockIndex")) {
        emit(ClockBase::timeChangedSignal, getClockTime().asSimTime());
        activeClock = check_and_cast<IClock *>(getSubmodule("clock", par("activeClockIndex")));
        emit(ClockBase::timeChangedSignal, getClockTime().asSimTime());
    }
}

void MultiClock::receiveSignal(cComponent *source, int signal, const simtime_t& time, cObject *details)
{
    if (signal == ClockBase::timeChangedSignal) {
        if (check_and_cast<IClock *>(source) == activeClock)
            emit(ClockBase::timeChangedSignal, time, details);
    }
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace inet

