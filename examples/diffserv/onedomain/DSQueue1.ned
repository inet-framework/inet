//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.diffserv.onedomain;

import inet.networklayer.diffserv.BehaviorAggregateClassifier;
import inet.queueing.queue.CompoundPacketQueueBase;
import inet.queueing.queue.DropTailQueue;
import inet.queueing.scheduler.PriorityScheduler;
import inet.queueing.scheduler.WrrScheduler;

//
// Diffserv Queue used in Experiment 1.1 - 1.6 and 5.1.
//
module DSQueue1 extends CompoundPacketQueueBase
{
    submodules:
        classifier: BehaviorAggregateClassifier {
            dscps = "EF AF11 AF21 AF31 AF41";
            @display("p=100,350");
        }
        efQueue: DropTailQueue {
            @display("p=250,100");
        }
        af1xQueue: DropTailQueue {
            @display("p=250,200");
        }
        af2xQueue: DropTailQueue {
            @display("p=250,300");
        }
        af3xQueue: DropTailQueue {
            @display("p=250,400");
        }
        af4xQueue: DropTailQueue {
            @display("p=250,500");
        }
        beQueue: DropTailQueue {
            @display("p=250,600");
        }
        wrr: WrrScheduler {
            @display("p=400,350");
        }
        priority: PriorityScheduler {
            @display("p=550,350");
        }

    connections:
        in --> classifier.in;
        classifier.out++ --> efQueue.in;
        classifier.out++ --> af1xQueue.in;
        classifier.out++ --> af2xQueue.in;
        classifier.out++ --> af3xQueue.in;
        classifier.out++ --> af4xQueue.in;
        classifier.defaultOut --> beQueue.in;

        af1xQueue.out --> wrr.in++;
        af2xQueue.out --> wrr.in++;
        af3xQueue.out --> wrr.in++;
        af4xQueue.out --> wrr.in++;
        beQueue.out --> wrr.in++;
        efQueue.out --> priority.in++;
        wrr.out --> priority.in++;
        priority.out --> out;
}

