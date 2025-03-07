//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.gate;

//
// Implements the Gate Control List (GCL) mechanism as defined in IEEE 802.1Qbv
// for Time-Sensitive Networking (TSN). Configures multiple PeriodicGate modules
// according to a time-based schedule to enable deterministic packet transmission.
//
// The Gate Control List defines a cyclic schedule that determines when each gate
// is open or closed. This enables precise control over when different traffic
// classes can transmit, creating time-based isolation between traffic streams.
//
// Operation mechanism:
// - Configures multiple PeriodicGate modules located at ^.transmissionGate[*]
// - Each entry in the GCL specifies a duration and gate states for all gates
// - Gate states are represented as binary strings (e.g., "10000000" means gate 0 is open)
// - The mapping parameter defines how gate state bits map to gate indices
// - The schedule repeats cyclically after all entries have been processed
//
// Key features:
// - Implements IEEE 802.1Qbv Time-Aware Shaper functionality
// - Provides deterministic packet transmission with bounded latency
// - Enables time-based isolation between traffic classes
// - Supports configurable cycle times and gate schedules
// - Allows for complex traffic shaping patterns
//
// Applications:
// - Time-Sensitive Networking (TSN)
// - Industrial automation requiring deterministic communication
// - Automotive Ethernet networks
// - Professional audio/video networks
// - Mission-critical systems requiring deterministic packet delivery
//
// Example configuration:
// - durations = [4ms, 2ms, 2ms, 2ms]
// - gateStates = ["10000000", "01111111", "10000000", "01111111"]
// - mapping = "76543210"
//
// This creates a 10ms cycle where gate 0 is open for 4ms, then gates 1-7 are open
// for 2ms, then gate 0 is open for 2ms, then gates 1-7 are open for 2ms.
//
// @see ~PeriodicGate, ~TimeAwareShaper
//
simple GateControlList
{
    parameters:
        object durations @unit(s) = default([]); // List of entry durations, e.g. [4ms, 2ms, 2ms, 2ms]
        object gateStates = default([]); // List of gate states, e.g. ["10000000", "01111111", "10000000", "01111111"]
        int numGates = default(8); // Number of configured PeriodicGate modules
        string mapping; // String specifying the gate index for the ith gate state, e.g. "76543210" means reverse gate index order
        @class(GateControlList);
        @display("i=block/table");
}
