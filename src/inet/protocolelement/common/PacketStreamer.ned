//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.common;

import inet.queueing.base.PacketProcessorBase;
import inet.queueing.contract.IPacketFlow;

//
// Converts complete packets into a stream of packet fragments by breaking
// packets into start, progress, and end events for transmission over time.
//
// Operation:
// - Receives complete packets at its input
// - Breaks packets into streaming fragments (start, progress, end)
// - Forwards these fragments through its output at a configurable datarate
// - Supports both push and pull packet processing modes
//
// Particularly useful for:
// - Simulating gradual packet transmission over physical media
// - Converting from packet-based interfaces to streaming-based interfaces
// - Implementing realistic transmission timing in network simulations
// - Modeling bit-by-bit or byte-by-byte transmission processes
// - Creating protocol adapters between different network layers
//
// @see ~PacketDestreamer, ~PacketProcessorBase, ~IPacketFlow
//
simple PacketStreamer extends PacketProcessorBase like IPacketFlow
{
    parameters:
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
        double datarate @unit(bps) = default(nan bps);
        @class(PacketStreamer);
        @display("i=block/cogwheel");
    gates:
        input in @labels(send,push,pull,pass);
        output out @labels(send,push,pull,stream);
}
