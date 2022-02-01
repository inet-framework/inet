//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.queue;

import inet.queueing.classifier.PacketClassifier;
import inet.queueing.queue.CompoundPacketQueueBase;
import inet.queueing.queue.PacketQueue;
import inet.queueing.scheduler.PriorityScheduler;

module CompoundPendingQueue extends CompoundPacketQueueBase
{
    parameters:
        packetCapacity = default(100); // -1 means unlimited
    submodules:
        classifier: PacketClassifier {
            parameters:
                classifierClass = default("inet::ieee80211::MgmtOverDataClassifier"); // MgmtOverDataClassifier, MgmtOverMulticastOverUnicastClassifier, ...
                @display("p=100,200");
        }
        managementQueue: PacketQueue {
            parameters:
                @display("p=300,100");
        }
        multicastQueue: PacketQueue {
            parameters:
                @display("p=300,200");
        }
        unicastQueue: PacketQueue {
            parameters:
                @display("p=300,300");
        }
        prioritizer: PriorityScheduler {
            parameters:
                @display("p=500,200");
        }
    connections:
        in --> classifier.in;
        classifier.out++ --> managementQueue.in;
        classifier.out++ --> multicastQueue.in;
        classifier.out++ --> unicastQueue.in;
        managementQueue.out --> prioritizer.in++;
        multicastQueue.out --> prioritizer.in++;
        unicastQueue.out --> prioritizer.in++;
        prioritizer.out --> out;
}

