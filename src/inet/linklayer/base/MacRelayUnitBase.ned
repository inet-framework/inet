//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.base;

import inet.common.SimpleModule;

//
// Abstract base module for MAC relay unit implementations (Ethernet switches).
// Provides core functionality for MAC address learning and frame forwarding in
// bridged Ethernet networks. Contains functionality for maintaining a MAC
// address table that maps addresses to ports, learning addresses from incoming
// frames, and forwarding frames to appropriate interfaces. It is able to handle
// unicast, multicast, and broadcast traffic according to standard Ethernet
// switching rules.
//
// @see IMacForwardingTable
//
simple MacRelayUnitBase extends SimpleModule
{
    parameters:
        @class(MacRelayUnitBase);
        string macTableModule;
        string interfaceTableModule;
        displayStringTextFormat = default("proc: %p\ndisc: %d");
        @display("i=block/switch");
        @signal[packetSentToLower](type=inet::Packet);
        @signal[packetReceivedFromUpper](type=inet::Packet);
        @signal[packetReceivedFromLower](type=inet::Packet);
        @signal[packetDropped](type=inet::Packet);
        @statistic[packetDropped](title="packets dropped"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
    gates:
        input lowerLayerIn;
        output lowerLayerOut;
        input upperLayerIn;
        output upperLayerOut;
}

