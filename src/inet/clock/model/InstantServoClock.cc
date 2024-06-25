//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/clock/model/InstantServoClock.h"

namespace inet {

Define_Module(InstantServoClock);

void InstantServoClock::adjustClockTo(clocktime_t newClockTime)
{
    // not tested
    Enter_Method("adjustClockTo");

    clocktime_t oldClockTime = getClockTime();

    if (newClockTime != oldClockTime) {
        // At every clock jump we increase the clock time by offset
        // For our drift estimation, we need to know to keep track of the local times without
        // offsets and the offsets themselves
        // This we subtract the offset from the local time to get the local time without the offset
        // and accumulate the offsets
        auto local = oldClockTime.inUnit(SIMTIME_NS) - offsetPrev;
        auto offset = (newClockTime - oldClockTime).inUnit(SIMTIME_NS) + offsetPrev;

        drift += ppm(1e6 * (offsetPrev - offset) / (localPrev - local));
        EV_INFO << "Drift: " << drift << "\n";

        jumpClockTo(newClockTime);

        setOscillatorCompensation(drift);

        offsetPrev = offset;
        localPrev = local;
    }
}
// TODO: Add a mechanism that estimates the drift rate based on the previous and current local and received
//  timestamps, similar to case 0 and 1 in PiServoClock

} // namespace inet
