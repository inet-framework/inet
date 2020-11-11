//
// Copyright (C) 2012 OpenSim Ltd.
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

package inet.linklayer.ethernet.common;

import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.IPacketScheduler;
import inet.queueing.filter.RedDropper;
import inet.queueing.queue.CompoundPacketQueueBase;
import inet.queueing.queue.DropTailQueue;

//
// Queue module that gives the PAUSE frames a higher priority.
//
//# TODO: rename to EthernetQosQueue or keep name as it is?
module EthernetQueue extends DropTailQueue
{
    parameters:
        comparatorClass = default("inet::EthernetFrameTypeComparator");
}

//
// Queue module that gives the PAUSE frames a higher priority,
// and can be parametrized with an ~IPacketQueue for serving the
// data frames.
//
//# TODO: rename to CompoundEtherQueue? or CompoundEtherQosQueue?
module EthernetQosQueue extends CompoundPacketQueueBase like IPacketQueue
{
    parameters:
        @display("i=block/queue");
    submodules:
        classifier: <default("EthernetFrameClassifier")> like IPacketClassifier {
            parameters:
                @display("p=100,200");
        }
        pauseQueue: <default("DropTailQueue")> like IPacketQueue {
            parameters:
                @display("p=250,100");
        }
        dataQueue: <default("DropTailQueue")> like IPacketQueue {
            parameters:
                @display("p=250,300;q=l2queue");
        }
        scheduler: <default("PriorityScheduler")> like IPacketScheduler {
            parameters:
                @display("p=400,200");
        }
    connections:
        in --> classifier.in;
        classifier.out++ --> pauseQueue.in;
        classifier.out++ --> dataQueue.in;
        pauseQueue.out --> scheduler.in++;
        dataQueue.out --> scheduler.in++;
        scheduler.out --> out;
}

//
// Queue module that gives the PAUSE frames a higher priority,
// and using Random Early Detection algorithm on data frames,
// and can be parametrized with an ~IPacketQueue for serving the
// data frames.
//
module EthernetQosRedQueue extends CompoundPacketQueueBase like IPacketQueue
{
    parameters:
        @display("i=block/queue");
    submodules:
        classifier: <default("EthernetFrameClassifier")> like IPacketClassifier {
            parameters:
                @display("p=100,200");
        }
        pauseQueue: <default("DropTailQueue")> like IPacketQueue {
            parameters:
                @display("p=250,100");
        }
        redFilter: RedDropper {
            @display("p=190,300");
        }
        dataQueue: <default("DropTailQueue")> like IPacketQueue {
            parameters:
                @display("p=310,300;q=l2queue");
        }
        scheduler: <default("PriorityScheduler")> like IPacketScheduler {
            parameters:
                @display("p=400,200");
        }
    connections:
        in --> classifier.in;
        classifier.out++ --> pauseQueue.in;
        classifier.out++ --> redFilter.in;
        pauseQueue.out --> scheduler.in++;
        dataQueue.out --> scheduler.in++;
        scheduler.out --> out;
        redFilter.out --> dataQueue.in;
}

