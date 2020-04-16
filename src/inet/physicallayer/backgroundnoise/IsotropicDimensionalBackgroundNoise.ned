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

package inet.physicallayer.backgroundnoise;

import inet.physicallayer.contract.packetlevel.IBackgroundNoise;

//
// This background noise model describes noise that does not change over space,
// time and frequency. It produces dimensional noise signals that can be further
// used in dimensional computations.
//
module IsotropicDimensionalBackgroundNoise like IBackgroundNoise
{
    parameters:
        double powerSpectralDensity @unit(dBmWpMHz) = default(nan dBmWpMHz); // when specified, the power and bandwidth parameters are not used
        double power @unit(dBm) = default(nan dBm); // when specified, the bandwidth can be determined automatically by matching the signal's bandwidth
        double bandwidth @unit(Hz) = default(nan Hz); // determined automatically if the value is NaN, otherwise must match the signal's bandwidth
        @class(IsotropicDimensionalBackgroundNoise);
        @display("i=block/mac");
}

