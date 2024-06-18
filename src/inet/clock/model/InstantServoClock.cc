//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/clock/model/InstantServoClock.h"

namespace inet {

Define_Module(InstantServoClock);


void InstantServoClock::adjustClockTime(clocktime_t newClockTime)
{
    Enter_Method("adjustClockTime");
    setClockTime(newClockTime);

    // TODO: Add a mechanism that estimates the drift rate based on the previous and current local and received
    //  timestamps, similar to case 0 and 1 in PiServoClock
}

} // namespace inet

