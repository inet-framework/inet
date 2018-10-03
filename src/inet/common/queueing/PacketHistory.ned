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

package inet.common.queueing;

import inet.common.queueing.contract.IPacketConsumer;
import inet.common.queueing.contract.IPacketProducer;
import inet.common.queueing.contract.IPacketQueue;

module PacketHistory like IPacketConsumer, IPacketProducer
{
    parameters:
        int size = default(100);
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
                frameCapacity = default(size);
                @display("p=100,200");
        }
    connections allowunconnected:
        in --> { @display("m=w"); } --> cloner.in;
        cloner.out++ --> { @display("m=e"); } --> out;
        cloner.out++ --> queue.in;
}

