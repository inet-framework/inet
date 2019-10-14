//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
// 

package inet.linklayer.ieee80211.mac.channelaccess;

import inet.queueing.contract.IPacketQueue;
import inet.linklayer.ieee80211.mac.contract.IContention;
import inet.linklayer.ieee80211.mac.queue.InProgressFrames;

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

