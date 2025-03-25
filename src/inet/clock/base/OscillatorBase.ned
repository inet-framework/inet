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
        bool emitNumTicksSignal = default(false); // enables emitting a signal at each oscillator tick, can be useful for validating the behavior of the more efficient implementation without events
        @class(OscillatorBase);
        @display("i=block/tunnel");
        @signal[numTicksChanged](type=long);
        @signal[driftRateChanged](type=double);
        @statistic[numTicksChanged](title="number of ticks"; record=vector; interpolationmode=sample-hold);
        @statistic[driftRateChanged](title="Oscillator drift rate"; record=vector; interpolationmode=sample-hold; unit=ppm);
}

