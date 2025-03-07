//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for packet tagger components in the queueing framework.
// Packet taggers attach protocol-specific tags to packets, enabling
// proper handling by protocol layers and network devices.
//
// Packet taggers are essential for implementing:
// - Quality of Service (QoS) mechanisms through DSCP/TOS/ECN marking
// - Virtual LAN (VLAN) tagging and priority control
// - Interface selection for multi-homed devices
// - Hop limit configuration for network layer protocols
// - Transmission power settings for wireless communications
// - Protocol encapsulation specifications
//
// Taggers operate transparently in packet processing pipelines by:
// 1. Receiving packets (either pushed to their input or pulled from their output)
// 2. Attaching appropriate protocol-specific tags based on configuration
// 3. Forwarding packets without changing their content or timing
//
// The module supports a wide range of protocol-specific parameters:
// - DSCP (Differentiated Services Code Point) for IP QoS marking
// - ECN (Explicit Congestion Notification) for congestion signaling
// - TOS (Type of Service) for IPv4 or Traffic Class for IPv6
// - VLAN ID and PCP (Priority Code Point) for Ethernet frames
// - User priority for 802.11 frames
// - Interface selection for routing decisions
// - Hop limit (TTL) for controlling packet lifetime
// - Transmission power for physical layer configuration
// - Encapsulation protocol specifications
//
// Derived tagger modules can implement various tagging strategies including:
// - Static tagging based on configuration
// - Content-based tagging using packet inspection
// - Dynamic tagging based on network conditions
//
// @see ~PacketTagger, ~ContentBasedTagger, ~PacketLabeler
//
simple PacketTaggerBase extends PacketMarkerBase
{
    parameters:
        int dscp = default(-1); // Differentiated Services Code Point for IP QoS marking (-1 means no change)
        int ecn = default(-1); // Explicit Congestion Notification value for congestion signaling (-1 means no change)
        int tos = default(-1); // Type Of Service (IPv4) / Traffic Class (IPv6) field value (-1 means no change)
        string interfaceName = default(""); // Outgoing interface name for routing decisions (empty means no change)
        int hopLimit = default(-1); // Network layer TTL/hop limit value (-1 means no change)
        int vlanId = default(-1); // Virtual LAN identifier for Ethernet frames (-1 means no change)
        int pcp = default(-1); // Priority Code Point for VLAN frames (-1 means no change)
        int userPriority = default(-1); // User priority for 802.11 frames (-1 means no change)
        double transmissionPower @unit(W) = default(nan W); // Signal transmission power for physical layer (nan means no change)
        object encapsulationProtocols = default([]); // Protocol encapsulation specifications (empty means no change)
        displayStringTextFormat = default("tagged %p pk (%l)");
        @class(PacketTaggerBase);
        @display("i=block/star");
}
