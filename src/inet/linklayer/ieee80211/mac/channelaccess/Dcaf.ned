//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.channelaccess;

import inet.queueing.contract.IPacketQueue;
import inet.linklayer.ieee80211.mac.contract.IContention;
import inet.linklayer.ieee80211.mac.queue.InProgressFrames;

//
// Implements the DCAF (Distributed Channel Access Function) for IEEE 802.11.
//
module Dcaf
{
    parameters:
        int difsn = default(-1);
        int cwMin = default(-1);
        int cwMax = default(-1);
        string rxModule;
        @class("Dcaf");
        @display("i=block/star");
        @signal[channelOwnershipChanged](type=bool);
        @statistic[channelOwnershipChanged](title="channel ownership"; record=count,vector?; interpolationmode=sample-hold);
    submodules:
        pendingQueue: <default("PendingQueue")> like IPacketQueue {
            parameters:
                @display("p=100,100");
        }
        inProgressFrames: InProgressFrames {
            parameters:
                pendingQueueModule = default("^.pendingQueue");
                originatorMacDataServiceModule = default("^.^.originatorMacDataService");
                ackHandlerModule = default("^.^.ackHandler");
                @display("p=100,200");
        }
        contention: <default("Contention")> like IContention {
            parameters:
                @display("p=100,300");
        }
}

