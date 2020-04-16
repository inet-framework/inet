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

package inet.networklayer.diffserv;

import inet.queueing.queue.CompoundPacketQueue;
import inet.queueing.queue.DropTailQueue;
import inet.queueing.scheduler.PriorityScheduler;
import inet.queueing.scheduler.WrrScheduler;
import inet.queueing.sink.PassivePacketSink;

//
// This is an example queue, that can be used in
// interfaces of DS core and edge nodes to support
// the AFxy (RFC 2597) and EF (RFC 3246) PHBs.
//
// The incoming packets are first classified according to
// their DSCP field. DSCPs other than AFxy and EF are handled
// as BE (best effort).
//
// EF packets are stored in a dedicated queue, and served first
// when a packet is requested. Because they can preempt the other
// queues, the rate of the EF packets should be limited to a fraction
// of the bandwith of the link. This is achieved by metering the EF
// traffic with a token bucket meter and dropping packets that
// does not conform to the traffic profile.
//
// There are other queues for AFx classes and BE. The AFx queues
// use RED to implement 3 different drop priorities within the class.
// BE packets are stored in a drop tail queue.
// Packets from AFxy and BE queues are sheduled by a WRR scheduler,
// which ensures that the remaining bandwith is allocated among the classes
// according to the specified weights.
//
// @see ~AFxyQueue
//
module DiffservQueue extends CompoundPacketQueue
{
    parameters:
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(interfaceTableModule));
    submodules:
        classifier: BehaviorAggregateClassifier {
            dscps = "EF AF11 AF12 AF13 AF21 AF22 AF23 AF31 AF32 AF33 AF41 AF42 AF43";
            @display("p=100,330");
        }
        efMeter: TokenBucketMeter {
            cir = default("10%"); // reserved EF bandwith as percentage of datarate of the interface
            cbs = default(5000B); // 5 1000B packets
            @display("p=250,130");
        }
        sink: PassivePacketSink {
            @display("p=400,80");
        }
        efQueue: DropTailQueue {
            packetCapacity = default(5); // keep low, for low delay and jitter
            @display("p=400,180");
        }
        af1xQueue: AFxyQueue {
            @display("p=250,230");
        }
        af2xQueue: AFxyQueue {
            @display("p=250,330");
        }
        af3xQueue: AFxyQueue {
            @display("p=250,430");
        }
        af4xQueue: AFxyQueue {
            @display("p=250,530");
        }
        beQueue: DropTailQueue {
            @display("p=250,630");
        }
        wrr: WrrScheduler {
            weights = default("1 1 1 1 1");
            @display("p=400,330");
        }
        priority: PriorityScheduler {
            @display("p=550,330");
        }

    connections:
        in --> classifier.in;
        classifier.out++ --> efMeter.in;
        classifier.out++ --> af1xQueue.afx1In;
        classifier.out++ --> af1xQueue.afx2In;
        classifier.out++ --> af1xQueue.afx3In;
        classifier.out++ --> af2xQueue.afx1In;
        classifier.out++ --> af2xQueue.afx2In;
        classifier.out++ --> af2xQueue.afx3In;
        classifier.out++ --> af3xQueue.afx1In;
        classifier.out++ --> af3xQueue.afx2In;
        classifier.out++ --> af3xQueue.afx3In;
        classifier.out++ --> af4xQueue.afx1In;
        classifier.out++ --> af4xQueue.afx2In;
        classifier.out++ --> af4xQueue.afx3In;
        classifier.defaultOut --> beQueue.in;

        efMeter.greenOut --> { @display("ls=green"); } --> efQueue.in;
        efMeter.redOut --> { @display("ls=red"); } --> sink.in;

        af1xQueue.out --> wrr.in++;
        af2xQueue.out --> wrr.in++;
        af3xQueue.out --> wrr.in++;
        af4xQueue.out --> wrr.in++;
        beQueue.out --> wrr.in++;
        efQueue.out --> priority.in++;
        wrr.out --> priority.in++;
        priority.out --> out;
}

