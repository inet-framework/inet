//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.contract.bitlevel;

//
// Interface for digital modulator modules that convert sequences of bits into
// sequences of symbols. Modulators implement various modulation schemes (such as
// BPSK, QPSK, QAM, etc.) that determine how digital information is encoded into
// changes in the carrier signal's amplitude, frequency, or phase. The modulator
// takes a bit model as input and produces a symbol model as output, forming an
// essential component in the physical layer signal processing chain for wireless
// communication systems.
//
// @see IDemodulator
//
moduleinterface IModulator
{
    parameters:
        @display("i=block/mac");
}

