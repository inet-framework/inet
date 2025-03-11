//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.server;

import inet.queueing.contract.IPacketServer;

//
// Optional placeholder module that implements the IPacketServer interface but
// removes itself from the module hierarchy during initialization. This creates
// a direct connection between the input and output gates.
//
// The OmittedPacketServer is useful in scenarios where:
// - A server is conditionally needed based on simulation parameters
// - Model variants need to be tested with and without processing delays
// - A simplified model is desired without changing the overall network structure
// - Processing delays need to be selectively enabled or disabled
//
// When this module is used, packets flow directly from the input to the output
// without any processing delay or state changes, effectively creating a zero-delay
// passthrough connection.
//
// @see ~PacketServer, ~InstantServer, ~PreemptingServer, ~TokenBasedServer
//
module OmittedPacketServer like IPacketServer
{
    parameters:
        @class(::inet::OmittedModule);
        @display("i=block/star,black,50");
    gates:
        input in;
        output out;
    connections:
        in --> out;
}
