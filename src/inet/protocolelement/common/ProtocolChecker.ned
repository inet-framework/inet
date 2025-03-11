//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.common;

import inet.queueing.base.PacketFilterBase;

//
// Filters packets based on their protocol, allowing only packets with
// registered protocols to pass through while dropping others.
//
// Operation:
// - Maintains a set of registered protocols
// - Examines each packet's PacketProtocolTag
// - Passes packets with recognized protocols
// - Drops packets with unrecognized protocols or missing protocol tags
// - Emits signals when packets are dropped
//
// Particularly useful for:
// - Enforcing protocol compliance at layer boundaries
// - Implementing protocol-specific filtering in network nodes
// - Detecting and handling protocol errors in simulations
// - Validating packet flows in complex protocol stacks
// - Testing protocol implementation correctness
//
// @see ~PacketFilterBase
//
simple ProtocolChecker extends PacketFilterBase
{
    parameters:
        @class(ProtocolChecker);
        @display("i=block/checker");
        @signal[packetDropped](type=inet::Packet);
}
