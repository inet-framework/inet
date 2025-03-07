//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.shaper;

//
// Implements a Credit-Based Shaper (CBS) as defined in IEEE 802.1Qav for
// Time-Sensitive Networking (TSN). CBS is a traffic shaping mechanism that
// controls bandwidth allocation using a credit-based algorithm.
//
// The credit-based algorithm works as follows:
// - Credits accumulate at a specified rate when the queue is not empty but packets cannot be transmitted
// - Credits decrease when packets are being transmitted
// - Transmission is allowed only when the credit value is non-negative
// - Credits are bounded both from above and below
//
// Key features:
// - Provides fair bandwidth allocation among competing traffic streams
// - Prevents traffic bursts while maintaining guaranteed bandwidth
// - Ensures bounded latency for time-sensitive traffic
// - Compatible with other TSN mechanisms
// - Supports multiple priority levels when used with priority queues
//
// Applications:
// - Audio/Video Bridging (AVB) networks
// - Time-Sensitive Networking (TSN)
// - Industrial automation networks
// - Automotive Ethernet
// - Professional media networks
//
// The credit-based shaper is particularly useful in scenarios where:
// - Bandwidth guarantees are required for specific traffic classes
// - Traffic bursts need to be smoothed to reduce jitter
// - Multiple streams with different priorities need to share limited bandwidth
// - Deterministic network behavior is required
//
// This implementation uses a specialized CreditBasedGate component that
// manages the credit accounting and controls packet transmission based on
// the current credit value.
//
// @see ~CreditBasedGate
//
module CreditBasedShaper extends PacketShaper
{
    parameters:
        gate.typename = default("CreditBasedGate"); // Gate that implements the credit-based algorithm
}
