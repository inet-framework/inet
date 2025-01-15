//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.oscillator;

//
// This module behaves like the RandomDriftOscillator but ensures that the drift does not exceed a certain limit
// from the simulation time.
//
simple OffsetLimitedRandomDriftOscillator extends RandomDriftOscillator
{
    parameters:
        @class(OffsetLimitedRandomDriftOscillator);
        double maxOffset @unit(s) = default(0s); // Max offset to simulation time until the drift is adjusted
        double maxDriftRateAdjustment @unit(ppm) = default(0ppm); // Max drift rate adjustment per changeInterval
}
