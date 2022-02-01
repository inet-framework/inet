//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.queueing.queue.PriorityQueue;

module EthernetPriorityQueue extends PriorityQueue
{
    parameters:
        classifier.reverseOrder = default(true);
        classifier.classifierClass = default("inet::queueing::PacketUserPriorityIndClassifier");
}
