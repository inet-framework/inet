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
    //not tested
    Enter_Method("adjustClockTo");
    int64_t offsetNsPrev, offsetNs, localNsPrev, localNs;

    if  (newClockTime != oldClockTime)
    {
        clocktime_t oldClockTime = getClockTime();
        switch (phase) {
            case 0:
                offset[0] = newClockTime - oldClockTime;
                local[0] = oldClockTime;
                phase = 1;
                jumpClockTo(newClockTime);
                break;

            case 1:
                offset[1] = newClockTime - oldClockTime;
                local[1] = oldClockTime;

                offsetNsPrev = offset[0].inUnit(SIMTIME_NS);
                offsetNs = offset[1].inUnit(SIMTIME_NS);

                localNsPrev = local[0].inUnit(SIMTIME_NS);
                localNs = local[1].inUnit(SIMTIME_NS);

                drift = ppm(1e6 * (offsetNsPrev - offsetNs) / (localNsPrev - localNs));
                EV_INFO << "Drift: " << drift << "\n";
                break;
    }



    jumpClockTo(newClockTime);

    // TODO: Add a mechanism that estimates the drift rate based on the previous and current local and received
    //  timestamps, similar to case 0 and 1 in PiServoClock
}

} // namespace inet

