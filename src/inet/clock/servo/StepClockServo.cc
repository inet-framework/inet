//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/clock/servo/StepClockServo.h"

#include "inet/common/IPrintableObject.h"

namespace inet {

Define_Module(StepClockServo);

void StepClockServo::adjustClockForDifference(clocktime_t timeDifference, ppm rateDifference)
{
    Enter_Method("adjustClockForDifference");
    EV_INFO << "Immediately adjusting clock time and oscillator compensation for difference" << EV_FIELD(timeDifference) << EV_FIELD(rateDifference) << EV_ENDL;
    clock->setClockTime(clock->getClockTime() - timeDifference, rateDifference, true);
}

} // namespace inet

