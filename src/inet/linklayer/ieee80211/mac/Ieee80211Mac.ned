//
// Copyright (C) 2006 Andras Varga and Levente M�sz�ros
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
// Author: Andras Varga
//

package inet.linklayer.ieee80211.mac;

import inet.linklayer.ieee80211.IIeee80211Mac;

// near copy-paste of inet.linklayer.base.MACProtocolBase to work around "compound-module cannot extend a simple-module" error in OMNeT++ 4.x
module LayeredProtocolBase
{
    parameters:
        @signal[packetSentToUpper](type=cPacket);
        @signal[packetReceivedFromUpper](type=cPacket);
        @signal[packetFromUpperDropped](type=cPacket);
        @signal[packetSentToLower](type=cPacket);
        @signal[packetReceivedFromLower](type=cPacket);
        @signal[packetFromLowerDropped](type=cPacket);
}

// near copy-paste of inet.linklayer.base.MACProtocolBase to work around "compound-module cannot extend a simple-module" error in OMNeT++ 4.x
module MACProtocolBase extends LayeredProtocolBase
{
    parameters:
        string interfaceTableModule;
        int mtu @unit("B") = default(2304B);
        @display("i=block/rxtx");
    gates:
        input upperLayerIn @labels(INetworkDatagram/down);
        output upperLayerOut @labels(INetworkDatagram/up);
        input lowerLayerIn @labels(ILinkLayerFrame/up);
        output lowerLayerOut @labels(ILinkLayerFrame/down);
}

//
// Implementation of the 802.11b MAC protocol. This module is intended
// to be used in combination with the ~Ieee80211Radio module as the physical
// layer. (The ~SnrEval80211 and ~Decider80211 modules should also work if
// per-packet bitrate setting gets implemented.)
//
// Encapsulation/decapsulation must be done in the upper layers. (It is
// typically in the 802.11 management module, see in ~Ieee80211Nic).
// The base class for 802.11 frame messages is ~Ieee80211Frame, but this
// module expects ~Ieee80211DataOrMgmtFrame (a subclass) from upper layers
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
// The following features not supported: 1) fragmentation, 2) power management,
// 3) polling (PCF). Physical layer algorithms such as frequency hopping and
// direct sequence spread spectrum are not modelled directly.
//
// Fields related to the above unsupported features are omitted from
// management frame formats as well (for example, FH/DS/CF parameter sets,
// beacon/probe timestamp which is related to physical layer synchronization,
// listen interval which is related to power management, capability information
// which is related to PCF and other non-modelled features).
//
module Ieee80211Mac extends MACProtocolBase like IIeee80211Mac
{
    parameters:
        string address = default("auto"); // MAC address as hex string (12 hex digits), or
                                          // "auto". "auto" values will be replaced by
                                          // a generated MAC address in init stage 0.
        string rateControlModuleType = default("");
        string upperMacModule = ".upperMac";
        string rxModule = ".rx";
        string txModule = ".tx";
        string firstContentionModule = ".ctn[0]";
        int numContention = default(4);
        string collisionControllerModule = ".ctrl";
        string rateSelectionModule = ".rateSelection";
        string rateControlModule = ".rateControl";
        string statisticsModule = ".statistics";

        *.macModule = "^";
        *.upperMacModule = "^" + upperMacModule;
        *.rxModule = "^" + rxModule;
        *.txModule = "^" + txModule;
        *.firstContentionModule = "^" + firstContentionModule;
        *.collisionControllerModule = "^" + collisionControllerModule;
        *.rateSelectionModule = "^" + rateSelectionModule;
        *.rateControlModule = "^" + rateControlModule;
        *.statisticsModule = "^" + statisticsModule;

        @display("i=block/layer;bgb=721,355");
        @class(Ieee80211Mac);
        @statistic[passedUpPk](title="packets passed to higher layer"; source=packetSentToUpper; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[sentDownPk](title="packets sent to lower layer"; source=packetSentToLower; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rcvdPkFromHL](title="packets received from higher layer"; source=packetReceivedFromUpper; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rcvdPkFromLL](title="packets received from lower layer"; source=packetReceivedFromLower; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);

    submodules:
        rateSelection: <default("BasicRateSelection")> like IRateSelection {
            @display("p=492,66");
        }
        rateControl: <rateControlModuleType> like IRateControl if rateControlModuleType != "" {
            @display("p=581,66");
        }
        statistics: <default("BasicStatistics")> like IStatistics {
            @display("p=49,66");
        }
        upperMac: <default("EdcaUpperMac")> like IUpperMac {
            @display("p=227,88");
        }
        tx: <default("Tx")> like ITx {
            @display("p=198,260");
        }
        ctn[numContention]: <default("Contention")> like IContention {
            @display("p=295,260,r,60");
        }
        rx: <default("Rx")> like IRx {
            @display("p=112,260");
        }
        ctrl: <default("CollisionController")> like ICollisionController if numContention > 1 {
            @display("p=334,180");
        }
}

//
// Delegate some upperMac parameters to Ieee80211Mac, to be more compatible with the old MAC.
// NOTE: NOT ALL PARAMETERS OF THE OLD MAC CAN BE MAPPED.
//
module Ieee80211CompatibleMac extends Ieee80211Mac
{
    parameters:
        string opMode @enum("a","b","g","n","p") = default("g");
        double bitrate @unit("bps");
        double basicBitrate @unit("bps") = default(-1bps);
        double controlBitrate @unit("bps") = default(-1bps);
        int rtsThresholdBytes @unit("B") = default(2346B);
        int retryLimit = default(-1);
        bool EDCA = default(false);
        int maxQueueSize = default(14);

        int AIFSN0 = default(7); // AIFSN for background
        int AIFSN1 = default(3); // AIFSN for best effort
        int AIFSN2 = default(2); // AIFSN for video
        int AIFSN3 = default(2); // AIFSN for voice
        double TXOP0 @unit(s) = default(0s);
        double TXOP1 @unit(s) = default(0s);
        double TXOP2 @unit(s) = default(3.008ms);
        double TXOP3 @unit(s) = default(1.504ms);
        int AIFSN = default(2); // if there is only one AC (EDCA = false)
        bool prioritizeMulticast = default(false); // if true, prioritize multicast frames (9.3.2.1 Fundamental access)
        double slotTime @unit("s") = default(-1s); // slot time, -1 means default (mode dependent)
        int cwMinData = default(-1); // contention window for normal data frames, -1 means default
        int cwMaxData = default(-1); // contention window for normal data frames, -1 means default
        int cwMinMulticast = default(-1); // contention window for broadcast messages, -1 means default

        upperMac.typename = EDCA ? "EdcaUpperMac" : "DcfUpperMac";
        numContention = EDCA ? 4 : 1;

        rateSelection.modeSet = opMode; // BasicRateSelection
        rateSelection.dataBitrate = bitrate; // BasicRateSelection
        rateSelection.multicastBitrate = basicBitrate;
        rateSelection.controlBitrate = controlBitrate;
        upperMac.rtsThreshold = rtsThresholdBytes; // DcfUpperMac, EdcaUpperMac
        upperMac.shortRetryLimit = retryLimit; // DcfUpperMac, EdcaUpperMac
        upperMac.maxQueueSize = maxQueueSize; // DcfUpperMac, EdcaUpperMac

        upperMac.aifsn0 = AIFSN0; // EdcaUpperMac
        upperMac.aifsn1 = AIFSN1; // EdcaUpperMac
        upperMac.aifsn2 = AIFSN2; // EdcaUpperMac
        upperMac.aifsn3 = AIFSN3; // EdcaUpperMac
        upperMac.txopLimit0 = TXOP0; // EdcaUpperMac
        upperMac.txopLimit1 = TXOP1; // EdcaUpperMac
        upperMac.txopLimit2 = TXOP2; // EdcaUpperMac
        upperMac.txopLimit3 = TXOP3; // EdcaUpperMac
        //TODO upperMac.difsn =  AIFSN;
        upperMac.prioritizeMulticast = prioritizeMulticast; // DcfUpperMac, EdcaUpperMac
        upperMac.slotTime = slotTime; // DcfUpperMac, EdcaUpperMac
        upperMac.cwMin = cwMinData; // DcfUpperMac
        upperMac.cwMax = cwMaxData; // DcfUpperMac
        upperMac.cwMinMulticast = cwMulticast; // DcfUpperMac, EdcaUpperMac
}
