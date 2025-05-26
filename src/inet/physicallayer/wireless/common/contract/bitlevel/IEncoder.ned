//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.contract.bitlevel;

//
// Module interface for bit-level encoders used in wireless communication
// systems. Encoders transform packet models into bit models by applying
// techniques such as scrambling, forward error correction (FEC), and
// interleaving. These processes add redundancy and structure to the data stream
// to improve transmission reliability over noisy channels.
//
// @see IDecoder
//
moduleinterface IEncoder
{
    parameters:
        @display("i=block/blackboard");
}

