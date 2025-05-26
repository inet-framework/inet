//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.queue;

import inet.queueing.queue.DropTailQueue;

//
// Implements a priority queue for IEEE 802.11 MAC frames. Uses a comparator to
// determine the order in which frames are processed. By default, prioritizes
// management frames over data frames, but can be configured to use different
// prioritization schemes such as management over multicast over unicast frames.
//
// @see CompoundPendingQueue
//
simple PendingQueue extends DropTailQueue
{
    parameters:
        comparatorClass = default("inet::ieee80211::MgmtOverDataComparator"); // MgmtOverDataComparator, MgmtOverMulticastOverUnicastComparator, ...
        packetCapacity = default(100);
}

