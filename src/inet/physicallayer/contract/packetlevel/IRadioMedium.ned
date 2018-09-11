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

package inet.physicallayer.contract.packetlevel;

//
// The medium model describes the shared physical medium where communication
// takes place. It keeps track of radios, noise sources, ongoing transmissions,
// background noise, and other ongoing noises. The medium computes when, where
// and how transmissions and noises arrive at receivers.
//
moduleinterface IRadioMedium
{
    parameters:
        @display("i=misc/sun");
        @signal[radioAdded](type=cModule);    // (type=IRadio)
        @signal[radioRemoved](type=cModule);    // (type=IRadio)
        @signal[signalAdded](type=cObject);    // (type=ITransmission)
        @signal[signalRemoved](type=cObject);    // (type=ITransmission)
        @signal[signalDepartureStarted](type=cObject);    // (type=ITransmission)
        @signal[signalDepartureEnded](type=cObject);    // (type=ITransmission)
        @signal[signalArrivalStarted](type=cObject);    // (type=IReception)
        @signal[signalArrivalEnded](type=cObject);    // (type=IReception)
}

