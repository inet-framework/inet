//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.queueing.queue.GatingPriorityQueue;

//
// Implements an Ethernet-specific gating priority queue for Time-Sensitive Networking
// (TSN) applications according to IEEE 802.1Q. Classifies incoming packets based on
// their user priority indication tags and directs them to appropriate subqueues,
// each controlled by a gate that can be opened and closed according to a schedule.
//
// The gating mechanism enables time-aware shaping, a key TSN feature that provides
// deterministic latency for time-critical traffic by controlling precisely when
// packets from different priority queues can be transmitted. This implementation
// uses PacketUserPriorityIndClassifier to map packets to priority queues based
// on the UserPriorityInd tag, which corresponds to the IEEE 802.1Q priority field.
//
// @see ~EthernetPriorityQueue, ~GatingPriorityQueue, ~Ieee8021qTimeAwareShaper
//
module EthernetGatingQueue extends GatingPriorityQueue
{
    parameters:
        classifier.classifierClass = default("inet::queueing::PacketUserPriorityIndClassifier");
}
