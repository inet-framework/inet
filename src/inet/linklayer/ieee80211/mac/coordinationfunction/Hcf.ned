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
        int maxQueueSize = default(100); // -1 means unlimited
        bool isBlockAckSupported = default(false);
        bool prioritizeMulticast = default(false); // if true, prioritize multicast frames (9.3.2.1 Fundamental access)

        string rxModule;
        string txModule;

        *.rateSelectionModule = "^.rateSelection";
        *.rxModule = "^." + rxModule;

        @class(Hcf);
        @display("i=block/queue");
        @signal[packetSentToPeer](type=inet::Packet);
        @signal[packetReceivedFromPeer](type=inet::Packet);
        @signal[linkBroken](type=inet::Packet);
        @signal[packetDropped](type=inet::Packet);
        @signal[edcaCollisionDetected](type=unsigned long);
        @signal[frameSequenceStarted];
        @signal[frameSequenceFinished];
        @signal[datarateSelected](type=double);
        @signal[blockAckAgreementAdded];
        @signal[blockAckAgreementDeleted];
        @statistic[packetSentToPeer](title="packet sent"; record=count, vector?; interpolationmode=none);
        @statistic[packetSentToPeerUnicast](title="packet sent: unicast"; source=ieee80211Unicast(packetSentToPeer); record=count, vector?; interpolationmode=none);
        @statistic[packetSentToPeerMulticast](title="packet sent: multicast"; source=ieee80211Multicast(packetSentToPeer); record=count, vector?; interpolationmode=none);
        @statistic[packetSentToPeerBroadcast](title="packet sent: broadcast "; source=ieee80211Broadcast(packetSentToPeer); record=count, vector?; interpolationmode=none);
        @statistic[packetSentToPeerWithRetry](title="packet sent: with retry"; source=ieee80211Retry(packetSentToPeer); record=count);
        @statistic[packetSentToPeerWithoutRetry](title="packet sent: without retry"; source=ieee80211NotRetry(packetSentToPeer); record=count);
        @statistic[packetReceivedFromPeer](title="packet received"; record=count, vector?; interpolationmode=none);
        @statistic[packetReceivedFromPeerUnicast](title="packet received: unicast"; source=ieee80211Unicast(packetReceivedFromPeer); record=count, vector?; interpolationmode=none);
        @statistic[packetReceivedFromPeerMulticast](title="packet received: multicast"; source=ieee80211Multicast(packetReceivedFromPeer); record=count, vector?; interpolationmode=none);
        @statistic[packetReceivedFromPeerBroadcast](title="packet received: broadcast"; source=ieee80211Broadcast(packetReceivedFromPeer); record=count, vector?; interpolationmode=none);
        @statistic[packetReceivedFromPeerWithRetry](title="packet received: with retry"; source=ieee80211Retry(packetReceivedFromPeer); record=count);
        @statistic[packetReceivedFromPeerWithoutRetry](title="packet received: without retry"; source=ieee80211NotRetry(packetReceivedFromPeer); record=count);
        @statistic[linkBroken](title="link broken"; record=count; interpolationmode=none);
        @statistic[packetDropNotAddressedToUs](title="packet drop: not addressed to us"; source=packetDropReasonIsNotAddressedToUs(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropQueueOverflow](title="packet drop: queue overflow"; source=packetDropReasonIsQueueOverflow(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropRetryLimitReached](title="packet drop: retry limit reached"; source=packetDropReasonIsRetryLimitReached(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[edcaCollisionDetected](title="EDCA collision detected"; record=count);
        @statistic[frameSequenceFinished](title="frame sequence finished"; record=count);
        @statistic[frameSequenceDuration](title="frame sequence duration"; source=frameSequenceDuration(frameSequenceFinished); record=histogram,vector?);
        @statistic[frameSequenceNumPackets](title="frame sequence number of packets"; source=frameSequenceNumPackets(frameSequenceFinished); record=histogram,vector?);
        @statistic[datarateSelected](title="datarate selected"; record=vector; interpolationmode=none);
        @statistic[blockAckAgreementAdded](title="block ack agreement added"; record=count);

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
        rateSelection: QosRateSelection {
            parameters:
                rateControlModule = "^.rateControl";
                @display("p=350,100");
        }
        rateControl: <default("")> like IRateControl if typename != "" {
            parameters:
                @display("p=350,200");
        }
        singleProtectionMechanism: SingleProtectionMechanism {
            parameters:
                @display("p=350,300");
        }
        edcaMgmtAndNonQoSRecoveryProcedure: NonQosRecoveryProcedure {
            parameters:
                cwCalculatorModule = "^.edca.edcaf[1]"; // best effort
                rtsPolicyModule = "^.rtsPolicy";
                @display("p=550,300");
        }
        originatorAckPolicy: OriginatorQosAckPolicy {
            parameters:
                @display("p=750,100");
        }
        recipientAckPolicy: RecipientQosAckPolicy {
            parameters:
                @display("p=750,200");
        }
        rtsPolicy: QosRtsPolicy {
            parameters:
                @display("p=750,300");
        }
        ctsPolicy: QosCtsPolicy {
            parameters:
                @display("p=750,400");
        }
        originatorBlockAckAgreementPolicy: OriginatorBlockAckAgreementPolicy if isBlockAckSupported {
            parameters:
                originatorAckPolicyModule = "^.originatorAckPolicy";
                @display("p=750,500");
        }
        recipientBlockAckAgreementPolicy: RecipientBlockAckAgreementPolicy if isBlockAckSupported {
            parameters:
                @display("p=750,600");
        }
}

