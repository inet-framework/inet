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
// per receiver on demand, as `branch[k]`, and the ~AirtimeFairnessScheduler finds the gate of
// each branch at the far end of the connection into its own input gate. The
// `quantum`/`weight`/`fairnessEnabled`/`subqueueTypename` parameters default to those of the
// enclosing ~AirtimeFairnessQueue, and may also be set per station, as in `branch[2].weight`.
//
module PerStationAirtimeQueue like IPacketQueue
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
        input in @labels(push);
        output out @labels(pull);
    submodules:
        queue: <subqueueTypename> like IPacketQueue;
        gate: AirtimeFairnessGate;
    connections:
        in --> queue.in;
        queue.out --> gate.in;
        gate.out --> out;
}
