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

import inet.linklayer.IOutputQueue;
import inet.linklayer.queue.DropTailQueue;
import inet.linklayer.queue.PriorityScheduler;


//
// Queue module that gives the PAUSE frames a higher priority,
// and can be parametrized with an ~IOutputQueue for serving the
// other frames.
//
module EtherQoSQueue like IOutputQueue
{
    parameters:
        string dataQueueType = default("DropTailQueue");  // class that inherits from IOutputQueue (~DropTailQueue, ~DropTailQoSQueue, ~REDQueue)
        @display("i=block/queue");
    gates:
        input in;
        output out;
    submodules:
        classifier: EtherFrameClassifier {
            @display("p=46,145");
        }
        pauseQueue: DropTailQueue {
            queueName = "pauseQueue";
            @display("p=187,91");
        }
        dataQueue: <dataQueueType> like IOutputQueue {
            parameters:
                @display("p=187,192;q=l2queue");
        }
        scheduler: PriorityScheduler {
            @display("p=318,145");
        }
    connections:
        in --> classifier.in;
        classifier.pauseOut --> pauseQueue.in;
        classifier.defaultOut --> dataQueue.in;
        pauseQueue.out --> scheduler.in++;
        dataQueue.out --> scheduler.in++;
        scheduler.out --> out;
}

