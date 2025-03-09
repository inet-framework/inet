//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.contract;

//
// Defines the interface for packet lifetime management modules that track
// and enforce time-based constraints on packets in the network.
//
// Operation:
// - Monitors packet lifetime from creation or entry point
// - Enforces maximum lifetime constraints
// - Handles expired packets according to protocol requirements
// - Provides timing information for protocol operations
// - Supports time-based packet management policies
//
// @see ~CarrierBasedLifeTimer
//
moduleinterface IPacketLifeTimer
{
    parameters:
        @display("i=block/timer");
}
