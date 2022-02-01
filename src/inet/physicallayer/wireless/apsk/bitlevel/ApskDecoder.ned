//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.apsk.bitlevel;

import inet.physicallayer.wireless.common.contract.bitlevel.IDecoder;
import inet.physicallayer.wireless.common.contract.bitlevel.IFecCoder;
import inet.physicallayer.wireless.common.contract.bitlevel.IInterleaver;
import inet.physicallayer.wireless.common.contract.bitlevel.IScrambler;

//
// This module is part of a simple hypothetical layered receiver. It computes
// the packet domain representation from the bit domain representation by applying
// the configured descrambling, forward errror correction decoding, and deinterleaving.
//
// @see ~ApskEncoder, ~ApskLayeredTransmitter.
//
module ApskDecoder like IDecoder
{
    parameters:
        @display("i=block/blackboard");
        @class(ApskDecoder);

    submodules:
        deinterleaver: <default("")> like IInterleaver if typename != "" {
            @display("p=100,100");
        }
        fecDecoder: <default("")> like IFecCoder if typename != "" {
            @display("p=100,200");
        }
        descrambler: <default("")> like IScrambler if typename != "" {
            @display("p=100,300");
        }
}

