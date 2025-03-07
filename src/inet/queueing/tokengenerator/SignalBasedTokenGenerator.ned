//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.tokengenerator;

import inet.queueing.base.TokenGeneratorBase;
import inet.queueing.contract.ITokenGenerator;

//
// Generates tokens for a token-based server in response to signals
// emitted by other modules in the simulation. Provides a flexible mechanism
// for coordinating packet processing based on events occurring elsewhere in the
// network.
//
// Key features:
// - Subscribes to one or more signals from specified modules
// - Generates tokens when matching signals are received
// - Can filter signals based on specific integer or double values
// - Configurable number of tokens generated per signal
// - Enables event-driven token generation
//
// Operation mechanism:
// 1. The module subscribes to the specified signals from the subscription module
// 2. When a matching signal is received, the module checks if the signal value matches
//    the configured intSignalValue or doubleSignalValue (if specified)
// 3. If the signal value matches or no specific value is configured, the module
//    generates the specified number of tokens (numTokens)
// 4. These tokens are added to the configured token storage module
//
// Applications:
// - Coordinating packet processing with external events
// - Implementing complex control mechanisms across different parts of a network
// - Creating event-driven traffic shaping and policing
// - Synchronizing packet flows with protocol state changes
// - Building reactive queueing systems that respond to network conditions
//
// Particularly useful for creating sophisticated feedback loops
// and control mechanisms that span multiple components in a network simulation.
// For example, it can be used to trigger packet processing in response to
// congestion signals, link state changes, or application-level events.
//
// @see ~TokenBasedServer, ~PacketBasedTokenGenerator, ~QueueBasedTokenGenerator, ~TimeBasedTokenGenerator
//
simple SignalBasedTokenGenerator extends TokenGeneratorBase like ITokenGenerator
{
    parameters:
        string subscriptionModule = default("^"); // Module path to subscribe for signals (default: parent module)
        string signals; // Space-separated list of signal names to subscribe to
        int intSignalValue = default(-1); // Integer signal value to match (-1: match any integer value)
        double doubleSignalValue = default(nan); // Double signal value to match (nan: match any double value)
        volatile double numTokens = default(1); // Number of tokens generated per matching signal
        @class(SignalBasedTokenGenerator);
        @display("i=block/control");
}
