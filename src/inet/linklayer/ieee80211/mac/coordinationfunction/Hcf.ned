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

package inet.linklayer.ieee80211.mac.coordinationfunction;

import inet.linklayer.ieee80211.mac.blockack.OriginatorBlockAckAgreementPolicy;
import inet.linklayer.ieee80211.mac.blockack.RecipientBlockAckAgreementPolicy;
import inet.linklayer.ieee80211.mac.channelaccess.Edca;
import inet.linklayer.ieee80211.mac.channelaccess.Hcca;
import inet.linklayer.ieee80211.mac.contract.IRateControl;
import inet.linklayer.ieee80211.mac.contract.IHcf;
import inet.linklayer.ieee80211.mac.originator.NonQosRecoveryProcedure;
import inet.linklayer.ieee80211.mac.originator.OriginatorQosAckPolicy;
import inet.linklayer.ieee80211.mac.originator.OriginatorQosMacDataService;
import inet.linklayer.ieee80211.mac.originator.QosRecoveryProcedure;
import inet.linklayer.ieee80211.mac.originator.QosRtsPolicy;
import inet.linklayer.ieee80211.mac.originator.TxopProcedure;
import inet.linklayer.ieee80211.mac.protectionmechanism.SingleProtectionMechanism;
import inet.linklayer.ieee80211.mac.rateselection.QosRateSelection;
import inet.linklayer.ieee80211.mac.recipient.QosCtsPolicy;
import inet.linklayer.ieee80211.mac.recipient.RecipientQosAckPolicy;
import inet.linklayer.ieee80211.mac.recipient.RecipientQosMacDataService;

//
// HCF -- Hybrid Coordination Function
//
module Hcf like IHcf
{
    parameters:
        bool isBlockAckSupported = default(false);

        string rxModule;
        string txModule;

        *.rateSelectionModule = "^.rateSelection";
        *.rxModule = "^." + this.rxModule;
        *.*.originatorMacDataServiceModule = "^.^.originatorMacDataService";

        @class(Hcf);
        @display("i=block/layer");
        @signal[packetSentToPeer](type=inet::Packet);
        @signal[packetReceivedFromPeer](type=inet::Packet);
        @signal[linkBroken](type=inet::Packet);
        @signal[packetDropped](type=inet::Packet);
        @signal[edcaCollisionDetected](type=unsigned long);
        @signal[frameSequenceStarted];
        @signal[frameSequenceFinished];
        @signal[frameSequenceAborted];
        @signal[datarateSelected](type=double);
        @signal[blockAckAgreementAdded];
        @signal[blockAckAgreementDeleted];
        @statistic[packetSentToPeer](title="packets sent"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetSentToPeerUnicast](title="packets sent: unicast"; source=ieee80211Unicast(packetSentToPeer); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetSentToPeerMulticast](title="packets sent: multicast"; source=ieee80211Multicast(packetSentToPeer); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetSentToPeerBroadcast](title="packets sent: broadcast "; source=ieee80211Broadcast(packetSentToPeer); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetSentToPeerWithRetry](title="packets sent: with retry"; source=ieee80211Retry(packetSentToPeer); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetSentToPeerWithoutRetry](title="packets sent: without retry"; source=ieee80211NotRetry(packetSentToPeer); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetReceivedFromPeer](title="packets received"; count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
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
        @statistic[edcaCollisionDetected](title="EDCA collisions detected"; record=count);
        @statistic[frameSequenceFinished](title="finished frame sequences"; record=count,vector?);
        @statistic[frameSequenceDuration](title="frame sequence durations"; source=frameSequenceDuration(frameSequenceFinished); record=histogram,vector?);
        @statistic[frameSequenceNumPackets](title="frame sequences: number of packets"; source=frameSequenceNumPackets(frameSequenceFinished); record=histogram,vector?);
        @statistic[frameSequenceActive](title="frame sequence active"; source=count(frameSequenceStarted)-count(frameSequenceFinished); record=vector; interpolationmode=sample-hold);
        @statistic[datarateSelected](title="datarates selected"; record=vector; interpolationmode=none);
        @statistic[blockAckAgreementAdded](title="added block ack agreements"; record=count);
        @statistic[blockAckAgreementDeleted](title="deleted block ack agreements"; record=count);
        @statistic[blockAckAgreementActive](title="active block ack agreements"; source=count(blockAckAgreementAdded)-count(blockAckAgreementDeleted); record=vector; interpolationmode=sample-hold);

    submodules:
        edca: Edca {
            parameters:
                @display("p=150,100");
        }
        hcca: Hcca {
            parameters:
                @display("p=150,200");
        }
        originatorMacDataService: OriginatorQosMacDataService {
            parameters:
                @display("p=150,300");
        }
        recipientMacDataService: RecipientQosMacDataService {
            parameters:
                @display("p=150,400");
        }
        singleProtectionMechanism: SingleProtectionMechanism {
            parameters:
                @display("p=150,500");
        }
        rateSelection: QosRateSelection {
            parameters:
                rateControlModule = "^.rateControl";
                @display("p=350,100");
        }
        rateControl: <default("")> like IRateControl if typename != "" {
            parameters:
                @display("p=350,200");
        }
        originatorAckPolicy: OriginatorQosAckPolicy {
            parameters:
                @display("p=550,100");
        }
        recipientAckPolicy: RecipientQosAckPolicy {
            parameters:
                @display("p=550,200");
        }
        rtsPolicy: QosRtsPolicy {
            parameters:
                @display("p=550,300");
        }
        ctsPolicy: QosCtsPolicy {
            parameters:
                @display("p=550,400");
        }
        originatorBlockAckAgreementPolicy: OriginatorBlockAckAgreementPolicy if isBlockAckSupported {
            parameters:
                originatorAckPolicyModule = "^.originatorAckPolicy";
                @display("p=550,500");
        }
        recipientBlockAckAgreementPolicy: RecipientBlockAckAgreementPolicy if isBlockAckSupported {
            parameters:
                @display("p=550,600");
        }
}

