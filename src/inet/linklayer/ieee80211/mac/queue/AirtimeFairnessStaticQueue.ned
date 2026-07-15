//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.queue;

import inet.queueing.contract.IPacketQueue;
import inet.queueing.queue.CompoundPacketQueueBase;

//
// TEMPORARY validation scaffold for the airtime-fairness compound queue: a fixed-size
// (numQueues) variant of the eventual ~AirtimeFairnessQueue, wired like a
// ~GatingPriorityQueue -- classifier -> per-station sub-queue -> per-station gate ->
// scheduler. Used to validate ~AirtimeFairnessGate + ~AirtimeFairnessScheduler against
// the frozen monolithic behaviour before the on-demand (dynamic) branch creation is added.
// This module is superseded by the dynamic ~AirtimeFairnessQueue and will be removed.
//
module AirtimeFairnessStaticQueue extends CompoundPacketQueueBase
{
    parameters:
        int numQueues = default(8); // fixed number of per-station branches (upper bound on stations)
        double quantum @unit(s) = default(1500us);
        double weight = default(1);
        bool fairnessEnabled = default(true);
        gate[*].quantum = default(this.quantum);
        gate[*].weight = default(this.weight);
        gate[*].fairnessEnabled = default(this.fairnessEnabled);
        scheduler.fairnessEnabled = default(this.fairnessEnabled);
    submodules:
        classifier: AirtimeFairnessClassifier {
            @display("p=100,150");
        }
        queue[numQueues]: <default("PacketQueue")> like IPacketQueue {
            @display("p=300,150,column,80");
        }
        gate[numQueues]: AirtimeFairnessGate {
            @display("p=500,150,column,80");
        }
        scheduler: AirtimeFairnessScheduler {
            @display("p=700,150");
        }
    connections:
        in --> { @display("m=w"); } --> classifier.in;
        for i=0..numQueues-1 {
            classifier.out++ --> queue[i].in;
            queue[i].out --> gate[i].in;
            gate[i].out --> scheduler.in++;
        }
        scheduler.out --> { @display("m=e"); } --> out;
}
