//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.analogmodel.dimensional;

import inet.common.CompoundModule;
import inet.physicallayer.wireless.common.contract.packetlevel.ITransmitterAnalogModel;

module DimensionalTransmitterAnalogModel extends CompoundModule like ITransmitterAnalogModel
{
    parameters:
        double power @unit(W) = default(nan Hz); // Default transmission power
        double centerFrequency @unit(Hz) = default(nan Hz); // Default center frequency
        double bandwidth @unit(Hz) = default(nan Hz); // Default bandwidth
        int gainFunctionCacheLimit = default(0); // Limits the number of gain functions cached for a given duration, centerFrequency and bandwidth
        string timeGains = default("left s 0dB either e 0dB right"); // Sequence of interpolation method, time, gain tuples; time can be an expression, gain is in [dB] or [0..inf); default value is a flat signal
        string timeGainsNormalization @enum("","maximum","integral") = default(""); // Determines how the time gains function is normalized, no normalization by default
        string frequencyGains = default("left s 0dB either e 0dB right"); // Sequence of interpolation method, frequency, gain tuples; frequency can be an expression, gain is in [dB] or [0..1]; default value is a flat signal
        string frequencyGainsNormalization @enum("","maximum","integral") = default("integral"); // Determines how the frequency gains function is normalized, integral normalization by default
        @class(DimensionalTransmitterAnalogModel);
        @display("i=block/tunnel");
}
