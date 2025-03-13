//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.acking;

import inet.linklayer.base.MacProtocolBase;
import inet.linklayer.contract.ILinkLayer;
import inet.linklayer.contract.IMacProtocol;

//
// Implements a simplified MAC protocol with optional acknowledgments, designed
// for use in idealized wireless network simulations where detailed MAC behavior
// is not important. This protocol provides basic packet encapsulation and
// addressing functionality without the complexity of real-world MAC protocols.
//
// Key features:
// - Simple packet encapsulation and decapsulation
// - Optional out-of-band acknowledgment mechanism
// - Configurable bitrate and full/half duplex operation
// - No carrier sensing, collision avoidance, or collision detection
// - No backoff mechanism or retransmission strategy
//
// This protocol is particularly useful for:
// - Testing higher layer protocols without MAC layer interference
// - Simulating idealized wireless networks with perfect transmission
// - Educational scenarios where MAC complexity would be distracting
// - Rapid prototyping of wireless network applications
//
// Higher layer packets must include a `MacAddressReq` tag for proper addressing.
//
// @see ~AckingWirelessInterface, ~UnitDiskRadioMedium
//
module AckingMac extends MacProtocolBase like ILinkLayer, IMacProtocol
{
    parameters:
        string address @mutable = default("auto");   // MAC address as a hex string (12 hex digits), or
                                                     // "auto". "auto" values will be replaced by
                                                     // a generated MAC address in init stage 0.
        double bitrate @unit(bps);
        int mtu @unit(B) = default(4470B);
        int headerLength @unit(B) = default(16B); // AckingMacHeader length
        bool promiscuous = default(false);
        bool fullDuplex = default(true);    // Allows transmitting and receiving simultaneously (transceiver radio mode)
        bool useAck = default(true);
        double ackTimeout @unit(s) = default(100ms);

        @class(AckingMac);
        @signal[linkBroken](type=inet::Packet);
        @statistic[linkBroken](title="link break"; source=linkBroken; record=count; interpolationmode=none);
        @statistic[passedUpPk](title="packets passed to higher layer"; source=packetSentToUpper; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[sentDownPk](title="packets sent to lower layer"; source=packetSentToLower; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rcvdPkFromHl](title="packets received from higher layer"; source=packetReceivedFromUpper; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rcvdPkFromLl](title="packets received from lower layer"; source=packetReceivedFromLower; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetDropIncorrectlyReceived](title="packet drop: incorrectly received"; source=packetDropReasonIsIncorrectlyReceived(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropInterfaceDown](title="packet drop: interface down"; source=packetDropReasonIsInterfaceDown(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropNotAddressedToUs](title="packet drop: not addressed to us"; source=packetDropReasonIsNotAddressedToUs(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
}
