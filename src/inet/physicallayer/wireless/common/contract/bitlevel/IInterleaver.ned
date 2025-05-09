//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.contract.bitlevel;

//
// Module interface for bit-level interleavers used in wireless communication
// systems. Interleavers rearrange the order of bits in a data stream to spread
// the effects of burst errors, improving the performance of forward error
// correcting codes. Implementations provide methods for interleaving and
// deinterleaving bit sequences according to specific patterns.
//
moduleinterface IInterleaver
{
    parameters:
        @display("i=block/switch");
}

