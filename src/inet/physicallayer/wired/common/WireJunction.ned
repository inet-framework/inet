//
// Copyright (C) 2020 Opensim Ltd.
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


package inet.physicallayer.wired.common;

//
// Models a generic wiring hub.
//
// The model simply broadcasts messages (packets, frames, signals)
// received on one port on all other ports, mimicking the propagation
// of electrical signals. Messages are not interpreted by the model
// in any way, so this module is protocol agnostic.
//
// Note that no special attention is made to appropriately model
// frame truncations that occur when the link breaks, or comes
// back up while a transmission is underway. If you need
// to precisely model what happens when the link state changes,
// you cannot use this module.
//
simple WireJunction
{
    parameters:
        @display("i=misc/node_vs");
        @signal[packetReceived](type=cMessage);
        @statistic[packetReceived](title="packets"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        inout port[];
}
