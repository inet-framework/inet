//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/clock/model/IdealClock.h"

namespace inet {

Define_Module(IdealClock);

clocktime_t IdealClock::computeClockTimeFromSimTime(simtime_t t) const
{
    return ClockTime::from(t);
}

simtime_t IdealClock::computeSimTimeFromClockTime(clocktime_t clock) const
{
    return clock.asSimTime();
}

} // namespace inet

