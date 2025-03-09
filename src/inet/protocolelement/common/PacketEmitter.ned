//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.common;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Emits signals for packets that match a specified filter, allowing for
// packet monitoring and analysis without affecting packet flow.
//
// Operation:
// - Filters packets based on a configurable expression
// - Emits signals for matching packets using the specified signal name
// - Can set packet direction tags (inbound, outbound, or undefined)
// - Passes packets through without modification
//
// @see ~PacketFlowBase, ~IPacketFlow
//
simple PacketEmitter extends PacketFlowBase like IPacketFlow
{
    parameters:
        object packetFilter = default("*"); // Which packets are considered, matches all packets by default
        string direction @enum("undefined", "inbound", "outbound") = default("undefined");
        string signalName;
        @class(PacketEmitter);
        @display("i=block/star");
        @signal[packetSentToLower](type=Packet);
        @signal[packetReceivedFromLower](type=Packet);
}
