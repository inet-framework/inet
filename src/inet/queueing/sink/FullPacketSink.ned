//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.sink;

import inet.queueing.base.PacketProcessorBase;
import inet.queueing.contract.IActivePacketSink;
import inet.queueing.contract.IPacketSink;
import inet.queueing.contract.IPassivePacketSink;

//
// Special packet sink that simulates a full or congested destination. Rejects
// all incoming packets by generating permanent backpressure on its input gate,
// effectively modeling a destination that cannot accept any more packets.
//
// Key features:
// - Implements both active and passive sink interfaces for compatibility
// - Always generates backpressure for push operations
// - Never pulls packets from connected modules
// - Simulates a permanently congested or unavailable destination
//
// @see ~ActivePacketSink, ~PassivePacketSink
//
simple FullPacketSink extends PacketProcessorBase like IPacketSink, IActivePacketSink, IPassivePacketSink
{
    parameters:
        @class(FullPacketSink);
        @display("i=block/sink");
    gates:
        input in @labels(push,pull);
}
