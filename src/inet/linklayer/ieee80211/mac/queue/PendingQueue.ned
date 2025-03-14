//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.queue;

import inet.queueing.queue.DropTailQueue;

//
// Implements a priority queue for IEEE 802.11 MAC frames awaiting transmission. This module 
// extends the basic DropTailQueue with frame prioritization capabilities using a comparator 
// class. By default, it uses the MgmtOverDataComparator which prioritizes management frames 
// over data frames, ensuring that control operations take precedence over data transmission. 
// Alternative comparators like MgmtOverMulticastOverUnicastComparator can be configured to 
// implement different prioritization schemes. The queue has a default capacity of 100 packets 
// and drops packets from the tail when this limit is reached.
//
// @see ~CompoundPendingQueue, ~InProgressFrames
//
simple PendingQueue extends DropTailQueue
{
    parameters:
        comparatorClass = default("inet::ieee80211::MgmtOverDataComparator"); // MgmtOverDataComparator, MgmtOverMulticastOverUnicastComparator, ...
        packetCapacity = default(100);
}
