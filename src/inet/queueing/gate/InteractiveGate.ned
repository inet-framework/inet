//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.gate;

import inet.queueing.base.PacketGateBase;
import inet.queueing.contract.IPacketGate;

//
// Provides a manually controllable gate for packet flows that can be opened
// or closed during simulation runtime through the graphical user interface.
// Enables interactive experimentation with network behavior by allowing
// users to directly influence packet transmission.
//
// Key features:
// - Simple binary open/closed state
// - Runtime control through the GUI while simulation is paused
// - Immediate effect on packet flow when state changes
// - Mutable parameter for programmatic control if needed
//
// Applications:
// - Interactive demonstrations and educational scenarios
// - Debugging complex network behaviors
// - Testing network resilience to link failures
// - Simulating manual network configuration changes
// - Exploring "what-if" scenarios during presentations
// - Validating protocol behavior under controlled conditions
//
// Usage examples:
// - Simulating manual link failures by closing gates at specific points
// - Demonstrating congestion control by selectively enabling traffic flows
// - Testing failover mechanisms by controlling primary/backup path availability
// - Creating interactive network management demonstrations
// - Implementing user-controlled traffic engineering experiments
//
// To use interactively:
// 1. Run the simulation
// 2. Pause at the desired moment
// 3. Right-click on the gate module and select "Contents"
// 4. Change the "open" parameter value
// 5. Continue the simulation to observe the effects
//
simple InteractiveGate extends PacketGateBase like IPacketGate
{
    parameters:
        bool open @mutable = default(true); // Gate state: true=open (allows packets), false=closed (blocks packets)
        @class(InteractiveGate);
}
