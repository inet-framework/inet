//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.queue;

//
// Manages IEEE 802.11 frames that are currently being processed by the MAC layer. This module 
// tracks frames that have been dequeued from the pending queue but have not yet been successfully 
// transmitted or acknowledged. It works in conjunction with the originator MAC data service for 
// frame processing and the ACK handler for tracking acknowledgment status. The module maintains 
// statistics on queueing time and queue length, providing insights into MAC layer performance 
// and potential bottlenecks. It signals events when packets are enqueued or dequeued, allowing 
// other modules to react to these state changes.
//
// @see ~PendingQueue, ~CompoundPendingQueue
//
simple InProgressFrames
{
    parameters:
        string pendingQueueModule;
        string originatorMacDataServiceModule;
        string ackHandlerModule;
        @display("i=block/queue");
        @signal[packetEnqueued](type=inet::Packet);
        @signal[packetDequeued](type=inet::Packet);
        @statistic[queueingTime](title="queueing times"; source=queueingTime(packetDequeued); record=histogram,vector; interpolationmode=none);
        @statistic[queueLength](title="queue length"; source=warmup(count(packetEnqueued) - count(packetDequeued)); record=max,timeavg,vector; interpolationmode=sample-hold; autoWarmupFilter=false);
}
