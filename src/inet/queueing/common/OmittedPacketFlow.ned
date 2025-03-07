//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.contract.IPacketFlow;

//
// Provides a placeholder implementation of the IPacketFlow interface that
// removes itself from the module hierarchy during initialization. Enables
// flexible network designs where packet flow processing can be optionally
// included or excluded without changing the model structure.
//
// Key features:
// - Implements the IPacketFlow interface
// - Self-removes during initialization phase
// - Creates a direct connection between input and output
// - Zero impact on simulation performance
// - Maintains model structure compatibility
//
// Operation mechanism:
// - During module initialization, replaces itself with a direct connection
// - Packets flow directly from input to output without any processing
// - No packet modification, filtering, or state tracking occurs
//
// Applications:
// - Creating configurable network models where packet flow processing is optional
// - Maintaining consistent model structure across different simulation scenarios
// - Simplifying model parameterization without changing topology
// - Supporting gradual model refinement and complexity management
// - Enabling easy A/B testing of different packet processing configurations
//
// This module is part of INET's "omitted" module family, which provides
// placeholder implementations that can be easily replaced with functional
// modules when needed, without changing the overall model structure.
//
// @see ~PacketFlow, ~OmittedPacketQueue, ~OmittedPacketServer, ~OmittedPacketDelayer
//
module OmittedPacketFlow like IPacketFlow
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
