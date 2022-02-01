//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.apsk.bitlevel;

import inet.physicallayer.wireless.common.contract.bitlevel.IDemodulator;

//
// This module is part of a simple hypothetical layered receiver. It computes
// the bit domain representation from the symbol domain representation by applying
// the configured modulation.
//
// @see ~ApskModulator, ~ApskLayeredReceiver.
//
simple ApskDemodulator like IDemodulator
{
    parameters:
        string modulation; // BPSK, QPSK, QAM-16, QAM-64, QAM-256, etc. (see ApskModulationBase::findModulation)
        @display("i=block/mac");
}

