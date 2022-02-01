//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// This is a base module for various packet buffer modules which maintains a few
// statistics.
//
// Available display string format directives:
// - %p - number of packet
// - %l - total length
// - %a - number of added packets
// - %r - number of removed packets
// - %d - number of dropped packets
//
// @see ~IPacketBuffer
//
simple PacketBufferBase extends PacketProcessorBase
{
    parameters:
        displayStringTextFormat = default("contains %p pk (%l)\nadded %a removed %r dropped %d"); // determines display string text above the submodule
        @display("i=block/buffer");
    gates:
        input in @loose;
        output out @loose;
}
