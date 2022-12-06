//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.coordinationfunction;

import inet.linklayer.ieee80211.mac.channelaccess.Dcaf;
import inet.linklayer.ieee80211.mac.contract.ICtsPolicy;
import inet.linklayer.ieee80211.mac.contract.IDcf;
import inet.linklayer.ieee80211.mac.contract.IOriginatorAckPolicy;
import inet.linklayer.ieee80211.mac.contract.IRateControl;
import inet.linklayer.ieee80211.mac.contract.IRecipientAckPolicy;
import inet.linklayer.ieee80211.mac.contract.IRtsPolicy;
import inet.linklayer.ieee80211.mac.originator.AckHandler;
import inet.linklayer.ieee80211.mac.originator.NonQosRecoveryProcedure;
import inet.linklayer.ieee80211.mac.originator.OriginatorMacDataService;
import inet.linklayer.ieee80211.mac.protectionmechanism.OriginatorProtectionMechanism;
import inet.linklayer.ieee80211.mac.rateselection.RateSelection;
import inet.linklayer.ieee80211.mac.recipient.RecipientMacDataService;


//
// Implements the DCF (Distributed Coordination Function) for IEEE 802.11.
//
module Dcf like IDcf
{
    parameters:
        string rxModule;
        string txModule;

        *.rateSelectionModule = "^.rateSelection";
        *.rxModule = "^." + this.rxModule;

        @class(Dcf);
        @display("i=block/layer");
        @signal[packetSentToPeer](type=inet::Packet);
        @signal[packetReceivedFromPeer](type=inet::Packet);
        @signal[linkBroken](type=inet::Packet);
        @signal[packetDropped](type=inet::Packet);
        @signal[frameSequenceStarted];
        @signal[frameSequenceFinished];
        @signal[datarateSelected](type=double);
        @statistic[packetSentToPeer](title="packets sent"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetSentToPeerUnicast](title="packets sent: unicast"; source=ieee80211Unicast(packetSentToPeer); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetSentToPeerMulticast](title="packets sent: multicast"; source=ieee80211Multicast(packetSentToPeer); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetSentToPeerBroadcast](title="packets sent: broadcast "; source=ieee80211Broadcast(packetSentToPeer); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetSentToPeerWithRetry](title="packets sent: with retry"; source=ieee80211Retry(packetSentToPeer); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetSentToPeerWithoutRetry](title="packets sent: without retry"; source=ieee80211NotRetry(packetSentToPeer); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetReceivedFromPeer](title="packets received"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetReceivedFromPeerUnicast](title="packets received: unicast"; source=ieee80211Unicast(packetReceivedFromPeer); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetReceivedFromPeerMulticast](title="packets received: multicast"; source=ieee80211Multicast(packetReceivedFromPeer); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetReceivedFromPeerBroadcast](title="packets received: broadcast"; source=ieee80211Broadcast(packetReceivedFromPeer); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetReceivedFromPeerWithRetry](title="packets received: with retry"; source=ieee80211Retry(packetReceivedFromPeer); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetReceivedFromPeerWithoutRetry](title="packets received: without retry"; source=ieee80211NotRetry(packetReceivedFromPeer); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[linkBroken](title="link breaks"; record=count,vector?; interpolationmode=none);
        @statistic[packetDrop](title="packet drops"; source=packetDropped; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropNotAddressedToUs](title="packet drops: not addressed to us"; source=packetDropReasonIsNotAddressedToUs(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropQueueOverflow](title="packet drops: queue overflow"; source=packetDropReasonIsQueueOverflow(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropRetryLimitReached](title="packet drops: retry limit reached"; source=packetDropReasonIsRetryLimitReached(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[frameSequenceFinished](title="finished frame sequences"; record=count);
        @statistic[frameSequenceDuration](title="frame sequence durations"; source=frameSequenceDuration(frameSequenceFinished); record=histogram,vector?);
        @statistic[frameSequenceNumPackets](title="frame sequences: number of packets"; source=frameSequenceNumPackets(frameSequenceFinished); record=histogram,vector?);
        @statistic[frameSequenceActive](title="frame sequence active"; source=warmup(count(frameSequenceStarted)-count(frameSequenceFinished)); record=vector; interpolationmode=sample-hold; autoWarmupFilter=false);
        @statistic[datarateSelected](title="datarates selected"; record=vector; interpolationmode=none);

    submodules:
        // TODO consider merging Dcaf into Dcf, because its not like the relationship between Hcf and Edcaf or
        // TODO maybe move PendingQueue and InProgressFrames in Edcaf?
        channelAccess: Dcaf {
            parameters:
                @display("p=150,100");
        }
        originatorMacDataService: OriginatorMacDataService {
            parameters:
                @display("p=150,200");
        }
        recipientMacDataService: RecipientMacDataService {
            parameters:
                @display("p=150,300");
        }
        rateSelection: RateSelection {
            parameters:
                rateControlModule = "^.rateControl";
                @display("p=350,100");
        }
        rateControl: <default("")> like IRateControl if typename != "" {
            parameters:
                @display("p=350,200");
        }
        recoveryProcedure: NonQosRecoveryProcedure {
            parameters:
                cwCalculatorModule = "^.channelAccess";
                rtsPolicyModule = "^.rtsPolicy";
                @display("p=350,300");
        }
        originatorProtectionMechanism: OriginatorProtectionMechanism {
            parameters:
                @display("p=150,400");
        }
        ackHandler: AckHandler {
            parameters:
                @display("p=350,400");
        }
        originatorAckPolicy: <default("OriginatorAckPolicy")> like IOriginatorAckPolicy {
            parameters:
                @display("p=550,100");
        }
        recipientAckPolicy: <default("RecipientAckPolicy")> like IRecipientAckPolicy {
            parameters:
                @display("p=550,200");
        }
        rtsPolicy: <default("RtsPolicy")> like IRtsPolicy {
            parameters:
                @display("p=550,300");
        }
        ctsPolicy: <default("CtsPolicy")> like ICtsPolicy {
            parameters:
                @display("p=550,400");
        }
}

