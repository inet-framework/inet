//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.queueing.common;

import inet.queueing.contract.IPacketDelayer;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.IPacketServer;

module QueueingPacketDelayer like IPacketDelayer
{
    parameters:
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        volatile double delay @unit(s);
        *.clockModule = default(absPath("^" + clockModule));
        @class(PacketDelayer);
        @display("i=block/delay");
    gates:
        input in @labels(push);
        output out @labels(push);
    submodules:
        queue: <default("PacketQueue")> like IPacketQueue {
            parameters:
                @display("p=150,100");
        }
        server: <default("PacketServer")> like IPacketServer {
            parameters:
                processingTime = default(delay);
                @display("p=350,100");
        }
    connections:
        in --> { @display("m=w"); } --> queue.in;
        queue.out --> server.in;
        server.out --> { @display("m=e"); } --> out;
}

