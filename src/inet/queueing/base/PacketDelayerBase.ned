//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for packet delayer implementations. Provides common functionality
// for delaying packets for a specified amount of time before forwarding them.
// Supports clock synchronization, scheduling priorities, and zero-delay handling.
// Adds delay-related tags to packets for tracking and analysis.
//
simple PacketDelayerBase extends PacketProcessorBase
{
    parameters:
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
        int schedulingPriority = default(0);
        bool scheduleZeroDelay = default(true);
        @class(PacketDelayerBase);
        @display("i=block/delay");
    gates:
        input in @labels(push);
        output out @labels(push);
}
