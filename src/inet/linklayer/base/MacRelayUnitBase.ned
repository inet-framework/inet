//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.base;

//
// Base module for MAC relay unit implementations (Ethernet switches). Provides
// core functionality for MAC address learning and frame forwarding in bridged
// Ethernet networks. Maintains a MAC address table that maps addresses to ports,
// learns addresses from incoming frames, and forwards frames to appropriate
// interfaces. Handles unicast, multicast, and broadcast traffic according to
// standard Ethernet switching rules. Tracks statistics for processed and dropped
// frames. Extended by concrete implementations like MacRelayUnit.
//
simple MacRelayUnitBase
{
    parameters:
        string macTableModule;
        string interfaceTableModule;
        string displayStringTextFormat = default("proc: %p\ndisc: %d");
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
