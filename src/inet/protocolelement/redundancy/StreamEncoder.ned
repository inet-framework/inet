//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.redundancy;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Translates stream identifiers into network parameters by attaching appropriate
// request tags to packets. Enables stream-based communication by mapping logical
// stream identifiers to network-level attributes.
//
// Operation:
// - Examines packets for stream identification
// - Maps stream names to network parameters using configurable mapping
// - Attaches request tags (e.g., VLAN ID, PCP) based on stream identity
// - Maintains packet flow while adding network parameter requests
// - Provides the sending side of stream-based communication
//
// Particularly useful for:
// - Implementing stream-based transmission in redundant networks
// - Creating stream-aware protocol stacks
// - Supporting Quality of Service differentiation
// - Building systems with multiple redundant paths
// - Enabling stream-based packet processing and routing
//
// @see ~StreamDecoder, ~StreamCoderLayer
//
simple StreamEncoder extends PacketFlowBase like IPacketFlow
{
    parameters:
        object mapping @mutable = default([]); // Array of objects, where each object maps a stream name to a VLAN ID and PCP value, for example [{stream: "s1", vlan: 12, pcp: 7}, ...]
        @class(StreamEncoder);
        @display("i=block/blackboard");
}
