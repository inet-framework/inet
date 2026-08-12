//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.queue;

import inet.common.Module;
import inet.queueing.contract.IPacketQueue;

//
// One per-station branch of the ~AirtimeFairnessQueue: a FIFO sub-queue followed by an
// ~AirtimeFairnessGate. ~AirtimeFairnessQueue's ~DynamicClassifier instantiates one of these
// per receiver on demand, as `branch[k]`, and the ~AirtimeFairnessScheduler finds the gate of
// each branch at the far end of the connection into its own input gate. The
// `quantum`/`weight`/`fairnessEnabled`/`subqueueTypename` parameters default to those of the
// enclosing ~AirtimeFairnessQueue, and may also be set per station, as in `branch[2].weight`.
//
module PerStationAirtimeQueue extends Module like IPacketQueue
{
    parameters:
        // the state of the station, which was visible on the sub-queue and the gate directly
        // while the branch was flattened into the queue: the airtime deficit held by the
        // gate, and the frames waiting in the sub-queue; the leading dot makes the submodule
        // references relative (a bare `gate.deficit` would be looked up as an absolute path)
        displayStringTextFormat = default("deficit {.gate.deficit}\ncontains {.queue.numPackets} pk ({.queue.totalLength})");
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
