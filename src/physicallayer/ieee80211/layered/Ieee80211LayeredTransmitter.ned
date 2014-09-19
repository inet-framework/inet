//
// Copyright (C) 2014 OpenSim Ltd
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

package inet.physicallayer.ieee80211.layered;

import inet.physicallayer.contract.IDigitalAnalogConverter;
import inet.physicallayer.contract.IEncoder;
import inet.physicallayer.contract.IModulator;
import inet.physicallayer.contract.IPulseShaper;
import inet.physicallayer.layered.LayeredTransmitter;


module Ieee80211LayeredTransmitter extends LayeredTransmitter
{
    parameters:
        encoderType = "Ieee80211LayeredEncoder";
        modulatorType = "Ieee80211OFDMModulator";
        string codeRate; // "1/2", "3/4", ...
        @class(Ieee80211LayeredTransmitter);
}
