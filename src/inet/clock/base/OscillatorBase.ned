//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.base;

//
// Base module for oscillators.
//
simple OscillatorBase
{
    parameters:
        string displayStringTextFormat = default("%n"); // Determines the text that is written on top of the submodule
        @class(OscillatorBase);
        @display("i=block/tunnel");
        @signal[driftRateChanged](type=double);
        @statistic[driftRateChanged](title="Oscillator drift rate"; record=vector; interpolationmode=sample-hold; unit=ppm);
}

