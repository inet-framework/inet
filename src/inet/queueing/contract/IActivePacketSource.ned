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
// This module interface must be implemented by active packet source modules.
// An active packet source, also called a producer, has one output that must
// be connected to a passive packet sink module. The active packet source is
// expected to push packets into the connected passive packet sink over time.
//
// @see ~PassivePacketSink, ~PacketServer, ~PacketClassifier, ~IPassivePacketSink
//
moduleinterface IActivePacketSource
{
    parameters:
        @display("i=block/source");
    gates:
        output out @labels(push);
}
