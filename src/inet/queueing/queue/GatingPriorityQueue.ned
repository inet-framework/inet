//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.queue;

import inet.queueing.contract.IPacketBuffer;
import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketGate;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.IPacketScheduler;

//
// Priority queue with multiple inner queues each having its own periodic gate
// for packet selection and an optional shared memory buffer. This module combines
// the functionality of PriorityQueue and GatingQueue to provide both traffic
// prioritization and time-based transmission control.
//
// The GatingPriorityQueue operates by:
// 1. Classifying incoming packets into different priority queues
// 2. Controlling when packets from each queue can be transmitted using individual gates
// 3. Using a priority scheduler to select packets from the highest priority
//    non-empty queue when multiple gates are open
//
// The bitrate parameter specifies the transmission rate, and the extraLength
// parameter accounts for additional overhead (e.g., Ethernet PHY header).
//
// @see ~PriorityQueue, ~GatingQueue, ~PeriodicGate, ~PriorityScheduler
//
module GatingPriorityQueue extends CompoundPacketQueueBase
{
    parameters:
        int numQueues; // Number of priority queues to create
        double bitrate @unit(bps); // Transmission rate when gates are open
        *.bitrate = default(this.bitrate); // Propagate bitrate to submodules
        *.extraLength = default(64b); // Overhead for Ethernet PHY header (preamble, SFD, IFG)
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
        gate[numQueues]: <default("PeriodicGate")> like IPacketGate {
            parameters:
                @display("p=500,100,column,125");
        }
        scheduler: <default("PriorityScheduler")> like IPacketScheduler {
            parameters:
                @display("p=700,100");
        }
    connections:
        in --> { @display("m=w"); } --> classifier.in;
        for i=0..sizeof(queue)-1 {
            classifier.out++ --> queue[i].in;
            queue[i].out --> gate[i].in;
            gate[i].out --> scheduler.in++;
        }
        scheduler.out --> { @display("m=e"); } --> out;
}
