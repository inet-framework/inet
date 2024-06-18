//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/clock/model/SettableClock.h"

namespace inet {

Define_Module(SettableClock);


void SettableClock::adjustClockTime(clocktime_t newClockTime)
{
    Enter_Method("adjustClockTime");
    clocktime_t oldClockTime = getClockTime();
    clocktime_t diff = newClockTime - oldClockTime;

    if (newClockTime != oldClockTime) {
        emit(timeChangedSignal, oldClockTime.asSimTime());

        simtime_t currentSimTime = simTime();
        EV_DEBUG << "Setting clock time from " << oldClockTime << " to " << newClockTime << " at simtime " << currentSimTime << ".\n";
        originSimulationTime = simTime();
        originClockTime = newClockTime;

        ASSERT(newClockTime == getClockTime());
        rescheduleClockEvents(oldClockTime, newClockTime);
        emit(timeChangedSignal, newClockTime.asSimTime());

    }

}

} // namespace inet

