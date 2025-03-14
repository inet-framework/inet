//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for packet buffer implementations in the queueing framework.
// Packet buffers store packets on behalf of multiple queues, modeling bounded
// shared storage space. They are typically used to implement different packet
// dropping strategies when the shared storage capacity is reached.
//
// Packet buffers track:
// - Current number of stored packets and their total length
// - Number of packets added to the buffer
// - Number of packets removed from the buffer
// - Number of packets dropped due to buffer management policies
//
// Derived modules implement specific buffer management strategies such as
// dropping packets based on priority, size, or other criteria.
//
// Available display string format directives:
// - %p - number of packets
// - %l - total length
// - %a - number of added packets
// - %r - number of removed packets
// - %d - number of dropped packets
//
// @see ~IPacketBuffer, ~PacketBuffer, ~PriorityBuffer
//
simple PacketBufferBase extends PacketProcessorBase
{
    parameters:
        displayStringTextFormat = default("contains %p pk (%l)\nadded %a removed %r dropped %d"); // Determines display string text above the submodule
        @display("i=block/buffer");
    gates:
        input in @loose;
        output out @loose;
}
