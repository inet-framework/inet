//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.oscillator;

import inet.clock.base.OscillatorBase;
import inet.clock.contract.IOscillator;

//
// This module generates ticks periodically with a constant tick length.
//
simple IdealOscillator extends OscillatorBase like IOscillator
{
    parameters:
        double tickLength @unit(s) = default(0s); // 0 means simulation time precision
        @class(IdealOscillator);
}

