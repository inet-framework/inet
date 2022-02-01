//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.redundancy;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// This module decodes the stream of a packet by matching various tags on the
// packet against the mapping. It can check for network interface, source address,
// destination address, VLAN id, and PCP. If a match was found then the stream
// is attached to the packet in a ~StreamInd tag.
//
// @see ~StreamEncoder
//
simple StreamDecoder extends PacketFlowBase like IPacketFlow
{
    parameters:
        string interfaceTableModule; // relative module path of the interface table
        object mapping @mutable = default([]); // array of objects, where each object maps a network interface name pattern, a source address, a destination address, a VLAN id and a PCP value to a stream name, for example [{stream: "s1", interface: "eth0", destination: "dest1", pcp: 3}, ...]
        @class(StreamDecoder);
        @display("i=block/blackboard");
}
