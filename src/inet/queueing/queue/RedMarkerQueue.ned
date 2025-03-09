//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.queue;

import inet.queueing.contract.IPacketQueue;
import inet.queueing.marker.RedMarker;

//
// Queue with Random Early Detection (RED) and Explicit Congestion Notification (ECN)
// active queue management. It combines a standard packet queue with a RED marker
// to provide congestion avoidance and control through ECN marking rather than
// packet dropping.
//
// This implementation uses RED algorithm to detect congestion but instead of
// dropping packets, it marks them with ECN bits in the IP header. This approach:
// - Provides congestion feedback to ECN-capable endpoints without packet loss
// - Reduces latency by avoiding retransmissions
// - Improves throughput in congested networks
// - Maintains the benefits of RED's early congestion detection
//
// The RED-ECN mechanism operates by:
// 1. Calculating the average queue size using an exponential weighted moving average
// 2. If the average queue size is below a minimum threshold, no packets are marked
// 3. If the average queue size is between minimum and maximum thresholds, packets are
//    marked with ECN with a probability that increases linearly with the average queue size
// 4. If the average queue size exceeds the maximum threshold, all incoming packets are marked
//
// This implementation uses a RedMarker module connected to a standard packet queue.
// The RedMarker handles the RED algorithm logic and ECN marking, while the queue
// (by default a PacketQueue) stores all packets.
//
// RED-ECN queues are particularly useful in:
// - Data center networks where low latency is critical
// - Internet routers supporting ECN
// - Networks with ECN-capable endpoints
// - Any network where congestion control without packet loss is desirable
//
// Note: For this mechanism to be effective, both endpoints must support ECN,
// and the network path must preserve ECN markings.
//
// @see ~RedMarker, ~RedDropperQueue, ~PacketQueue, ~EcnMarker
//
module RedMarkerQueue extends CompoundPacketQueueBase
{
    parameters:
        @display("i=block/queue");

    submodules:
        redMarker: RedMarker {
            @display("p=100,100");
        }
        fifo: <default("PacketQueue")> like IPacketQueue {
            @display("p=100,200");
        }
    connections:
        in --> { @display("m=n"); } --> redMarker.in;
        redMarker.out --> fifo.in;
        fifo.out --> { @display("m=s"); } --> out;
}
