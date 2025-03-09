//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.redundancy;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Identifies packet streams based on network parameters and attaches stream
// identification tags. Enables stream-based processing by mapping network-level
// attributes to logical stream identifiers.
//
// Operation:
// - Examines incoming packets for network interface, addresses, VLAN ID, and PCP
// - Matches packet attributes against configurable mapping rules
// - Attaches StreamInd tags to packets when matches are found
// - Maintains packet flow while adding stream identification
// - Provides the receiving side of stream-based communication
//
// @see ~StreamEncoder, ~StreamCoderLayer
//
simple StreamDecoder extends PacketFlowBase like IPacketFlow
{
    parameters:
        string interfaceTableModule; // Relative module path of the interface table
        object mapping @mutable = default([]); // Array of objects, where each object maps a network interface name pattern, a source address, a destination address, a VLAN id, and a PCP value to a stream name, for example [{stream: "s1", interface: "eth0", destination: "dest1", pcp: 3}, ...]
        @class(StreamDecoder);
        @display("i=block/blackboard");
}
