//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.common;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

simple PacketEmitter extends PacketFlowBase like IPacketFlow
{
    parameters:
        object packetFilter = default("*"); // which packets are considered, matches all packets by default
        string direction @enum("undefined", "inbound", "outbound") = default("undefined");
        string signalName;
        @class(PacketEmitter);
        @display("i=block/star");
        @signal[packetSentToLower](type=Packet);
        @signal[packetReceivedFromLower](type=Packet);
}
