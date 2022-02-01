//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.diffserv.onedomain;

import inet.networklayer.diffserv.BehaviorAggregateClassifier;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.filter.RedDropper;
import inet.queueing.queue.CompoundPacketQueueBase;
import inet.queueing.queue.DropTailQueue;
import inet.queueing.queue.PacketQueue;
import inet.queueing.scheduler.WrrScheduler;

//
// Diffserv Queue used in Experiment 2.1 - 2.4.
//
module DSQueue2 extends CompoundPacketQueueBase
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

