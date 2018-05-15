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

package inet.linklayer.ieee80211.mac;

import inet.linklayer.base.MacProtocolBase;
import inet.linklayer.ieee80211.IIeee80211Mac;
import inet.linklayer.ieee80211.mac.contract.IRx;
import inet.linklayer.ieee80211.mac.contract.IStatistics;
import inet.linklayer.ieee80211.mac.contract.ITx;
import inet.linklayer.ieee80211.mac.coordinationfunction.Dcf;
import inet.linklayer.ieee80211.mac.coordinationfunction.Hcf;

//
// Implementation of the 802.11b MAC protocol. This module is intended
// to be used in combination with the ~Ieee80211Radio module as the physical
// layer. (The ~SnrEval80211 and ~Decider80211 modules should also work if
// per-packet bitrate setting gets implemented.)
//
// Encapsulation/decapsulation must be done in the upper layers. (It is
// typically in the 802.11 management module, see in ~Ieee80211Interface).
// The base class for 802.11 frame messages is ~Ieee80211MacHeader, but this
// module expects ~Ieee80211DataOrMgmtHeader (a subclass) from upper layers
// (the management module). This module will assign the transmitter address
// (address 2) and the frame sequence number/fragment number fields in the
// frames; all other fields must already be filled when this module gets
// the frame for transmission.
//
// The module has an internal queue, but usually it is to be used with an
// external passive queue module (its name should be given in the queueModule
// parameter; specifying "" causes the internal queue to be used). The passive
// queue module is a simple module whose C++ class implements the IPassiveQueue
// interface.
//
// <b>Limitations</b>
//
// The following features not supported: 1) power management, 2) polling (PCF).
// Physical layer algorithms such as frequency hopping and
// direct sequence spread spectrum are not modelled directly.
//
// Fields related to the above unsupported features are omitted from
// management frame formats as well (for example, FH/DS/CF parameter sets,
// beacon/probe timestamp which is related to physical layer synchronization,
// listen interval which is related to power management, capability information
// which is related to PCF and other non-modelled features).
//
module Ieee80211Mac extends MacProtocolBase like IIeee80211Mac
{
    parameters:
        string mibModule;
        string address = default("auto"); // MAC address as hex string (12 hex digits), or
                                          // "auto". "auto" values will be replaced by
                                          // a generated MAC address in init stage 0.
        string modeSet @enum("a", "b", "g(erp)", "g(mixed)", "n(mixed-2.4Ghz)", "p", "ac") = default("g(mixed)");
        string fcsMode @enum("declared","computed") = default("declared");

        int mtu @unit(B) = default(2304B);
        bool qosStation = default(false);

        *.mibModule = default(absPath(mibModule));
        *.rxModule = "^.rx";
        *.txModule = "^.tx";

        @display("i=block/layer");
        @class(Ieee80211Mac);
        @signal[packetSentToPeer](type=Packet);
        @signal[packetReceivedFromPeer](type=Packet);
        @statistic[passedUpPk](title="packets passed to higher layer"; source=packetSentToUpper; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[sentDownPk](title="packets sent to lower layer"; source=packetSentToLower; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rcvdPkFromHl](title="packets received from higher layer"; source=packetReceivedFromUpper; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rcvdPkFromLl](title="packets received from lower layer"; source=packetReceivedFromLower; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);

        @signal[linkBroken];
        @statistic[linkBroken](title="link break"; source=linkBroken; record=count; interpolationmode=none);
        @statistic[packetDropIncorrectlyReceived](title="packet drop: incorrectly received"; source=packetDropReasonIsIncorrectlyReceived(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropNotAddressedToUs](title="packet drop: not addressed to us"; source=packetDropReasonIsNotAddressedToUs(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropQueueOverflow](title="packet drop: queue overflow"; source=packetDropReasonIsQueueOverflow(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropRetryLimitReached](title="packet drop: retry limit reached"; source=packetDropReasonIsRetryLimitReached(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @signal[modesetChanged](type=inet::physicallayer::Ieee80211ModeSet);
    gates:
        input mgmtIn;
        output mgmtOut;
    submodules:
        dcf: Dcf {
            parameters:
                @display("p=100,100");
        }
        hcf: Hcf {
            parameters:
                @display("p=250,100");
        }
        ds: Ds {
            parameters:
                @display("p=400,100");
        }
        rx: <default("Rx")> like IRx {
            parameters:
                @display("p=100,250");
        }
        tx: <default("Tx")> like ITx {
            parameters:
                @display("p=250,250");
        }
        statistics: <default("BasicStatistics")> like IStatistics {
            parameters:
                @display("p=400,250");
        }
}

//
// Ieee80211Mac parameterized in a way to make it more similar to the old,
// monolithic 802.11 MAC module of INET 3.x.
//
// NOTE: NOT ALL PARAMETERS OF THE OLD MAC CAN BE MAPPED.
//
module Ieee80211CompatibleMac extends Ieee80211Mac
{
    parameters:
        string rateControlType = default("");
        string opMode @enum("a", "b", "g(erp)", "g(mixed)", "n(mixed-2.4Ghz)", "p", "ac") = default("g(mixed)");
        double bitrate @unit(bps) = default(-1bps); // for backward compatibility
        double basicBitrate @unit(bps) = default(-1bps); // for backward compatibility
        double dataBitrate @unit(bps) = default(bitrate);
        double mgmtBitrate @unit(bps) = default(basicBitrate);
        double multicastBitrate @unit(bps) = default(basicBitrate);
        double controlBitrate @unit(bps) = default(-1bps);
        double initialRateControlRate @unit(bps) = default(-1bps);
        int rtsThresholdBytes @unit(B) = default(2346B);
        int retryLimit = default(7);
        bool EDCA = default(false);
        int maxQueueSize = default(14);

        int AIFSN0 = default(7); // AIFSN for background
        int AIFSN1 = default(3); // AIFSN for best effort
        int AIFSN2 = default(2); // AIFSN for video
        int AIFSN3 = default(2); // AIFSN for voice
        int AIFSN = default(-1); // if there is only one AC (EDCA = false); -1 means default
        double TXOP0 @unit(s) = default(0s);
        double TXOP1 @unit(s) = default(0s);
        double TXOP2 @unit(s) = default(3.008ms);
        double TXOP3 @unit(s) = default(1.504ms);
        bool prioritizeMulticast = default(false); // if true, prioritize multicast frames (9.3.2.1 Fundamental access)
        int cwMinData = default(-1); // contention window for normal data frames (DCF) and best effort (EDCA), -1 means default
        int cwMaxData = default(-1); // contention window for normal data frames (DCF) and best effort (EDCA), -1 means default

        modeSet = opMode;
        qosStation = EDCA;
        dcf.maxQueueSize = maxQueueSize;
        hcf.maxQueueSize = maxQueueSize;
        dcf.rateControlType = rateControlType;
        hcf.rateControlType = rateControlType;
        dcf.rateSelection.multicastFrameBitrate = multicastBitrate;
        hcf.rateSelection.multicastFrameBitrate = multicastBitrate;
        dcf.rateSelection.dataFrameBitrate = dataBitrate;
        hcf.rateSelection.dataFrameBitrate = dataBitrate;
        dcf.rateSelection.mgmtFrameBitrate = mgmtBitrate;
        hcf.rateSelection.mgmtFrameBitrate = mgmtBitrate;
        dcf.rateSelection.controlFrameBitrate = controlBitrate;
        hcf.rateSelection.controlFrameBitrate = controlBitrate;
        dcf.rateSelection.responseAckFrameBitrate = controlBitrate;
        hcf.rateSelection.responseAckFrameBitrate = controlBitrate;
        dcf.rateControl.initialRate = initialRateControlRate; // applies to OnoeRateControl, AArfRateControl
        hcf.rateControl.initialRate = initialRateControlRate; // applies to OnoeRateControl, AArfRateControl
        dcf.rtsPolicy.rtsThreshold = rtsThresholdBytes;
        hcf.rtsPolicy.rtsThreshold = rtsThresholdBytes;
        dcf.recoveryProcedure.shortRetryLimit = retryLimit;
        hcf.edcaDataRecoveryProcedures[*].shortRetryLimit = retryLimit;

        hcf.edca.edcaf[0].aifsn = AIFSN0;
        hcf.edca.edcaf[1].aifsn = AIFSN1;
        hcf.edca.edcaf[2].aifsn = AIFSN2;
        hcf.edca.edcaf[3].aifsn = AIFSN3;
        dcf.channelAccess.difsn = AIFSN;
        hcf.edcaTxopProcedures[0].txopLimit = TXOP0;
        hcf.edcaTxopProcedures[1].txopLimit = TXOP1;
        hcf.edcaTxopProcedures[2].txopLimit = TXOP2;
        hcf.edcaTxopProcedures[3].txopLimit = TXOP3;
        dcf.prioritizeMulticast = prioritizeMulticast;
        hcf.prioritizeMulticast = prioritizeMulticast;
        dcf.channelAccess.cwMin = cwMinData;
        dcf.channelAccess.cwMax = cwMaxData;
        hcf.edca.edcaf[1].cwMin = cwMinData;
        hcf.edca.edcaf[1].cwMax = cwMaxData;
}
