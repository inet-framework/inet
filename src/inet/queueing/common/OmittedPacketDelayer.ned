//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.contract.IPacketDelayer;

//
// Provides a placeholder implementation of the IPacketDelayer interface that
// removes itself from the module hierarchy during initialization. Serves as a
// structural element that allows for optional packet delaying functionality
// in network models.
//
// Key features:
// - Implements the IPacketDelayer interface
// - Self-removes during initialization phase
// - Creates a direct connection between input and output
// - Zero impact on simulation performance
// - Maintains model structure compatibility
//
// Operation mechanism:
// - During module initialization, replaces itself with a direct connection
// - Packets flow directly from input to output without any delay
// - No packet processing occurs
//
// Applications:
// - Creating flexible network models where packet delay is optional
// - Maintaining consistent model structure across different configurations
// - Simplifying model parameterization without changing topology
// - Allowing for easy enabling/disabling of packet delay in simulations
// - Supporting gradual model refinement
//
// This module is part of INET's "omitted" module family, which provides
// placeholder implementations that can be easily replaced with functional
// modules when needed, without changing the overall model structure.
//
// @see ~PacketDelayer, ~OmittedPacketQueue, ~OmittedPacketServer, ~OmittedPacketFlow
//
module OmittedPacketDelayer like IPacketDelayer
{
    parameters:
        @class(::inet::OmittedModule);
        @display("i=block/delay,black,50");
    gates:
        input in;
        output out;
    connections:
        in --> out;
}
