//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/clock/servo/NoClockServo.h"

#include "inet/common/IPrintableObject.h"

namespace inet {

Define_Module(NoClockServo);

void NoClockServo::adjustClockForDifference(clocktime_t timeDifference, ppm rateDifference)
{
    Enter_Method("adjustClockForDifference");
    EV_INFO << "Ignoring clock difference" << EV_FIELD(timeDifference) << EV_FIELD(rateDifference) << EV_ENDL;
}

} // namespace inet

