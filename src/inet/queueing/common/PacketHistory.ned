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

package inet.queueing.common;

import inet.queueing.contract.IPassivePacketSink;
import inet.queueing.contract.IActivePacketSource;
import inet.queueing.contract.IPacketQueue;

//
// This module connects one packet producer to one packet consumer. It can be
// pushed with packets from the connected packet producer. It keeps a copy of
// the last N packets pushed into its input. The packets are available in the
// runtime user interface (Qtenv) for inspection.
//
module PacketHistory like IPassivePacketSink, IActivePacketSource
{
    parameters:
        int size = default(100); // the number of packets to remember
        @display("i=block/passiveq");
    gates:
        input in @labels(push);
        output out @labels(push);
    submodules:
        cloner: PacketCloner {
            parameters:
                @display("p=100,100");
        }
        queue: <default("DropHeadQueue")> like IPacketQueue {
            parameters:
                packetCapacity = default(size);
                @display("p=100,200");
        }
    connections:
        in --> { @display("m=w"); } --> cloner.in;
        cloner.out++ --> { @display("m=e"); } --> out;
        cloner.out++ --> queue.in;
}

