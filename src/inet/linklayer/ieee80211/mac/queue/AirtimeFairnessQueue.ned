//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.queue;

import inet.queueing.classifier.DynamicClassifier;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.queue.CompoundPacketQueueBase;

//
// A per-station airtime deficit round-robin (DRR) transmit queue for IEEE 802.11, modelled
// on the Linux `mac80211` airtime-fairness feature. It is built as a compound queue: a
// classifier routes each frame to a per-receiver sub-queue, each sub-queue is followed by a
// per-station airtime gate, and a deficit-round-robin scheduler serves the gates so that,
// over time, every backlogged station gets an equal share of on-air *time* (scaled by an
// optional per-station weight) rather than an equal share of frames.
//
// The per-station branches (sub-queue + gate) are created on demand as new receivers
// appear, so the queue adapts to the dynamic set of stations an access point serves. The
// sub-queue type is configurable via `subqueueTypename`.
//
// This fixes the downlink form of the 802.11 rate anomaly: when an access point saturates a
// mix of fast and slow clients, a plain FIFO (or frame-fair round robin) lets the slow
// client's long frames drag the fast clients down to its throughput; airtime fairness holds
// the slow client to its fair *time* share and lets the fast clients recover.
//
// Each station's airtime deficit is charged a-posteriori from the actual on-air duration of
// its transmitted frames (retries included), reported by the coordination function via the
// `frameTransmittedAirtime` signal. A shared `packetCapacity` is enforced across all
// stations with a drop-from-longest overflow policy (see ~Ieee80211LongestFlowDropper), so a
// slow station's backlog cannot lock the others out. With `fairnessEnabled = false` the
// queue degrades to a plain per-station round robin (the frame-fair anomaly baseline).
//
// Drops into the `pendingQueue` slot of an ~Edcaf or ~Dcaf via a typename override, e.g.:
//   *.host.wlan[*].mac.dcf.channelAccess.pendingQueue.typename = "AirtimeFairnessQueue"
//
// @see ~DynamicClassifier, ~AirtimeFairnessGate, ~AirtimeFairnessScheduler,
//      ~Ieee80211LongestFlowDropper, ~PendingQueue
//
module AirtimeFairnessQueue extends CompoundPacketQueueBase
{
    parameters:
        @class(AirtimeFairnessCompoundQueue);
        double quantum @unit(s) = default(1500us); // base DRR quantum in airtime units; ~one max-length frame's airtime
        double weight = default(1); // per-station airtime weight; equal weight means equal airtime share
        bool fairnessEnabled = default(true); // when false, degrades to a plain per-station round robin (for OFF/ON contrast)
        string subqueueTypename = default("inet.queueing.queue.PacketQueue"); // NED type of each per-station sub-queue
        packetCapacity = default(100); // shared across all per-station sub-queues; same limit as ~PendingQueue, which this replaces
        dropperClass = default("inet::ieee80211::Ieee80211LongestFlowDropper"); // on overflow, drop the tail frame of the longest per-station backlog
        displayStringTextFormat = default("{numStations} stations\ncontains %p pk (%l) dropped %d");
        scheduler.fairnessEnabled = default(this.fairnessEnabled);
    submodules:
        // Generic dynamic classifier: routes by receiver MAC (Ieee80211ReceiverAddressClassifier)
        // and creates a PerStationAirtimeQueue (queue -> gate) for each new station as branch[k],
        // wiring it into the scheduler.
        classifier: DynamicClassifier {
            classifierClass = default("inet::ieee80211::Ieee80211ReceiverAddressClassifier");
            moduleType = default("inet.linklayer.ieee80211.mac.queue.PerStationAirtimeQueue");
            submoduleName = default("branch");
            aggregatorSubmoduleName = default("scheduler");
            @display("p=100,150");
        }
        // Grown on demand, one sub-queue and airtime gate per receiver. The branch is created
        // with its final name and index, so a station can also be configured individually, as
        // in branch[2].weight = 2.
        branch[0]: PerStationAirtimeQueue {
            quantum = default(parent.quantum);
            weight = default(parent.weight);
            fairnessEnabled = default(parent.fairnessEnabled);
            subqueueTypename = default(parent.subqueueTypename);
            @display("p=400,150,column,80");
        }
        scheduler: AirtimeFairnessScheduler {
            @display("p=700,150");
        }
    connections allowunconnected:
        in --> { @display("m=w"); } --> classifier.in;
        scheduler.out --> { @display("m=e"); } --> out;
}
