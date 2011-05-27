//
// Copyright (C) 2006 Andras Varga and Levente Meszaros and Lukáš Hlůže
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


package inet.linklayer.ieee80211.mac;

//
// Implementation of the 802.11b MAC protocol. This module is intended
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
simple Ieee80211NewMac
{
    parameters:
        string address = default("auto"); // MAC address as hex string (12 hex digits), or
                                          // "auto". "auto" values will be replaced by
                                          // a generated MAC address in init stage 0.
        string queueModule = default("");    // name of optional external queue module
        int maxQueueSize = default(50); // max queue length in frames; only used if queueModule==""
        int defaultAC = default(0); // what is default AC category
        int AIFSN0 = default(7); //AIFSN for background
        int AIFSN1 = default(3); //AIFSN for best effort
        int AIFSN2 = default(2); //AIFSN for video
        int AIFSN3 = default(2); //AIFSN for voice
        int AIFSN  = default(2); //if only exist 1 AC
        double TXOP0 @unit(s)= default(0s);
        double TXOP1 @unit(s)= default(0s);
        double TXOP2 @unit(s)= default(3.008ms);    //3.008ms
        double TXOP3 @unit(s)= default(1.504ms);    //1.504ms
        string classifierClass =default("Ieee80211eClassifier");
        bool EDCA = default(true);

        double bitrate @unit("bps");
        string opMode@enum ("b","g","a","p") = default("g");
        string WifiPreambleMode @enum ("LONG","SHORT") = default("LONG"); // Wifi preambre mode Ieee 2007, 19.3.2

        double basicBitrate @unit("bps")= default(2e6bps);
        int mtu = default(1500);
        double slotTime @unit("s")= default(9us);
        int rtsThresholdBytes @unit("B") = default(2346B); // longer messages will be sent using RTS/CTS
        int retryLimit = default(-1); // maximum number of retries per message, -1 means default
        int cwMinData = default(-1); // contention window for normal data frames, -1 means default
        int cwMaxData = default(-1); // contention window for normal data frames, -1 means default
        int cwMinBroadcast = default(-1); // contention window for broadcast messages, -1 means default
        bool fixFSM = default(false);
// If -1 The mac compute the  PHY_HEADER_LENGTH in function of the modulation type
        double PHY_HEADER_LENGTH = default(-1);// if negative the code use 26 usec
        bool ForceBitRate = default(false); // if true the mac force the bitrate to the physical layer
// autobitrate algorithm by default is disabled
        int autoBitrate @enum(0,1,2)= default(0);// 0 Contant Bit rate, 1 ARF Rate, 2 AARF Rate
// parameters used by the autobitrate
        int minSuccessThreshold =default(10);
        int minTimerTimeout=default(15);
        int timerTimeout=default(15);
        int successThreshold =default(10);
        int maxSuccessThreshold = default(60);
        double successCoeff=default(2.0);
        double timerCoeff = double(2.0);
        // period of time usef byt the throughput measure
        double throughputTimePeriod @unit("s")=default(0);

        @display("i=block/layer");
    gates:
        input uppergateIn @labels(Ieee80211Frame);
        output uppergateOut @labels(Ieee80211Frame);
        input lowergateIn @labels(Ieee80211Frame);
        output lowergateOut @labels(Ieee80211Frame);
}
