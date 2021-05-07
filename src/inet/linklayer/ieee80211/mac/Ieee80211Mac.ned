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
import inet.linklayer.ieee80211.mac.contract.ITx;
import inet.linklayer.ieee80211.mac.coordinationfunction.Dcf;
import inet.linklayer.ieee80211.mac.coordinationfunction.Hcf;

//
// Implementation of the 802.11b MAC protocol. This module is intended
// to be used in combination with the ~Ieee80211Radio module as the physical
// layer.
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
// <b>Limitations</b>
//
// Power management and polling (PCF) are not supported.
//
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
        string modeSet @enum("a", "b", "g(erp)", "g(mixed)", "n(mixed-2.4Ghz)", "p", "ac") = default("g(mixed)");
        string fcsMode @enum("declared", "computed") = default("declared");
        string initialRadioMode @enum("off", "sleep", "receiver", "transmitter", "transceiver") = default("receiver");

        int mtu @unit(B) = default(2304B);
        bool qosStation = default(false);

        *.mibModule = default(absPath(this.mibModule));
        *.rxModule = "^.rx";
        *.txModule = "^.tx";

        @display("i=block/layer");
        @class(Ieee80211Mac);
        @signal[linkBroken](type=inet::Packet); // TODO: this signal is only present for the statistic to pass the signal check, to be removed
        @signal[modesetChanged](type=inet::physicallayer::Ieee80211ModeSet);
        @statistic[packetSentToUpper](title="packets sent to upper layer"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetSentToLower](title="packets sent to lower layer"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetReceivedFromUpper](title="packets received from upper layer"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetReceivedFromLower](title="packets received from lower layer"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[linkBroken](title="link breaks"; record=count, vector?; interpolationmode=none);
        @statistic[packetDrop](title="packet drops"; source=packetDropped; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropIncorrectlyReceived](title="packet drops: incorrectly received"; source=packetDropReasonIsIncorrectlyReceived(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropNotAddressedToUs](title="packet drops: not addressed to us"; source=packetDropReasonIsNotAddressedToUs(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropQueueOverflow](title="packet drops: queue overflow"; source=packetDropReasonIsQueueOverflow(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropRetryLimitReached](title="packet drops: retry limit reached"; source=packetDropReasonIsRetryLimitReached(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropDuplicateDetected](title="packet drops: duplicate detected"; source=packetDropReasonIsDuplicateDetected(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropOther](title="packet drops: other"; source=packetDropReasonIsOther(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
    gates:
        input mgmtIn;
        output mgmtOut;
    submodules:
        dcf: <default("Dcf")> like IDcf {
            parameters:
                @display("p=100,100");
        }
        hcf: <default("Hcf")> like IHcf if qosStation {
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
}

