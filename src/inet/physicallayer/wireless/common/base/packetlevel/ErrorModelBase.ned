//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.base.packetlevel;

import inet.physicallayer.wireless.common.contract.packetlevel.IErrorModel;

//
// This module servces as the base module for error models.
//
module ErrorModelBase like IErrorModel
{
    parameters:
        string corruptionMode @enum("packet", "chunk", "byte", "bit") = default("packet");
        string snirMode @enum("min", "mean") = default("min");
        double snirOffset @unit(dB) = default(0dB); // offset the calculated SNIR before further computing the PER, BER, SER or corrupt bits and symbols
                                                    // this can be used, for example, to represents cross symbol interference due to OFDM lobe overlapping, symbol frequency/time offset mismatch, and symbol frequency/time size mismatch
        @display("i=block/broadcast");
        @class(ErrorModelBase);
}

