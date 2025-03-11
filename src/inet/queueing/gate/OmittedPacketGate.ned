//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.gate;

import inet.queueing.contract.IPacketGate;

//
// Provides a placeholder implementation of the IPacketGate interface
// that automatically removes itself from the module hierarchy during initialization.
// Creates a direct connection between its input and output gates, effectively
// bypassing any gate functionality.
//
// Key features:
// - Self-removing module that creates a direct connection
// - Implements the IPacketGate interface for compatibility
// - Zero overhead when gate functionality is not needed
// - Simplifies model structure by eliminating unnecessary components
//
// Applications:
// - Creating configurable network models where gates can be conditionally included
// - Maintaining consistent module interfaces throughout a model while selectively
//   omitting functionality in specific instances
// - Facilitating parameter studies where the impact of gates needs to be evaluated
// - Simplifying complex models by removing unnecessary processing steps
// - Supporting gradual model development by providing placeholders for future components
//
// Usage example:
// In a network configuration, you can use a parameter to conditionally select
// either a functional gate or an OmittedPacketGate:
//
// gate: <default("OmittedPacketGate")> like IPacketGate if enableGate else OmittedPacketGate;
//
// @see ~PacketGate, ~PeriodicGate, ~CreditBasedGate
//
module OmittedPacketGate like IPacketGate
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
