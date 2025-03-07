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
