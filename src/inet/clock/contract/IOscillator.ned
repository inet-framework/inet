//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.contract;

//
// This module interface is implemented by oscillator models. Oscillators are
// typically simple modules, and are used by other modules via direct C++ method
// calls.
//
// The IOscillator C++ interface provides an API to efficiently simulate the
// generation of ticks that are usually counted by clocks. The tick period is
// not necessarily constant, it can change over time.
//
// @see ~IClock
//
moduleinterface IOscillator
{
    parameters:
        @display("i=block/tunnel");
}

