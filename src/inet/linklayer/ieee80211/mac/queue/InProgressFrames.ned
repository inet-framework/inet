//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.queue;

import inet.common.SimpleModule;

//# TODO rename to `InProgressQueue`?
simple InProgressFrames extends SimpleModule
{
    parameters:
        @class(InProgressFrames);
        string pendingQueueModule;
        string originatorMacDataServiceModule;
        string ackHandlerModule;
        @display("i=block/queue");
        @signal[packetEnqueued](type=inet::Packet);
        @signal[packetDequeued](type=inet::Packet);
        @statistic[queueingTime](title="queueing times"; source=queueingTime(packetDequeued); record=histogram,vector; interpolationmode=none);
        @statistic[queueLength](title="queue length"; source=warmup(count(packetEnqueued) - count(packetDequeued)); record=max,timeavg,vector; interpolationmode=sample-hold; autoWarmupFilter=false);
}

