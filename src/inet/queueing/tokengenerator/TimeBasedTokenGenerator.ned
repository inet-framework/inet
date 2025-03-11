//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.tokengenerator;

import inet.queueing.base.TokenGeneratorBase;
import inet.queueing.contract.ITokenGenerator;

//
// Generates tokens for a token-based server at regular time intervals.
// Provides a mechanism for controlling packet processing rates based on time,
// which is essential for implementing various rate control and traffic shaping
// algorithms.
//
// Key features:
// - Generates tokens at configurable time intervals
// - Can use simulation time or an external clock module
// - Configurable number of tokens generated per interval
// - Supports both fixed and variable (random) generation intervals
// - Implements a simple token bucket algorithm when used with TokenBasedServer
//
// Operation mechanism:
// 1. The module schedules token generation events at regular intervals
//    specified by the generationInterval parameter
// 2. At each scheduled time, it generates the specified number of tokens (numTokens)
// 3. These tokens are added to the configured token storage module
// 4. The process repeats throughout the simulation
//
// Applications:
// - Implementing token bucket traffic shapers
// - Creating rate-limited packet processors
// - Modeling bandwidth allocation in network devices
// - Simulating time-based resource allocation
// - Implementing leaky bucket rate control
//
// Particularly useful for simulating traffic shaping and policing
// mechanisms that operate based on time rather than packet arrivals or queue states.
// When combined with a TokenBasedServer, it can implement classic traffic management
// algorithms like token bucket and leaky bucket.
//
// @see ~TokenBasedServer, ~PacketBasedTokenGenerator, ~QueueBasedTokenGenerator, ~SignalBasedTokenGenerator
//
simple TimeBasedTokenGenerator extends TokenGeneratorBase like ITokenGenerator
{
    parameters:
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
        volatile double generationInterval @unit(s); // Time interval between two subsequent token generation events
        volatile double numTokens = default(1); // Number of tokens generated per interval (can be fixed or random)
        @class(TimeBasedTokenGenerator);
        @display("i=block/control");
}
