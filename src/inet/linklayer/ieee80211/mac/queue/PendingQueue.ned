//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.queue;

import inet.queueing.queue.DropTailQueue;

simple PendingQueue extends DropTailQueue
{
    parameters:
        comparatorClass = default("inet::ieee80211::MgmtOverDataComparator"); // MgmtOverDataComparator, MgmtOverMulticastOverUnicastComparator, ...
        packetCapacity = default(100);
}

