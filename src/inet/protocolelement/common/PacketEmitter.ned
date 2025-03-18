//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.common;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Emits packets as signals that can be observed by other modules. This module
// passes packets through unchanged while emitting matching packets as signals.
// It can filter packets based on the packetFilter parameter and optionally set
// a direction (inbound, outbound, or undefined) for the packets. The module
// registers a signal with the name specified in the signalName parameter and
// emits packets that match the filter as signals. It also adds a direction tag
// to packets if a direction is specified.
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
