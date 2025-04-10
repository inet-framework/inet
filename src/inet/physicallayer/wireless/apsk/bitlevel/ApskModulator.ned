//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.apsk.bitlevel;

import inet.common.SimpleModule;
import inet.physicallayer.wireless.common.contract.bitlevel.IModulator;

//
// Part of a simple hypothetical layered transmitter. It computes
// the symbol domain representation from the bit domain representation by applying
// the configured modulation.
//
// @see ~ApskDemodulator, ~ApskLayeredTransmitter.
//
simple ApskModulator extends SimpleModule like IModulator
{
    parameters:
        @class(ApskModulator);
        string modulation; // BPSK, QPSK, QAM-16, QAM-64, QAM-256, etc. (see ApskModulationBase::findModulation)
        @display("i=block/mac");
}

