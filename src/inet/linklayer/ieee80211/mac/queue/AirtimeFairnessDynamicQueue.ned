//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.queue;

import inet.queueing.contract.IPacketQueue;
import inet.queueing.queue.CompoundPacketQueueBase;

//
// TEMPORARY validation scaffold: the on-demand (dynamic) form of the airtime-fairness
// compound queue, wired like a ~GatingPriorityQueue but with the per-station sub-queue and
// gate branches created on demand as new receivers appear -- the classifier grows the
// `queue`/`gate` vectors and the scheduler's inputs at runtime (see
// ~AirtimeFairnessClassifier). Used to validate the dynamic branch creation before it is
// folded into the final ~AirtimeFairnessQueue that replaces the monolithic module.
//
module AirtimeFairnessDynamicQueue extends CompoundPacketQueueBase
{
    parameters:
        @class(AirtimeFairnessCompoundQueue);
        double quantum @unit(s) = default(1500us);
        double weight = default(1);
        bool fairnessEnabled = default(true);
        string subqueueTypename = default("inet.queueing.queue.PacketQueue"); // type of each per-station sub-queue
        dropperClass = default("inet::ieee80211::Ieee80211LongestFlowDropper"); // on overflow, drop the tail frame of the longest per-station backlog
        classifier.subqueueModuleType = default(this.subqueueTypename);
        gate[*].quantum = default(this.quantum);
        gate[*].weight = default(this.weight);
        gate[*].fairnessEnabled = default(this.fairnessEnabled);
        scheduler.fairnessEnabled = default(this.fairnessEnabled);
    submodules:
        classifier: AirtimeFairnessClassifier {
            @display("p=100,150");
        }
        queue[0]: <default("PacketQueue")> like IPacketQueue {
            @display("p=300,150,column,80");
        }
        gate[0]: AirtimeFairnessGate {
            @display("p=500,150,column,80");
        }
        scheduler: AirtimeFairnessScheduler {
            @display("p=700,150");
        }
    connections allowunconnected:
        in --> { @display("m=w"); } --> classifier.in;
        scheduler.out --> { @display("m=e"); } --> out;
}
