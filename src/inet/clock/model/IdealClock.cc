//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/clock/model/IdealClock.h"

namespace inet {

Define_Module(IdealClock);

clocktime_t IdealClock::computeClockTimeFromSimTime(simtime_t simulationTime, bool lowerBound) const
{
    return SIMTIME_AS_CLOCKTIME(lowerBound ? simulationTime : simulationTime + SimTime::fromRaw(1));
}

simtime_t IdealClock::computeSimTimeFromClockTime(clocktime_t clockTime, bool lowerBound) const
{
    return CLOCKTIME_AS_SIMTIME(lowerBound ? clockTime : clockTime + ClockTime::fromRaw(1));
}

} // namespace inet

