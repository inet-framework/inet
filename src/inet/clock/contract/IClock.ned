//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.contract;

//
// This module interface is implemented by clock models. Clocks are typically
// modules, and are used by other modules via direct C++ method calls.
//
// The IClock C++ interface provides an API similar to the standard OMNeT++
// simulation time based scheduling mechanism but relies on the underlying
// clock implementation for (re)scheduling events according to the clock.
// These events are transparently scheduled for the client module and will be
// delivered to it when the clock timer expires.
//
// The typical way to use clock modules is to include a clock submodule somewhere
// in the model (e.g. in the network interfaces, in the network nodes) and add
// optional module parameters (i.e. module path to the clock module) to modules
// using the clock.
//
// For example, having a clock submodule in the model of a network interface
// and using it for all timing (instead of the standard OMNeT++ simulation time
// based scheduling mechanism) allows for modeling clock skew and clock drift
// and their effect on the link layer protocol performance.
//
// @see ~IOscillator
//
moduleinterface IClock
{
    parameters:
        @display("i=block/timer");
}

