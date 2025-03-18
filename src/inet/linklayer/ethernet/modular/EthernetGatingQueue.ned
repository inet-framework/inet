//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ethernet.modular;

import inet.queueing.queue.GatingPriorityQueue;

//
// Implements a priority queue for Ethernet frames with multiple inner queues
// each having its own periodic gate for packet selection. Uses the
// PacketUserPriorityIndClassifier to classify packets based on their
// UserPriorityInd tag, supporting IEEE 802.1Q traffic prioritization and
// Time-Sensitive Networking (TSN) features through gating.
//
module EthernetGatingQueue extends GatingPriorityQueue
{
    parameters:
        classifier.classifierClass = default("inet::queueing::PacketUserPriorityIndClassifier");
}
