//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.contract.bitlevel;

//
// Interface for decoders in wireless communications. Decoders convert bit-level
// signal representations back into packet-level representations, performing the
// reverse of the encoding process. This typically involves operations such as
// descrambling, forward error correction (FEC) decoding, and deinterleaving.
// Implementations of this interface handle different wireless standards and
// encoding schemes, providing the final step in recovering transmitted data
// from received bits.
//
// @see IEncoder
//
moduleinterface IDecoder
{
    parameters:
        @display("i=block/blackboard");
}

