//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.channelaccess;

import inet.queueing.contract.IPacketQueue;
import inet.linklayer.ieee80211.mac.contract.IContention;
import inet.linklayer.ieee80211.mac.originator.QosAckHandler;
import inet.linklayer.ieee80211.mac.originator.QosRecoveryProcedure;
import inet.linklayer.ieee80211.mac.originator.TxopProcedure;
import inet.linklayer.ieee80211.mac.queue.InProgressFrames;

//
// Implements EDCAF (Enhanced Distributed Channel Access Function) for IEEE 802.11.
// EDCAF represents one access category within EDCA.
//
module Edcaf
{
    parameters:
        string rxModule;
        string collisionControllerModule;
        string originatorMacDataServiceModule;
        string accessCategory @enum("AC_BK","AC_BE","AC_VI","AC_VO");
        int aifsn = default(-1);
        int cwMin = default(-1);
        int cwMax = default(-1);
        @class(Edcaf);
        @display("i=block/queue");
        @signal[packetSentToPeer](type=inet::Packet);
        @signal[channelOwnershipChanged](type=bool);
        @statistic[packetSentToPeer](title="packets sent"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetSentToPeerUnicast](title="packets sent: unicast"; source=ieee80211Unicast(packetSentToPeer); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetSentToPeerMulticast](title="packets sent: multicast"; source=ieee80211Multicast(packetSentToPeer); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetSentToPeerBroadcast](title="packets sent: broadcast "; source=ieee80211Broadcast(packetSentToPeer); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetSentToPeerWithRetry](title="packets sent: with retry"; source=ieee80211Retry(packetSentToPeer); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetSentToPeerWithoutRetry](title="packets sent: without retry"; source=ieee80211NotRetry(packetSentToPeer); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[channelOwnershipChanged](title="channel ownership"; record=count,vector?; interpolationmode=sample-hold);
    submodules:
        pendingQueue: <default("PendingQueue")> like IPacketQueue {
            parameters:
                @display("p=100,100");
        }
        inProgressFrames: InProgressFrames {
            parameters:
                pendingQueueModule = default("^.pendingQueue");
                originatorMacDataServiceModule = default("^.^.^.originatorMacDataService");
                ackHandlerModule = default("^.ackHandler");
                @display("p=100,200");
        }
        contention: <default("Contention")> like IContention {
            parameters:
                @display("p=100,300");
        }
        // TODO consider moving QosRecoveryProcedure and QosAckHandler from Edcaf to Hcf, because they seem to be prepared to handle all ACs?
        ackHandler: QosAckHandler {
            parameters:
                @display("p=100,400");
        }
        txopProcedure: TxopProcedure {
            parameters:
                @display("p=100,500");
        }
        // TODO consider moving QosRecoveryProcedure and QosAckHandler from Edcaf to Hcf, because they seem to be prepared to handle all ACs?
        recoveryProcedure: QosRecoveryProcedure {
            parameters:
                cwCalculatorModule = "^";
                rtsPolicyModule = "^.^.^.rtsPolicy";
                @display("p=100,600");
        }
}

