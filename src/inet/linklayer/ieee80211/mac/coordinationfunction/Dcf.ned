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

import inet.linklayer.ieee80211.mac.channelaccess.Dcaf;
import inet.linklayer.ieee80211.mac.contract.IDcf;
import inet.linklayer.ieee80211.mac.contract.IRateControl;
import inet.linklayer.ieee80211.mac.originator.NonQosRecoveryProcedure;
import inet.linklayer.ieee80211.mac.originator.OriginatorAckPolicy;
import inet.linklayer.ieee80211.mac.originator.OriginatorMacDataService;
import inet.linklayer.ieee80211.mac.originator.RtsPolicy;
import inet.linklayer.ieee80211.mac.protectionmechanism.OriginatorProtectionMechanism;
import inet.linklayer.ieee80211.mac.rateselection.RateSelection;
import inet.linklayer.ieee80211.mac.recipient.CtsPolicy;
import inet.linklayer.ieee80211.mac.recipient.RecipientAckPolicy;
import inet.linklayer.ieee80211.mac.recipient.RecipientMacDataService;

//
// IEEE 802.11 Distributed Coordination Function
//
module Dcf like IDcf
{
    parameters:
        int maxQueueSize = default(100); // -1 means unlimited

        string rxModule;
        string txModule;
        bool prioritizeMulticast = default(false); // if true, prioritize multicast frames (9.3.2.1 Fundamental access)

        *.rateSelectionModule = "^.rateSelection";
        *.rxModule = "^." + rxModule;

        @class(Dcf);
        @display("i=block/queue");
        @signal[packetSentToPeer](type=inet::Packet);
        @signal[packetReceivedFromPeer](type=inet::Packet);
        @signal[linkBroken](type=inet::Packet);
        @signal[packetDropped](type=inet::Packet);
        @signal[frameSequenceStarted];
        @signal[frameSequenceFinished];
        @signal[datarateSelected](type=double);
        @statistic[packetSending](title="packet sending"; source=packetSentToPeer; record=count, vector?; interpolationmode=none);
        @statistic[packetSendingUnicast](title="packet sending: unicast"; source=ieee80211Unicast(packetSentToPeer); record=count, vector?; interpolationmode=none);
        @statistic[packetSendingMulticast](title="packet sending: multicast"; source=ieee80211Multicast(packetSentToPeer); record=count, vector?; interpolationmode=none);
        @statistic[packetSendingBroadcast](title="packet sending: broadcast "; source=ieee80211Broadcast(packetSentToPeer); record=count, vector?; interpolationmode=none);
        @statistic[packetReception](title="packet reception"; source=packetReceivedFromPeer; record=count, vector?; interpolationmode=none);
        @statistic[packetReceptionUnicast](title="packet reception: unicast"; source=ieee80211Unicast(packetReceivedFromPeer); record=count, vector?; interpolationmode=none);
        @statistic[packetReceptionMulticast](title="packet reception: multicast"; source=ieee80211Multicast(packetReceivedFromPeer); record=count, vector?; interpolationmode=none);
        @statistic[packetReceptionBroadcast](title="packet reception: broadcast"; source=ieee80211Broadcast(packetReceivedFromPeer); record=count, vector?; interpolationmode=none);
        @statistic[linkBroken](title="link break"; source=linkBroken; record=count; interpolationmode=none);
        @statistic[packetDropNotAddressedToUs](title="packet drop: not addressed to us"; source=packetDropReasonIsNotAddressedToUs(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropQueueOverflow](title="packet drop: queue overflow"; source=packetDropReasonIsQueueOverflow(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropRetryLimitReached](title="packet drop: retry limit reached"; source=packetDropReasonIsRetryLimitReached(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[frameSequence](title="frame sequence"; source=frameSequenceFinished; record=count);
        @statistic[frameSequenceDuration](title="frame sequence duration"; source=frameSequenceDuration(frameSequenceFinished); record=histogram,vector?);
        @statistic[frameSequenceNumPackets](title="frame sequence number of packets"; source=frameSequenceNumPackets(frameSequenceFinished); record=histogram,vector?);
        @statistic[datarate](title="datarate"; source=datarateSelected; record=vector; interpolationmode=none);

    submodules:
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
                @display("p=350,400");
        }
        originatorAckPolicy: OriginatorAckPolicy {
            parameters:
                @display("p=550,100");
        }
        recipientAckPolicy: RecipientAckPolicy {
            parameters:
                @display("p=550,200");
        }
        rtsPolicy: RtsPolicy {
            parameters:
                @display("p=550,300");
        }
        ctsPolicy: CtsPolicy {
            parameters:
                @display("p=550,400");
        }
}

