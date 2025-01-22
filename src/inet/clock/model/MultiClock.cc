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
        subscribe(ClockBase::timeJumpedSignal, this);
        subscribe(ClockBase::timeDifferenceToReferenceSignal, this);
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

void MultiClock::receiveSignal(cComponent *source, int signal, const simtime_t &time, cObject *details)
{
    if (signal == ClockBase::timeChangedSignal) {
        if (check_and_cast<IClock *>(source) == activeClock) {
            emit(ClockBase::timeChangedSignal, time, details);
        }
    }
    else if (signal == ClockBase::timeDifferenceToReferenceSignal) {
        // Might want to replace this with own signal and own reference clock
        if (check_and_cast<IClock *>(source) == activeClock) {
            emit(ClockBase::timeDifferenceToReferenceSignal, time, details);
        }
    }
    else {
        throw cRuntimeError("Unknown signal");
    }
}

void MultiClock::receiveSignal(cComponent *source, int signal, cObject *obj, cObject *details)
{
    if (signal == ClockBase::timeJumpedSignal) {
        if (check_and_cast<IClock *>(source) == activeClock)
            emit(ClockBase::timeJumpedSignal, this, details);
    }
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace inet
