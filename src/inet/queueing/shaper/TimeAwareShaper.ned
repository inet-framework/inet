//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.shaper;

//
// Implements a Time-Aware Shaper (TAS) as defined in IEEE 802.1Qbv for
// Time-Sensitive Networking (TSN). Controls packet transmission based on
// a predefined time schedule, enabling deterministic packet delivery with
// minimal jitter.
//
// The time-aware shaper operates by:
// - Opening and closing a gate according to a configurable time schedule
// - Allowing packets to be transmitted only during specific time intervals
// - Holding packets in the queue when the gate is closed
// - Providing precise timing control for critical traffic streams
//
// Key features:
// - Deterministic packet transmission with bounded latency
// - Elimination of interference between traffic classes
// - Support for time-triggered communication
// - Configurable transmission schedules
// - Compatible with other TSN mechanisms
//
// This implementation uses a PeriodicGate component that opens and closes
// according to a configurable schedule, providing the time-based control
// mechanism required for deterministic packet delivery.
//
// @see ~PeriodicGate, ~CreditBasedShaper, ~AsynchronousShaper
//
module TimeAwareShaper extends PacketShaper
{
    parameters:
        gate.typename = default("PeriodicGate"); // Gate that implements the time-based scheduling
}
