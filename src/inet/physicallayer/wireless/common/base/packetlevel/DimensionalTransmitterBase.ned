//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.base.packetlevel;

//
// This transmitter model produces transmissions that use dimensional
// transmission power (that changes over time and/or frequency) in their analog
// representation and the configured modulation.
//
// Allowed expressions for time and frequency coordinates in the gains parameters:
// <pre>
//   s|c|e
//   s|c|e+-quantity
//   s|c|e+-b|d
//   s|c|e+-b|d+-quantity
//   s|c|e+-b|d*number
//   s|c|e+-b|d*number+-quantity
// </pre>
// where s = start, e = end, c = center frequency, b = bandwidth, d = duration. For example: c-b*0.5-1MHz
//
module DimensionalTransmitterBase extends NarrowbandTransmitterBase
{
    parameters:
        int gainFunctionCacheLimit = default(0); // limits the number of gain functions cached for a given duration, centerFrequency and bandwidth
        string timeGains = default("left s 0dB either e 0dB right"); // sequence of interpolation method, time, gain tuples; time can be an expression, gain is in [dB] or [0..inf); default value is a flat signal
        string timeGainsNormalization @enum("","maximum","integral") = default(""); // determines how the time gains function is normalized, no normalization by default
        string frequencyGains = default("left s 0dB either e 0dB right"); // sequence of interpolation method, frequency, gain tuples; frequency can be an expression, gain is in [dB] or [0..1]; default value is a flat signal
        string frequencyGainsNormalization @enum("","maximum","integral") = default("integral"); // determines how the frequency gains function is normalized, integral normalization by default
        modulation = default("BPSK");
        @class(DimensionalTransmitterBase);
}

