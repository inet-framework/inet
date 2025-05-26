//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.contract.bitlevel;

//
// Module interface for demodulators in wireless communication systems.
// Demodulators convert symbols back to bits in the receiver's signal processing
// chain. They take a reception symbol model as input and produce a reception bit
// model as output by applying the inverse of the modulation scheme (such as
// BPSK, QPSK, QAM16, etc.) to each symbol to recover the original bit sequence.
//
// @see IModulator
//
moduleinterface IDemodulator
{
    parameters:
        @display("i=block/mac");
}

