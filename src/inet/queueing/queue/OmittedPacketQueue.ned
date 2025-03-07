//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.queue;

import inet.queueing.contract.IPacketQueue;

//
// Optional placeholder module that implements the IPacketQueue interface but
// removes itself from the module hierarchy during initialization. This creates
// a direct connection between the input and output gates.
//
// The OmittedPacketQueue is useful in scenarios where:
// - A queue is conditionally needed based on simulation parameters
// - Model variants need to be tested with and without queuing
// - A simplified model is desired without changing the overall network structure
//
// When this module is used, packets flow directly from the input to the output
// without any queuing delay or processing, effectively creating a zero-delay
// passthrough connection.
//
// @see ~PacketQueue, ~DropTailQueue
//
module OmittedPacketQueue like IPacketQueue
{
    parameters:
        @class(::inet::OmittedModule);
        @display("i=block/queue,black,50");
    gates:
        input in;
        output out;
    connections:
        in --> out;
}
