//
// Copyright (C) 2013 OpenSim Ltd.
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

import inet.physicallayer.contract.packetlevel.IMediumLimitCache;

module MediumLimitCache like IMediumLimitCache
{
    parameters:
        double centerFrequency @unit(Hz) = default(nan Hz);       // TODO: rename to maxRangeCenterFrequency?
        double maxSpeed @unit(mps) = default(nan mps);             // maxium speed on the medium, NaN means medium computes using mobility models
        double maxTransmissionPower @unit(W) = default(nan W);     // TODO: @unit(W) + dBm/dBW <--> W
        double minInterferencePower @unit(dBm) = default(nan dBm); // TODO: @unit(W) + dBm/dBW <--> W
        double minReceptionPower @unit(dBm) = default(nan dBm);    // TODO: @unit(W) + dBm/dBW <--> W
        double maxAntennaGain @unit(dB) = default(0dB);           // maximum antenna gain on the medium, NaN means medium computes using antenna models
        double minInterferenceTime @unit(s) = default(1ps);       // minimum time interval to consider two overlapping signals interfering
        double maxTransmissionDuration @unit(s) = default(10ms);  // maximum duration of a transmission on the medium
        double maxCommunicationRange @unit(m) = default(nan m);    // maximum communication range on the medium, NaN means medium computes using transmitter and receiver models
        double maxInterferenceRange @unit(m) = default(nan m);     // maximum interference range on the medium, NaN means medium computes using transmitter and receiver models
        @display("i=block/table2");
        @class(MediumLimitCache);
}

