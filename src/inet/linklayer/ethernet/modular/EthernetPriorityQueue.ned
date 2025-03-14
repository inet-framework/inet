//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.queueing.queue.PriorityQueue;

//
// Implements an Ethernet-specific priority queue that classifies and prioritizes
// packets based on their IEEE 802.1Q priority tags. Unlike EthernetGatingQueue,
// this implementation does not include time-aware gates for each priority level,
// making it suitable for applications that require priority-based forwarding
// without the need for time-synchronized transmission control.
//
// The module uses PacketUserPriorityIndClassifier to map incoming packets to
// appropriate priority queues based on the UserPriorityInd tag, which corresponds
// to the IEEE 802.1Q priority field. The reverseOrder parameter is set to true
// by default, meaning that higher priority values receive preferential treatment
// (consistent with IEEE 802.1Q priority conventions where 7 is highest priority).
//
// @see ~EthernetGatingQueue, ~PriorityQueue
//
module EthernetPriorityQueue extends PriorityQueue
{
    parameters:
        classifier.reverseOrder = default(true);
        classifier.classifierClass = default("inet::queueing::PacketUserPriorityIndClassifier");
}
