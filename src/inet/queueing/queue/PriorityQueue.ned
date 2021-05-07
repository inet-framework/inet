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

package inet.queueing.queue;

import inet.queueing.contract.IPacketBuffer;
import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.IPacketScheduler;

//
// This module implements a priority queue with multiple inner queues and an
// optional shared memory buffer.
//
module PriorityQueue extends CompoundPacketQueue
{
    parameters:
        int numQueues;
    submodules:
        buffer: <default("PriorityBuffer")> like IPacketBuffer if typename != "" {
            parameters:
                @display("p=100,225");
        }
        classifier: <default("PacketClassifier")> like IPacketClassifier {
            parameters:
                @display("p=100,100");
        }
        queue[numQueues]: <default("PacketQueue")> like IPacketQueue {
            parameters:
                bufferModule = default(exists(parent.buffer) ? "^.buffer" : "");
                @display("p=325,100,column,125");
        }
        scheduler: <default("PriorityScheduler")> like IPacketScheduler {
            parameters:
                @display("p=550,100");
        }
    connections:
        in --> { @display("m=w"); } --> classifier.in;
        for i=0..sizeof(queue)-1 {
            classifier.out++ --> queue[i].in;
            queue[i].out --> scheduler.in++;
        }
        scheduler.out --> { @display("m=e"); } --> out;
}

