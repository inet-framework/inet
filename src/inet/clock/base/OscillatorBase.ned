//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.base;

import inet.common.SimpleModule;

//
// Base module for oscillators.
//
simple OscillatorBase extends SimpleModule
{
    parameters:
        @class(OscillatorBase);
        bool emitNumTicksSignal = default(false); // Enables emitting a signal at each oscillator tick, can be useful for validating the behavior of the more efficient implementation without events
        int oscillatorTickEventSchedulingPriority = default(0); // Specifies the scheduling priority of the tick event
        displayStringTextFormat = default("%n"); // Determines the text that is written on top of the submodule
        @display("i=block/tunnel");
        @signal[numTicksChanged](type=unsigned long);
        @signal[driftRateChanged](type=double);
        @statistic[numTicksChanged](title="number of ticks"; record=vector; interpolationmode=sample-hold);
        @statistic[driftRateChanged](title="Oscillator drift rate"; record=vector; interpolationmode=sample-hold; unit=ppm);
}

