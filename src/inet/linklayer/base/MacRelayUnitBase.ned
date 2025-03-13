//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.base;

//
// Provides a common base module for implementing MAC relay units (switches/bridges)
// in the INET framework. This module handles the basic infrastructure needed by all
// MAC relay units, allowing derived modules to focus on specific switching behaviors.
//
// MAC relay units forward frames between different network interfaces based on
// destination MAC addresses. They maintain a forwarding table that maps MAC addresses
// to outgoing interfaces, which can be populated through learning or configuration.
//
// Key features provided by this base module:
// - Integration with MAC forwarding tables
// - Interface table access for port management
// - Common frame processing infrastructure
// - Broadcast handling capabilities
// - Statistics collection for monitoring
//
// This module serves as the foundation for various switch implementations,
// including basic learning switches, VLAN-aware switches, and more advanced
// bridging devices that implement spanning tree protocols.
//
// @see ~IMacRelayUnit, ~IMacForwardingTable
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
