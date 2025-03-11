//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.shaper;

import inet.queueing.contract.ITrafficConditioner;

//
// Provides a placeholder implementation of the ITrafficConditioner interface
// that automatically removes itself from the module hierarchy during initialization.
// Creates a direct connection between its input and output gates, effectively
// bypassing any traffic conditioning functionality.
//
// Key features:
// - Self-removing module that creates a direct connection
// - Implements the ITrafficConditioner interface for compatibility
// - Useful for conditional traffic conditioning in network models
// - Zero overhead when traffic conditioning is not needed
//
// Applications:
// - Creating configurable network models where traffic conditioning can be
//   easily enabled or disabled
// - Simplifying model structure when traffic conditioning is not needed
//   in certain scenarios
// - Maintaining consistent module interfaces throughout a model while
//   selectively omitting functionality in specific instances
// - Facilitating parameter studies where the impact of traffic conditioning
//   needs to be evaluated
//
// Usage example:
// In a network configuration, you can use a parameter to conditionally select
// either a functional traffic conditioner or an OmittedTrafficConditioner:
//
// conditioner: <default("OmittedTrafficConditioner")> like ITrafficConditioner if enableConditioner else OmittedTrafficConditioner;
//
// @see ~LeakyBucket, ~TokenBucket, ~PriorityShaper
//
module OmittedTrafficConditioner like ITrafficConditioner
{
    parameters:
        @class(::inet::OmittedModule);
        @display("i=block/downarrow,black,50");
    gates:
        input in;
        output out;
    connections:
        in --> out;
}
