//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.apsk.bitlevel;

import inet.physicallayer.wireless.common.contract.bitlevel.IEncoder;
import inet.physicallayer.wireless.common.contract.bitlevel.IFecCoder;
import inet.physicallayer.wireless.common.contract.bitlevel.IInterleaver;
import inet.physicallayer.wireless.common.contract.bitlevel.IScrambler;

//
// This module is part of a simple hypothetical layered transmitter. It computes
// the bit domain representation from the packet domain representation by applying
// the configured scrambling, forward errror correction encoding, and interleaving.
//
// @see ~ApskDecoder, ~ApskLayeredReceiver.
//
module ApskEncoder like IEncoder
{
    parameters:
        @display("i=block/blackboard");
        @class(ApskEncoder);

    submodules:
        scrambler: <default("")> like IScrambler if typename != "" {
            @display("p=100,100");
        }
        fecEncoder: <default("")> like IFecCoder if typename != "" {
            @display("p=100,200");
        }
        interleaver: <default("")> like IInterleaver if typename != "" {
            @display("p=100,300");
        }
}

