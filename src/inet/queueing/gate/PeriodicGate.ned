//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.gate;

import inet.queueing.base.PacketGateBase;
import inet.queueing.contract.IPacketGate;

//
// Implements a cyclic time-based gate control mechanism as defined in IEEE 802.1Qbv
// for Time-Sensitive Networking (TSN). Controls packet flow according to a repeating
// pattern of open and closed states with configurable durations.
//
// Operation mechanism:
// - Cycles through a list of time durations, alternating between open and closed states
// - The complete cycle repeats periodically (period = sum of all durations)
// - Configurable offset determines the starting point within the cycle
// - Optional guard band prevents transmission that would extend beyond the gate's open period
// - Can synchronize with an external clock module for distributed simulations
//
// Key features:
// - Implements IEEE 802.1Qbv Time-Aware Shaper functionality
// - Provides deterministic packet transmission with bounded latency
// - Supports configurable cycle times and transmission windows
// - Enables precise control over traffic timing
// - Guard band functionality prevents packet fragmentation across cycle boundaries
//
// Example configuration:
// - durations = [0.3ms, 0.7ms, 0.5ms, 0.5ms]
// - initiallyOpen = true
//
// This creates a 2ms cycle where the gate is open for 0.3ms, closed for 0.7ms,
// open for 0.5ms, and closed for 0.5ms, then repeats.
//
// @see ~GateControlList, ~TimeAwareShaper, ~PacketGate
//
simple PeriodicGate extends PacketGateBase like IPacketGate
{
    parameters:
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
        bool initiallyOpen @mutable = default(true); // Specifies if the gate is initially open or closed
        double offset @mutable @unit(s) = default(0s); // Specifies where the period starts in the list of gate state change durations; where it is in the period at the start
        object durations @mutable @unit(s) = default([]); // List of time durations within the period that determine if the gate is open or closed; period is the sum of the durations
        bool scheduleForAbsoluteTime = default(true); // When a clock is used relative means that setting the clock will not affect the simulation time of the event
        int openSchedulingPriority = default(0); // FES scheduling priority for the next gate open event
        int closeSchedulingPriority = default(0); // FES scheduling priority for the next gate close event
        bool enableImplicitGuardBand = default(true); // Implicit guard band means that a packet is not allowed to be forwarded if it cannot finish transmission before the end of window.
        @class(PeriodicGate);
        @signal[guardBandStateChanged](type=bool);
        @statistic[guardBandState](title="guard band state"; type=enum; enum=ON, OFF; source=guardBandStateChanged; record=vector; interpolationmode=sample-hold);
}
