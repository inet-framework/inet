//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
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

package inet.linklayer.ethernet;

import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.IPacketScheduler;
import inet.queueing.filter.RedDropper;
import inet.queueing.queue.CompoundPacketQueue;
import inet.queueing.queue.DropTailQueue;

//
// Queue module that gives the PAUSE frames a higher priority.
//
//# TODO: rename to EtherQosQueue or keep name as it is?
module EtherQueue extends DropTailQueue
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
module EtherQosQueue extends CompoundPacketQueue like IPacketQueue
{
    parameters:
        @display("i=block/queue");
    submodules:
        classifier: <default("EtherFrameClassifier")> like IPacketClassifier {
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
module EtherQosRedQueue extends CompoundPacketQueue like IPacketQueue
{
    parameters:
        @display("i=block/queue");
    submodules:
        classifier: <default("EtherFrameClassifier")> like IPacketClassifier {
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

