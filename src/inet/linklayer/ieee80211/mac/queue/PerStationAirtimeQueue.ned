//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.queue;

import inet.queueing.contract.IPacketQueue;

//
// One per-station branch of the ~AirtimeFairnessQueue: a FIFO sub-queue followed by an
// ~AirtimeFairnessGate. ~AirtimeFairnessQueue's ~DynamicClassifier instantiates one of these
// per receiver on demand and, with `spliceBranchSubmodules`, splices the inner `queue`/`gate`
// directly into the queue as `queue[k]`/`gate[k]` so the ~AirtimeFairnessScheduler can address
// the gate directly. The `quantum`/`weight`/`fairnessEnabled`/`subqueueTypename` parameters are
// forwarded from the enclosing ~AirtimeFairnessQueue by the classifier (matching parameter
// names), since the spliced submodules are finalized in this compound's parameter context.
//
module PerStationAirtimeQueue
{
    parameters:
        double quantum @unit(s) = default(1500us);
        double weight = default(1);
        bool fairnessEnabled = default(true);
        string subqueueTypename = default("inet.queueing.queue.PacketQueue");
        gate.quantum = default(this.quantum);
        gate.weight = default(this.weight);
        gate.fairnessEnabled = default(this.fairnessEnabled);
    gates:
        input in;
        output out;
    submodules:
        queue: <subqueueTypename> like IPacketQueue;
        gate: AirtimeFairnessGate;
    connections:
        in --> queue.in;
        queue.out --> gate.in;
        gate.out --> out;
}
