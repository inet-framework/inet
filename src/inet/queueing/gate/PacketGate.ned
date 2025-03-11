//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.gate;

import inet.queueing.base.PacketGateBase;
import inet.queueing.contract.IPacketGate;

//
// Provides a basic gate mechanism that controls packet flow based on
// scheduled open and close times. Enables precise temporal control over
// when packets can pass through a network path.
//
// Operation mechanism:
// - Controls packet flow with a binary open/closed state
// - Opens at specified openTime and closes at closeTime
// - When open, packets can pass through; when closed, packets are blocked
// - Can be configured to start in either open or closed state
// - Optionally synchronizes with an external clock module
//
// Key features:
// - Time-based packet flow control
// - Configurable initial state
// - Precise scheduling of open/close operations
// - Optional clock synchronization for distributed simulations
// - Simple binary operation model
//
// Applications:
// - Modeling time-division multiplexing systems
// - Simulating scheduled network access
// - Creating controlled traffic patterns
// - Testing protocol behavior under intermittent connectivity
// - Implementing basic time-aware shaping
// - Modeling duty-cycled networks
//
// Usage examples:
// - Implementing time slots in TDMA networks
// - Creating periodic connectivity patterns
// - Simulating scheduled access in industrial networks
// - Modeling energy-saving protocols with periodic wake/sleep cycles
// - Testing application resilience to intermittent connectivity
//
// @see ~PeriodicGate, ~CreditBasedGate, ~InteractiveGate
//
simple PacketGate extends PacketGateBase like IPacketGate
{
    parameters:
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
        bool initiallyOpen = default(false); // Whether the gate is initially open or closed
        double openTime @unit(s); // Time when the gate opens
        double closeTime @unit(s); // Time when the gate closes
        @class(PacketGate);
}
