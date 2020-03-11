//
// Copyright (C) 2013 OpenSim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.physicallayer.base.packetlevel;

import inet.physicallayer.base.packetlevel.NarrowbandTransmitterBase;

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
        string timeGainsNormalization @enum("", "maximum", "integral") = default(""); // determines how the time gains function is normalized, no normalization by default
        string frequencyGains = default("left s 0dB either e 0dB right"); // sequence of interpolation method, frequency, gain tuples; frequency can be an expression, gain is in [dB] or [0..1]; default value is a flat signal
        string frequencyGainsNormalization @enum("", "maximum", "integral") = default("integral"); // determines how the frequency gains function is normalized, integral normalization by default
        modulation = default("BPSK");
        @class(DimensionalTransmitterBase);
}

