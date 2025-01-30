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

clocktime_t IdealClock::computeClockTimeFromSimTime(simtime_t simulationTime) const
{
    return SIMTIME_AS_CLOCKTIME(simulationTime);
}

simtime_t IdealClock::computeSimTimeFromClockTime(clocktime_t clockTime, bool lowerBound) const
{
    return CLOCKTIME_AS_SIMTIME(lowerBound ? clockTime : clockTime + ClockTime::fromRaw(1));
}

} // namespace inet

