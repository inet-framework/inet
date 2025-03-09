//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.fragmentation;

import inet.queueing.base.PacketProcessorBase;
import inet.queueing.contract.IPacketFlow;

//
// Streams packets to its output with support for preemption, allowing
// high-priority packets to interrupt the streaming of lower-priority packets.
// Implements frame preemption similar to IEEE 802.1Qbu.
//
// Operation:
// - Converts packets into streams for transmission
// - Supports interruption of packet streaming for higher priority traffic
// - Ensures minimum packet fragment size for preemption
// - Rounds fragment sizes to specified boundaries
// - Provides configurable datarate for streaming
//
// @see ~PacketStreamer, ~IPacketFlow
//
simple PreemptableStreamer extends PacketProcessorBase like IPacketFlow
{
    parameters:
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
        double datarate @unit(bps) = default(nan bps);
        int minPacketLength @unit(b);
        int roundingLength @unit(b) = default(1B);
        @class(PreemptableStreamer);
        @display("i=block/cogwheel");
    gates:
        input in @labels(send,push,pull,pass);
        output out @labels(send,push,pull,stream);
}
