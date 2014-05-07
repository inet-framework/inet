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

package inet.physicallayer.radio.generic;

import inet.physicallayer.radio.common.RadioBase;
import inet.physicallayer.radio.contract.IRadioAntenna;
import inet.physicallayer.radio.contract.IRadioSignalReceiver;
import inet.physicallayer.radio.contract.IRadioSignalTransmitter;

simple Radio extends RadioBase
{
    parameters:
        string antennaType;
        string transmitterType;
        string receiverType;
        // TODO: add parameter to specify radio channel
        @class(Radio);

    submodules:
        antenna: <antennaType> like IRadioAntenna;
        transmitter: <transmitterType> like IRadioSignalTransmitter;
        receiver: <receiverType> like IRadioSignalReceiver;
}
