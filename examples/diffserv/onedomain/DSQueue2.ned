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

package inet.examples.diffserv.onedomain;

import inet.common.queue.DropTailQueue;
import inet.common.queue.FifoQueue;
import inet.common.queue.IOutputQueue;
import inet.common.queue.RedDropper;
import inet.common.queue.WrrScheduler;
import inet.networklayer.diffserv.BehaviorAggregateClassifier;


//
// Diffserv Queue used in Experiment 2.1 - 2.4.
//
module DSQueue2 like IOutputQueue
{
    gates:
        input in;
        output out;

    submodules:
        classifier: BehaviorAggregateClassifier {
            dscps = "AF11 AF12 AF13";
            @display("p=100,200");
        }
        red: RedDropper {
            numGates = 3;
            @display("p=250,100");
        }
        afQueue: FifoQueue {
            @display("p=400,100");
        }
        beQueue: DropTailQueue {
            @display("p=325,300");
        }
        wrr: WrrScheduler {
            @display("p=550,200");
        }
    connections:
        in --> classifier.in;
        classifier.outs++ --> red.in[0];
        classifier.outs++ --> red.in[1];
        classifier.outs++ --> red.in[2];
        classifier.defaultOut --> beQueue.in;

        red.out[0] --> afQueue.in++;
        red.out[1] --> afQueue.in++;
        red.out[2] --> afQueue.in++;
        beQueue.out --> wrr.in++;
        afQueue.out --> wrr.in++;
        wrr.out --> out;
}

