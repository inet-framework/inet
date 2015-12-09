//
// Copyright (C) 2006 Andras Varga, Levente Meszaros and Lukas Hluze
// Copyright (C) 2011 Alfonso Ariza
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

package inet.linklayer.ieee80211.oldmac;

import inet.linklayer.base.MACProtocolBase;
import inet.linklayer.ieee80211.IIeee80211Mac;


//
// Implementation of the 802.11 MAC protocol. This module is intended
// to be used in combination with the Ieee80211Radio module as the physical
// layer. (The SnrEval80211 and Decider80211 modules should also work if
// per-packet bitrate setting gets implemented.)
//
// Encapsulation/decapsulation must be done in the upper layers. (It is
// typically in the 802.11 management module, see in Ieee80211Nic).
// The base class for 802.11 frame messages is Ieee80211Frame, but this
// module expects Ieee80211DataOrMgmtFrame (a subclass) from upper layers
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
simple Ieee80211OldMac extends MACProtocolBase like IIeee80211Mac
{
    parameters:
        string address = default("auto"); // MAC address as hex string (12 hex digits), or
                                          // "auto". "auto" values will be replaced by
                                          // a generated MAC address in init stage 0.
        int maxQueueSize = default(50);   // max queue length in frames (for EDCA this is a per AC queue limit)

        bool EDCA = default(false); // enable Enhanced Distributed Channel Access (802.11e)
        // parameters for EDCA = true
        string classifier = default("");    // not in use, use the Ieee80211Nic.classifierType parameter instead
        int defaultAC = default(1); // the default AC category for frames that cannot be classified
        int AIFSN0 = default(7); // AIFSN for background
        int AIFSN1 = default(3); // AIFSN for best effort
        int AIFSN2 = default(2); // AIFSN for video
        int AIFSN3 = default(2); // AIFSN for voice
        double TXOP0 @unit(s) = default(0s);
        double TXOP1 @unit(s) = default(0s);
        double TXOP2 @unit(s) = default(3.008ms);
        double TXOP3 @unit(s) = default(1.504ms);
        // parameters for EDCA = false
        int AIFSN = default(2); // if there is only one AC (EDCA = false)

        bool useModulationParameters = default(false); // if true, slot time, DIFS, and ACK timeout (aPHY-RX-START-Delay) are function of modulation time (2007 standard)
        bool prioritizeMulticast = default(false); // if true, prioritize multicast frames (9.3.2.1 Fundamental access)

        double bitrate @unit("bps");
        string opMode @enum("a","b","g","n","p") = default("g");

        // TODO: These bitrates are not a real degrees of freedom
        // with OFDM modulation the basic bitrate is 6Mbps
        double basicBitrate @unit("bps") = default(1e6bps); // used for broadcast frames
        double controlBitrate @unit("bps") = default(1e6bps); // used for control frames

        int mtu @unit("B") = default(2304B);
        double slotTime @unit("s") = default(-1s); // slot time, -1 means default (mode dependent)
        int rtsThresholdBytes @unit("B") = default(2346B); // longer messages will be sent using RTS/CTS
        int retryLimit = default(-1); // maximum number of retries per message, -1 means default
        int cwMinData = default(-1); // contention window for normal data frames, -1 means default
        int cwMaxData = default(-1); // contention window for normal data frames, -1 means default
        int cwMinMulticast = default(-1); // contention window for broadcast messages, -1 means default

        double phyHeaderLength @unit("s") = default(-1s); // when <0, the MAC will compute it in function of the modulation type
        bool forceBitRate = default(false); // if true, the MAC will force the bitrate to the physical layer
        int autoBitrate @enum(0,1,2) = default(0); // 0 = constant bit rate (autobitrate algorithm disabled), 1 = ARF Rate, 2 = AARF Rate
        // parameters used by the autobitrate
        int minTimerTimeout = default(15);
        int timerTimeout = default(minTimerTimeout);
        int minSuccessThreshold = default(10);
        int successThreshold = default(minSuccessThreshold);
        int maxSuccessThreshold = default(60);
        double successCoeff = default(2.0);
        double timerCoeff = default(2.0);
        // duplicate detection
        bool duplicateDetectionFilter = default(true); // whether to detect and filter out duplicate frames
        bool purgeOldTuples = default(true); // delete old tuples in the duplicate list
        double duplicateTimeOut @unit("s") = default(20s); // timeout for the duplicate detection
        // statistics
        double throughputTimePeriod @unit("s") = default(0); // period of time used by throughput measurement statistic
        bool multiMac = default(false); // allows multiples mac interfaces in the same link layer

        @class(Ieee80211OldMac);
        @display("i=block/layer");

        @signal[NF_LINK_FULL_PROMISCUOUS](type=cPacket);
        @signal[NF_LINK_PROMISCUOUS](type=cPacket);
        @signal[NF_LINK_BREAK](type=Ieee80211DataOrMgmtFrame);
        @signal[packetSentToLower](type=Ieee80211Frame);
        @signal[packetReceivedFromLower](type=cPacket);     // expected type=Ieee80211Frame
        @signal[packetSentToUpper](type=cPacket);   // cPacket with Ieee802Ctrl ctrlinfo
        @signal[packetReceivedFromUpper](type=cPacket);   // cPacket with Ieee802Ctrl ctrlinfo

        @statistic[passedUpPk](title="packets passed to higher layer"; source=packetSentToUpper; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[sentDownPk](title="packets sent to lower layer"; source=packetSentToLower; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rcvdPkFromHL](title="packets received from higher layer"; source=packetReceivedFromUpper; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rcvdPkFromLL](title="packets received from lower layer"; source=packetReceivedFromLower; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);

    gates:
        upperLayerIn @labels(Ieee80211Frame);
        upperLayerOut @labels(Ieee80211Frame);
        lowerLayerIn @labels(Ieee80211Frame);
        lowerLayerOut @labels(Ieee80211Frame);
}
