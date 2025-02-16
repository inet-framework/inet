//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.servo;

// A PI-based servo clock with adjustable proportional (kp) and integral (ki) gains.
// It synchronizes the clock with an external time source by estimating and compensating drift.
// The clock operates in three phases:
//
// 1. Initialize: Collects initial offset data without adjustments.
//
// 2. Estimate Drift: Computes drift based on the difference between consecutive offsets.
//
// 3. Synchronize: Adjusts the clock using proportional and integral terms to minimize offset.
module PiServoClock extends ServoClockBase
{
    parameters:
        double offsetThreshold @unit(s) = default(0s); // Clock will perform a time jump if the offset is greater than this value, 0.0 disables this feature
        double kp = default(8);
        double ki = default(4);
        @signal[kp](type=double);
        @statistic[kp](title="kp value"; record=vector; interpolationmode=linear);
        @signal[drift](type=double);
        @statistic[drift](title="drift value"; record=vector; interpolationmode=linear);
        @class(PiServoClock);
}

