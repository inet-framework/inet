//
// Copyright (C) OpenSim Ltd
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

package inet.physicallayer.common.packetlevel;

import inet.physicallayer.contract.packetlevel.ITransmitter;

module NoiseDimensionalTransmitter like ITransmitter
{
    parameters:
        volatile double duration @unit(s);
        volatile double centerFrequency @unit(Hz) = default(nan Hz);
        volatile double bandwidth @unit(Hz) = default(0Hz);
        volatile double power @unit(W);
        string timeGains = default("left s 0dB either e 0dB right"); // sequence of interpolation method, time, gain tuples; time can be an expression, gain is in [dB] or [0..inf); default value is a flat signal
        string timeGainsNormalization @enum("","maximum","integral") = default(""); // determines how the time gains function is normalized, no normalization by default
        string frequencyGains = default("left s 0dB either e 0dB right"); // sequence of interpolation method, frequency, gain tuples; frequency can be an expression, gain is in [dB] or [0..1]; default value is a flat signal
        string frequencyGainsNormalization @enum("","maximum","integral") = default("integral"); // determines how the frequency gains function is normalized, integral normalization by default
        @class(NoiseDimensionalTransmitter);
        @display("i=block/wtx");
}

