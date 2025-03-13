//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.shaper;

//
// Implements an asynchronous shaper as defined in IEEE 802.1Q-2018 (formerly
// IEEE 802.1Qcr) for Time-Sensitive Networking (TSN). Provides deterministic
// latency bounds while allowing for efficient network utilization.
//
// The asynchronous shaper operates based on per-packet eligibility times:
// - Each packet is assigned a transmission eligibility time (~EligibilityTimeTag)
// - Packets are stored in a queue sorted by their eligibility times
// - The gate prevents packets from being transmitted before their eligibility time
// - When a packet becomes eligible, it is released for transmission
//
// Key features:
// - Provides bounded latency guarantees without synchronized clocks
// - Supports traffic shaping with minimal jitter
// - Enables efficient bandwidth utilization
// - Compatible with other TSN mechanisms
// - Preserves packet ordering within flows
//
// This implementation uses specialized queue and gate components:
// - EligibilityTimeQueue: Sorts packets based on their eligibility times
// - EligibilityTimeGate: Releases packets only after they become eligible
//
// @see ~EligibilityTimeMeter, ~EligibilityTimeFilter, ~EligibilityTimeQueue, ~EligibilityTimeGate
//
module AsynchronousShaper extends PacketShaper
{
    parameters:
        queue.typename = default("EligibilityTimeQueue"); // Queue that sorts packets by eligibility time
        gate.typename = default("EligibilityTimeGate");   // Gate that enforces eligibility time constraints
}
