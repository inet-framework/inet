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

import inet.physicallayer.common.packetlevel.Radio;

//
// This module servces as a base module for narrowband radio models.
//
module NarrowbandRadioBase extends Radio
{
    parameters:
        double centerFrequency @unit(Hz);     // center frequency of the band where the radio transmits and receives signals on the medium
        double bandwidth @unit(Hz);            // bandwidth of the band where the radio transmits and receives signals on the medium
        *.centerFrequency = default(this.centerFrequency); // passed down to transmitter and receiver by default
        *.bandwidth = default(this.bandwidth);               // passed down to transmitter and receiver by default
}

