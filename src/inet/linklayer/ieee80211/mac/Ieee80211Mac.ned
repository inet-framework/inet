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
import inet.linklayer.ieee80211.mac.contract.IDcf;
import inet.linklayer.ieee80211.mac.contract.IDs;
import inet.linklayer.ieee80211.mac.contract.IHcf;
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
        string fcsMode @enum("declared", "computed") = default("declared");
        string initialRadioMode @enum("off", "sleep", "receiver", "transmitter", "transceiver") = default("receiver");

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
        dcf: <default("Dcf")> like IDcf {
            parameters:
                @display("p=100,100");
        }
        hcf: <default("Hcf")> like IHcf {
            parameters:
                @display("p=250,100");
        }
        ds: <default("Ds")> like IDs {
            parameters:
                @display("p=400,100");
        }
        rx: <default("Rx")> like IRx {
            parameters:
                @display("p=100,200");
        }
        tx: <default("Tx")> like ITx {
            parameters:
                @display("p=250,200");
        }
        statistics: <default("BasicStatistics")> like IStatistics {
            parameters:
                @display("p=400,200");
        }
}

