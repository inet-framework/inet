//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.oscillator;

import inet.clock.base.DriftingOscillatorBase;
import inet.clock.contract.IOscillator;

//
// This module generates ticks periodically using a constant drift in the clock
// speed. The first tick is at the simulation time of the module initialization
// optionally shifted with an offset. The constant drift can be set from C++ or
// using a <set-oscillator module="..." drift-rate="..." tick-offset=".."/>
// command in a ScenarioManager script.
//
// @see ~ScenarioManager
//
simple ConstantDriftOscillator extends DriftingOscillatorBase like IOscillator
{
    parameters:
        double driftRate @unit(ppm) = default(0ppm); // expressed as a ratio in parts per million
        @class(ConstantDriftOscillator);
}

