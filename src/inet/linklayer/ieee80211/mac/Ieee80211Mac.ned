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
// Implementation of the 802.11 MAC protocol. This module is intended
// to be used in combination with the ~Ieee80211Radio module as the physical
// layer.
//
// This module has been heavily modularized to enable customization
// and extension of almost any aspect of its operation, with the aim of
// better supporting experimentation and research. Because of that, its
// features and details of operation are determined by its several
// subcomponents, and can be altered by extending or replacing the
// appropriate subcomponents.
//
// Some noteworthy features are:
//
// - Support for 802.11a/b/g/e/p, as well as partial/incomplete support for
//   later WiFi standards such as n and ac.
//
// - Implements DCF and HCF channel access, including support for QoS (EDCA).
//
// - Support for both infrastructure mode and ad hoc mode operation.
//
// - In infrastructure mode, authentication and association can be
//   implicit, or explicitly modeled using frame exchanges.
//
// - Support for advanced MAC features such as frame aggregation,
//   fragmentation, block acknowledgement and block agreements,
//   rate selection and adaptive rate control algorithms like AARF and
//   ONOE.
//
// - Ability to work with radio models of varying levels of detail,
//   from simple unit disk radio models to layered radio models that
//   explicitly model forward error correction, scrambling, symbol
//   encoding/decoding, etc, and may represent radio signals with a
//   multi-dimensional power density function over time and frequency.
//
// - Support for energy consumption modeling.
//
// The IEEE 802.11 header is represented by ~Ieee80211MacHeader and its
// subclasses.
//
// Note about the implementation:
//
// Despite its appearance, Ieee80211Mac is not a plain compound module.
// Although the bulk of its functionality is outsourced to submodules,
// it is an active module with an underlying C++ class that subclasses
// from cSimpleModule, and receives/sends messages on its own behalf.
//
// Notable omissions:
//
// - PCF is missing
// - HCCA is currently not implemented. (The module exists, but it is
//   currently a placeholder.)
// - Fields related to unsupported features are omitted from frame formats
//   as well (for example, FH/DS/CF parameter sets, beacon/probe timestamp
//   which is related to physical layer synchronization, listen interval
//   which is related to power management, capability information
//   which is related to PCF and other non-modeled features).
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

