//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ethernet.modular;

import inet.queueing.queue.PriorityQueue;

//
// Implements a priority queue for Ethernet frames based on user priority
// indication tags. Classifies incoming packets using the
// PacketUserPriorityIndClassifier, which extracts priority values from
// UserPriorityInd tags. By default, uses reverse ordering so that higher
// numerical priority values receive higher priority treatment. Inherits the
// multi-queue structure and optional shared buffer from PriorityQueue.
//
module EthernetPriorityQueue extends PriorityQueue
{
    parameters:
        classifier.reverseOrder = default(true);
        classifier.classifierClass = default("inet::queueing::PacketUserPriorityIndClassifier");
}
