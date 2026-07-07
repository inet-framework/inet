//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.queue;

import inet.queueing.base.PacketQueueBase;
import inet.queueing.contract.IPacketQueue;

//
// A per-station airtime deficit round-robin (DRR) transmit queue for IEEE 802.11,
// modelled on the Linux `mac80211` airtime-fairness feature. Keeps one FIFO sub-queue
// per receiver (destination MAC address) and hands frames to the MAC so that, over time,
// each backlogged station gets an equal share of *on-air time* (rather than an equal
// share of frames). This fixes the downlink form of the 802.11 rate anomaly: it stops a
// slow client's long frames from dragging fast clients down to its throughput.
//
// The airtime deficit of each station is charged a-posteriori from the actual on-air
// duration of its transmitted frames (retries included), reported by the coordination
// function via the `frameTransmittedAirtime` signal. A station may transmit while its
// deficit is non-negative; an ineligible backlogged station is topped up by
// `quantum * weight` when the round-robin cursor reaches it.
//
// Drops into the `pendingQueue` slot of an ~Edcaf or ~Dcaf via a typename override, e.g.:
//   *.host.wlan[*].mac.dcf.channelAccess.pendingQueue.typename = "AirtimeFairnessQueue"
//
// @see ~PendingQueue, ~CompoundPendingQueue
//
simple AirtimeFairnessQueue extends PacketQueueBase like IPacketQueue
{
    parameters:
        int packetCapacity = default(-1); // maximum total number of packets across all stations, no limit by default
        double quantum @unit(s) = default(1500us); // base DRR quantum in airtime units; ~one max-length frame's airtime
        double weight = default(1); // per-station airtime weight; equal weight means equal airtime share
        bool fairnessEnabled = default(true); // when false, degrades to a plain FIFO passthrough (for OFF/ON contrast)
        displayStringTextFormat = default("contains %p pk (%l)\npushed %u pulled %o dropped %d");
        @class(AirtimeFairnessQueue);
        @signal[packetPushStarted](type=inet::Packet);
        @signal[packetPushEnded](type=inet::Packet?);
        @signal[packetPulled](type=inet::Packet);
        @signal[packetRemoved](type=inet::Packet);
        @signal[packetDropped](type=inet::Packet);
        // the statistical value is the number of packets in the queue
        @statistic[queueLength](title="queue length"; source=warmup(atomic(constant0(packetPushEnded) + count(packetPushStarted) - count(packetPulled) - count(packetRemoved) - count(packetDropped))); record=last,max,timeavg,vector; unit=pk; interpolationmode=sample-hold; autoWarmupFilter=false);
        // the statistical value is the queueing time of packets
        @statistic[queueingTime](title="queueing times"; source=queueingTime(packetPulled); record=histogram,vector; unit=s; interpolationmode=none);
        // the statistical value is the packet that is dropped due to queue overflow
        @statistic[droppedPacketsQueueOverflow](title="dropped packets: queue overflow"; source=packetDropReasonIsQueueOverflow(packetDropped); record=count; unit=pk; interpolationmode=none);
        @defaultStatistic(queueLength:vector);
}
