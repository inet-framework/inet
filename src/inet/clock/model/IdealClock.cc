//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/clock/model/IdealClock.h"

namespace inet {

Define_Module(IdealClock);

void IdealClock::initialize(int stage)
{
    ClockBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        emit(timeChangedSignal, simTime());
}

void IdealClock::finish()
{
    ClockBase::finish();
    emit(timeChangedSignal, simTime());
}

clocktime_t IdealClock::computeClockTimeFromSimTime(simtime_t t) const
{
    return ClockTime::from(t);
}

simtime_t IdealClock::computeSimTimeFromClockTime(clocktime_t clock) const
{
    return clock.asSimTime();
}

} // namespace inet

