//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.backgroundnoise;

import inet.physicallayer.wireless.common.contract.packetlevel.IBackgroundNoise;

module DimensionalBackgroundNoise like IBackgroundNoise
{
    parameters:
        int gainFunctionCacheLimit = default(0); // limits the number of gain functions cached for a given duration, centerFrequency and bandwidth
        string timeGains = default("left 0s 0dB left"); // sequence of interpolation method, time, gain tuples; time can be an expression, gain is in [dB] or [0..inf); default value is a flat signal
        string timeGainsNormalization @enum("", "maximum", "integral") = default(""); // determines how the time gains function is normalized, no normalization by default
        string frequencyGains = default("left 0Hz 0dB left"); // sequence of interpolation method, frequency, gain tuples; frequency can be an expression, gain is in [dB] or [0..1]; default value is a flat signal
        string frequencyGainsNormalization @enum("", "maximum", "integral") = default("integral"); // determines how the frequency gains function is normalized, integral normalization by default
        double power @unit(dBm); // noise power
        @class(DimensionalBackgroundNoise);
        @display("i=block/mac");
}

