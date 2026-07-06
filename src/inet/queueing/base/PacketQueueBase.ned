//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for various packet queue modules which maintains a few
// statistics.
//
// Available display string format directives:
// - %p - number of packets
// - %l - total length
// - %u - number of pushed packets
// - %o - number of pulled packets
// - %r - number of removed packets
// - %d - number of dropped packets
// - %n - name of the next pulled packet
//
// @see ~IPacketQueue
//
simple PacketQueueBase extends PacketProcessorBase
{
    parameters:
        @class(-);  //# set to '-' to prevent unintentional instantiation. Also disable the linter: suppress(missing-cpp-class)
        displayStringTextFormat = default("contains {numPackets} pk ({totalLength})\npushed {numPushedPackets} pulled {numPulledPackets} dropped {numDroppedPackets}"); // Determines display string text above the submodule
        @display("i=block/queue");
    gates:
        input in @loose;
        output out @loose;
}
