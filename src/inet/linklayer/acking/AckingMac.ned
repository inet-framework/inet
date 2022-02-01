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
// This module implements a trivial MAC protocol for use in ~AckingWirelessInterface.
//
// The implementation provides packet encapsulation and decapsulation, but it
// doesn't have a real medium access protocol. It doesn't provide carrier sense
// mechanism, collision avoidance, collision detection, but it provides optional
// out-of-band acknowledgement. Higher layer packets should have MacAddressReq
// tag.
//
// See ~AckingWirelessInterface for rationale and details.
//
module AckingMac extends MacProtocolBase like ILinkLayer, IMacProtocol
{
    parameters:
        string address @mutable = default("auto");   // MAC address as hex string (12 hex digits), or
                                                     // "auto". "auto" values will be replaced by
                                                     // a generated MAC address in init stage 0.
        double bitrate @unit(bps);
        int mtu @unit(B) = default(4470B);
        int headerLength @unit(B) = default(16B); // AckingMacHeader length
        bool promiscuous = default(false);
        bool fullDuplex = default(true);    // allows transmitting and receiving simultaneously (transceiver radio mode)
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

