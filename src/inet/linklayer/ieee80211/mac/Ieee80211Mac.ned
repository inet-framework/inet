//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac;

import inet.linklayer.base.MacProtocolBase;
import inet.linklayer.ieee80211.IIeee80211Mac;
import inet.linklayer.ieee80211.mac.contract.IDcf;
import inet.linklayer.ieee80211.mac.contract.IDs;
import inet.linklayer.ieee80211.mac.contract.IHcf;
import inet.linklayer.ieee80211.mac.contract.IRx;
import inet.linklayer.ieee80211.mac.contract.ITx;


//
// Provides a highly modular implementation of the IEEE 802.11 MAC (Medium Access
// Control) protocol with support for multiple amendments and operational modes.
// This module implements the core MAC functionality for wireless LANs according
// to the IEEE 802.11 standard family.
//
// The implementation features a component-based architecture where specific MAC
// functions are delegated to specialized submodules. This design enables high
// flexibility for customization, extension, and experimentation by allowing
// individual components to be replaced or modified independently.
//
// Key features and capabilities:
//
// - Standards support: Implements 802.11a/b/g/e/p with partial support for newer
//   amendments like 802.11n and 802.11ac
//
// - Channel access mechanisms:
//   * DCF (Distributed Coordination Function) with CSMA/CA for basic access
//   * HCF (Hybrid Coordination Function) with EDCA (Enhanced Distributed Channel
//     Access) for QoS support
//
// - Operation modes:
//   * Infrastructure mode (stations connecting to access points)
//   * Ad hoc mode (direct station-to-station communication)
//
// - Advanced MAC features:
//   * Frame aggregation for improved throughput
//   * Fragmentation for handling large packets
//   * Block acknowledgment for efficient frame acknowledgment
//   * Dynamic rate selection with support for various rate control algorithms
//   * RTS/CTS mechanism for hidden node problem mitigation
//
// - Modular components:
//   * DCF: Handles basic CSMA/CA channel access
//   * HCF: Provides QoS support through EDCA
//   * DS: Manages distribution system services
//   * RX: Processes incoming frames
//   * TX: Manages outgoing frame transmission
//
// The module works in conjunction with the management layer (~IIeee80211Mgmt)
// that handles operations like scanning, authentication, and association, and
// the physical layer (~IRadio) that handles the actual transmission and reception
// of signals over the wireless medium.
//
// @see ~Ieee80211Interface, ~IIeee80211Mac, ~IIeee80211Mgmt
//
module Ieee80211Mac extends MacProtocolBase like IIeee80211Mac
{
    parameters:
        string mibModule;
        string modeSet @enum("a","b","g(erp)","g(mixed)","n(mixed-2.4Ghz)","p","ac") = default("g(mixed)");
        string fcsMode @enum("declared","computed") = default("declared");
        string initialRadioMode @enum("off","sleep","receiver","transmitter","transceiver") = default("receiver");

        int mtu @unit(B) = default(2304B);
        bool qosStation = default(false);

        *.mibModule = default(absPath(this.mibModule));
        *.rxModule = "^.rx";
        *.txModule = "^.tx";

        @display("i=block/layer");
        @class(Ieee80211Mac);
        @signal[linkBroken](type=inet::Packet); // TODO this signal is only present for the statistic to pass the signal check, to be removed
        @signal[modesetChanged](type=inet::physicallayer::Ieee80211ModeSet);
        @statistic[packetSentToUpper](title="packets sent to upper layer"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetSentToLower](title="packets sent to lower layer"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetReceivedFromUpper](title="packets received from upper layer"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetReceivedFromLower](title="packets received from lower layer"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[linkBroken](title="link breaks"; record=count,vector?; interpolationmode=none);
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
