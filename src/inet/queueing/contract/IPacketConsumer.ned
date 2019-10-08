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
// This module interface must be implemented by packet consumer modules. A packet
// consumer is a passive module with one input. It must be connected to a packet
// producer module which is going to actively push packets into it over time.
//
// @see ~IPacketProducer, ~PacketConsumer, ~PacketClassifier
//
moduleinterface IPacketConsumer
{
    parameters:
        @display("i=block/sink");
    gates:
        input in @labels(push);
}
