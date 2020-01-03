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

import inet.networklayer.diffserv.BehaviorAggregateClassifier;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.filter.RedDropper;
import inet.queueing.queue.CompoundPacketQueue;
import inet.queueing.queue.DropTailQueue;
import inet.queueing.queue.PacketQueue;
import inet.queueing.scheduler.WrrScheduler;

//
// Diffserv Queue used in Experiment 2.1 - 2.4.
//
module DSQueue2 extends CompoundPacketQueue
{
    submodules:
        classifier: BehaviorAggregateClassifier {
            dscps = "AF11 AF12 AF13";
            @display("p=100,200");
        }
        red1: RedDropper {
            collectionModule = "^.afQueue";
            @display("p=200,100");
        }
        red2: RedDropper {
            collectionModule = "^.afQueue";
            @display("p=200,200");
        }
        red3: RedDropper {
            collectionModule = "^.afQueue";
            @display("p=200,300");
        }
        mux: PacketMultiplexer {
            @display("p=300,200");
        }
        afQueue: PacketQueue {
            @display("p=400,200");
        }
        beQueue: DropTailQueue {
            @display("p=200,400");
        }
        wrr: WrrScheduler {
            @display("p=500,300");
        }
    connections:
        in --> classifier.in;
        classifier.out++ --> red1.in;
        classifier.out++ --> red2.in;
        classifier.out++ --> red3.in;
        classifier.defaultOut --> beQueue.in;

        red1.out --> mux.in++;
        red2.out --> mux.in++;
        red3.out --> mux.in++;
        mux.out --> afQueue.in;
        beQueue.out --> wrr.in++;
        afQueue.out --> wrr.in++;
        wrr.out --> out;
}

