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

package inet.physicallayer.contract;

import inet.physicallayer.contract.IPhysicalLayer;

//
// Prototype for radio modules. Radio modules deal with the transmission of
// frames over a wireless medium (the radio medium).
//
// <b>Transmission</b>
//
// Upper layers (wireless L2 protocols) can send frames to the radio module.
// Frames will be encapsulated into RadioFrame messages and distributed to
// other network nodes within communication range.
//
// The global ~IRadioMedium module keeps track of node positions, and knows
// which nodes are within communication and interference distance of others
// (neighbour list). When transmitting, the radio module obtains the neighbour
// list, and sends a copy of the RadioFrame to each neighbour.
//
//
// <b>Reception</b>
//
// Received RadioFrame messages get delivered to the radioIn gate of the
// module. If it was found to have been received correctly, it gets decapsulated
// and the frame sent to the upper layer.
//
// Note: currently the packet is also sent up if it was NOT received correctly,
// with its error flag set to true.
//
moduleinterface IRadio extends IPhysicalLayer
{
    parameters:
        @display("i=block/wrxtx");
        @signal[radioModeChanged](type=long);
        @signal[listeningChanged];
        @signal[receptionStateChanged](type=long);
        @signal[transmissionStateChanged](type=long);
        @signal[radioChannelChanged](type=long);
    gates:
        input radioIn @labels(IRadioFrame);
}
