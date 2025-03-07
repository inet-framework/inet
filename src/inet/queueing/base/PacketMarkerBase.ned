//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for various packet marker modules in the queueing framework.
// Packet markers are used to attach tags to packets as they flow through the network,
// enabling various forms of packet processing, classification, and quality of service.
//
// Packet markers serve several important functions in network simulations:
// - Identifying packets for special handling (e.g., priority treatment)
// - Marking packets for congestion control (e.g., ECN marking)
// - Adding metadata for routing or processing decisions
// - Supporting quality of service implementations
// - Enabling traffic engineering and policy enforcement
//
// Derived modules must implement a single `markPacket()` function which marks
// individual packets by attaching appropriate tags. The specific marking behavior
// is determined by the derived module's implementation.
//
// Packet markers typically operate as transparent elements in a packet processing
// pipeline - they modify packets by attaching tags but do not alter the packet flow.
// Packets are pushed into the marker's input and are immediately pushed to its output
// after being marked.
//
// @see ~IPacketMarker, ~PacketLabeler, ~EcnMarker, ~RedMarker, ~PacketTagger
//
simple PacketMarkerBase extends PacketFlowBase
{
    parameters:
        displayStringTextFormat = default("marked %p pk (%l)");
        @class(PacketMarkerBase);
}
