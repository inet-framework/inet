//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.servo;

// TODO: Write documentation
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

