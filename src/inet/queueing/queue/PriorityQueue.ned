//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.queue;

import inet.queueing.contract.IPacketBuffer;
import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.IPacketScheduler;

//
// Priority queue that consists of multiple internal queues with different priority
// levels and an optional shared memory buffer. It provides a comprehensive solution
// for Quality of Service (QoS) and traffic prioritization in network devices.
//
// The PriorityQueue operates by:
// 1. Using a classifier to direct incoming packets to different internal queues
//    based on packet attributes (e.g., TOS field, DSCP value, or user priority)
// 2. Maintaining separate queues for different priority levels
// 3. Using a priority scheduler to select packets from the highest priority
//    non-empty queue when pulling packets
// 4. Optionally sharing a common buffer among all queues to optimize memory usage
//
// The module consists of the following components:
// - Classifier: Determines which internal queue a packet should be directed to
// - Multiple internal queues: Store packets of different priority levels
// - Scheduler: Selects which queue to pull packets from (typically highest priority first)
// - Optional shared buffer: Manages memory allocation across all queues
//
// This queue is commonly used in:
// - Network routers and switches implementing QoS mechanisms
// - Traffic management systems that need to prioritize certain flows
// - Systems implementing Differentiated Services (DiffServ) architecture
// - Any network device that needs to provide preferential treatment to specific traffic types
//
// The default implementation uses PacketClassifier and PriorityScheduler, but these
// can be customized through parameters to implement different classification and
// scheduling strategies.
//
// @see ~PacketClassifier, ~PriorityScheduler, ~IPacketBuffer
//
module PriorityQueue extends CompoundPacketQueueBase
{
    parameters:
        int numQueues;
    submodules:
        buffer: <default("")> like IPacketBuffer if typename != "" {
            parameters:
                @display("p=100,225");
        }
        classifier: <default("PacketClassifier")> like IPacketClassifier {
            parameters:
                @display("p=100,100");
        }
        queue[numQueues]: <default("PacketQueue")> like IPacketQueue {
            parameters:
                bufferModule = default(exists(parent.buffer) ? "^.buffer" : "");
                @display("p=300,100,column,125");
        }
        scheduler: <default("PriorityScheduler")> like IPacketScheduler {
            parameters:
                @display("p=500,100");
        }
    connections:
        in --> { @display("m=w"); } --> classifier.in;
        for i=0..sizeof(queue)-1 {
            classifier.out++ --> queue[i].in;
            queue[i].out --> scheduler.in++;
        }
        scheduler.out --> { @display("m=e"); } --> out;
}
