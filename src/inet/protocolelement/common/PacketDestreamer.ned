//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.common;

import inet.queueing.base.PacketProcessorBase;
import inet.queueing.contract.IPacketFlow;

//
// Converts streamed packet fragments back into complete packets by collecting
// packet start, progress, and end events into a single packet object.
//
// Operation:
// - Receives packet streaming events (start, progress, end) at its input
// - Reconstructs the complete packet from these streaming fragments
// - Forwards the complete packet through its output when streaming ends
// - Supports both push and pull packet processing modes
//
// @see ~PacketStreamer, ~PacketProcessorBase, ~IPacketFlow
//
simple PacketDestreamer extends PacketProcessorBase like IPacketFlow
{
    parameters:
        double datarate @unit(bps) = default(nan bps);
        @class(PacketDestreamer);
        @display("i=block/cogwheel");
    gates:
        input in @labels(send,push,pull,stream);
        output out @labels(send,push,pull,pass);
}
