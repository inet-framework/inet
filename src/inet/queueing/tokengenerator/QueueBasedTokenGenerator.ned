//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.tokengenerator;

import inet.queueing.base.TokenGeneratorBase;
import inet.queueing.contract.ITokenGenerator;

//
// Generates tokens for a token-based server by monitoring the state
// of a packet queue. Provides a feedback mechanism that can dynamically adjust
// packet processing rates based on queue occupancy levels.
//
// Key features:
// - Monitors a specified packet queue's state (length, occupancy)
// - Generates tokens when the queue's state meets certain criteria
// - Configurable thresholds for minimum packet count and total queue length
// - Can generate a fixed or variable number of tokens per event
// - Particularly useful for maintaining optimal queue utilization
//
// Operation mechanism:
// 1. The module subscribes to state change signals from the specified queue module
// 2. When the queue's packet count falls below minNumPackets AND
//    the queue's total length falls below minTotalLength
// 3. The module generates the specified number of tokens (numTokens)
// 4. These tokens are added to the configured token storage module
//
// Particularly useful in scenarios where you want to ensure
// that a queue always has packets available for processing, which helps
// maximize throughput and utilization of downstream components.
//
// @see ~TokenBasedServer, ~PacketBasedTokenGenerator, ~TimeBasedTokenGenerator, ~SignalBasedTokenGenerator
//
simple QueueBasedTokenGenerator extends TokenGeneratorBase like ITokenGenerator
{
    parameters:
        string queueModule; // Module path of the observed packet queue
        int minNumPackets = default(1);
        int minTotalLength @unit(b) = default(0 b);
        volatile double numTokens = default(1); // Number of tokens generated when the queue becomes empty
        @class(QueueBasedTokenGenerator);
        @display("i=block/control");
}
