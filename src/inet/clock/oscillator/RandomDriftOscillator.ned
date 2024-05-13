//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.oscillator;

import inet.clock.base.DriftingOscillatorBase;
import inet.clock.contract.IOscillator;

//
// This oscillator changes the drift rate over time. The drift rate is the sum of
// a distribution that is evaluated periodically and a random walk process.
//
simple RandomDriftOscillator extends DriftingOscillatorBase like IOscillator
{
    parameters:
        double initialDriftRate @unit(ppm) = default(0ppm); // Expressed as a ratio in parts per million
        volatile double changeInterval @unit(s); // Drift change happens at the end of every interval
        volatile double driftRateChange @unit(ppm) = default(0ppm); // Integrated over time (random walk), no change by default
        double driftRateChangeLowerLimit @unit(ppm) = default(-inf ppm); // Lower limit for random walk, no limit by default
        double driftRateChangeUpperLimit @unit(ppm) = default(inf ppm); // Upper limit for random walk, no limit by default
        @class(RandomDriftOscillator);
}

