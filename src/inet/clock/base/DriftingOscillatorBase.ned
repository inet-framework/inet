//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.base;

//
// This is a base module for oscillators that drift relative to the nominal tick
// length over time.
//
simple DriftingOscillatorBase extends OscillatorBase
{
    parameters:
        displayStringTextFormat = default("%c (%n + %d)"); // determines the text that is written on top of the submodule
        double nominalTickLength @unit(s) = default(0s); // 0 means simulation time precision
        double tickOffset @unit(s) = default(0s); // shifts ticks to the past, must be in the range of [0, current tick length)
        @signal[preOscillatorStateChanged];
        @signal[postOscillatorStateChanged];
        @class(DriftingOscillatorBase);
}

