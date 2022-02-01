//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.noise;

import inet.physicallayer.wireless.common.contract.packetlevel.ITransmitter;

module NoiseDimensionalTransmitter like ITransmitter
{
    parameters:
        volatile double duration @unit(s);
        volatile double centerFrequency @unit(Hz) = default(nan Hz);
        volatile double bandwidth @unit(Hz) = default(0Hz);
        volatile double power @unit(W);
        int gainFunctionCacheLimit = default(0); // limits the number of gain functions cached for a given duration, centerFrequency and bandwidth
        string timeGains = default("left s 0dB either e 0dB right"); // sequence of interpolation method, time, gain tuples; time can be an expression, gain is in [dB] or [0..inf); default value is a flat signal
        string timeGainsNormalization @enum("","maximum","integral") = default(""); // determines how the time gains function is normalized, no normalization by default
        string frequencyGains = default("left s 0dB either e 0dB right"); // sequence of interpolation method, frequency, gain tuples; frequency can be an expression, gain is in [dB] or [0..1]; default value is a flat signal
        string frequencyGainsNormalization @enum("","maximum","integral") = default("integral"); // determines how the frequency gains function is normalized, integral normalization by default
        @class(NoiseDimensionalTransmitter);
        @display("i=block/wtx");
}

