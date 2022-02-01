//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.shaper;

import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketScheduler;
import inet.queueing.contract.IPacketShaper;
import inet.queueing.queue.CompoundPacketQueueBase;

//
// This module combines a classifier, a scheduler and several traffic shapers
// into a single packet shaper. Packets are classified into one of the traffic
// shapers and the scheduler prioritizes among them.
//
module PriorityShaper extends CompoundPacketQueueBase like IPacketShaper
{
    parameters:
        int numShapers;
    submodules:
        classifier: <default("PacketClassifier")> like IPacketClassifier {
            @display("p=100,150");
        }
        shaper[numShapers]: <default("PacketShaper")> like IPacketShaper {
            @display("p=300,150,column,150");
        }
        scheduler: <default("PriorityScheduler")> like IPacketScheduler {
            @display("p=500,150");
        }
    connections:
        in --> classifier.in;
        for i=0..numShapers-1 {
            classifier.out++ --> shaper[i].in;
            shaper[i].out --> scheduler.in++;
        }
        scheduler.out --> out;
}
