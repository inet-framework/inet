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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

package inet.node.ethernet;

//
// An Ethernet hub model.
//
// The model simply broadcasts frames (and transmission updates
// representing aborted frames, etc.) received on one port on all
// other ports, mimicking the propagation of electrical signals.
//
// Note that no special attention is made to appropriately model
// frame truncations that occur when the link breaks, or comes
// back up while a transmission is underway. If you need
// to precisely model what happens when the link state changes,
// you cannot use this module.
//
simple EthernetHub
{
    parameters:
        @networkNode();
        @class(::inet::physicallayer::WireJunction);
        @labels(node,ethernet-node);
        @display("i=device/hub;bgb=155,104");
        @signal[packetReceived](type=cMessage);
        @statistic[packetReceived](title="packets"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        inout ethg[] @labels(EtherFrame-conn);
}

