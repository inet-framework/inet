// Hierarchical Token Bucket Implementation for OMNeT++ & INET Framework
// Copyright (C) 2021 Marija Gajić (NTNU), Marcin Bosk (TUM), Susanna Schwarzmann (TU Berlin), Stanislav Lange (NTNU), and Thomas Zinner (NTNU)
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This implementation used the "PriorityQueue" as a base!
//

package inet.queueing.queue;

import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.IPacketScheduler;


module HtbQueue extends CompoundPacketQueueBase
{
    parameters:
        int numQueues;
        bool htbHysterisis = default(false); // Use hysterisis?
        xml htbTreeConfig = default(xml("<config></config>")); // The htb tree structure
    submodules:
        classifier: <default("ContentBasedClassifier")> like IPacketClassifier {
            parameters:
                @display("p=100,100");
        }
        queue[numQueues]: <default("PacketQueue")> like IPacketQueue {
            parameters:
                bufferModule = default(exists(buffer) ? "^.buffer" : "");
                @display("p=325,100,column,125");
        }
        scheduler: <default("HtbScheduler")> like IPacketScheduler {
            parameters:
                htbHysterisis = parent.htbHysterisis; // Use hysterisis?
                htbTreeConfig = default(parent.htbTreeConfig);
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

