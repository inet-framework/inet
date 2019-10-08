//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

package inet.queueing.contract;

//
// This module interface must be implemented by packet collector modules. A packet
// collector is an active module with one input. It must be connected to a packet
// provider module from which it's going to actively collect packets over time.
//
// @see ~IPacketProvider, ~PacketCollector, ~PacketScheduler
//
moduleinterface IPacketCollector
{
    parameters:
        @display("i=block/sink");
    gates:
        input in @labels(pop);
}
