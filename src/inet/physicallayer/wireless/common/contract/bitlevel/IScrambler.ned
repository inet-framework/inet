//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.contract.bitlevel;

//
// Module interface for scramblers that convert input bit sequences into
// seemingly random output sequences of the same length. Scramblers are used in
// wireless communication systems to randomize data, avoid long sequences of
// identical bits, improve clock recovery, and reduce transmission errors.
// Implementations typically use generator polynomials and seed values to define
// the scrambling algorithm. This interface supports both scrambling and
// descrambling operations needed in encoders and decoders of various wireless
// protocols.
//
moduleinterface IScrambler
{
    parameters:
        @display("i=block/tunnel");
}

