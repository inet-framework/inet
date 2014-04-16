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

package inet.physicallayer.radio.ideal;

import inet.physicallayer.radio.common.RadioBase;
import inet.physicallayer.radio.simplified.dummy;

//
// Implements a unit disk radio for use in ~IdealWirelessNic.
//
// See ~IdealWirelessNic for rationale and details.
//
// See ~IdealRadioChannel, ~IdealMac, ~IdealWirelessNic
//
simple IdealRadio extends RadioBase
{
    parameters:
        double bitrate @unit("bps");                         // transmission speed [bit/sec]
        double transmissionRange @unit("m") = default(1m/0); // max. distance from sender where reception is possible
        double interferenceRange @unit("m") = default(1m/0); // max. distance from sender where interference is possible
        bool drawCoverage = default(true);                   // draw the coverage area in Tkenv
        @class(IdealRadio);

    submodules:
        antenna: dummy;
        transmitter: dummy;
        receiver: dummy;
}
