//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.common.base.bitlevel;

import inet.physicallayer.wireless.common.contract.bitlevel.ILayeredErrorModel;

//
// Base module for layered error models in wireless communications. Provides
// functionality for computing errors at different protocol layers (packet, bit,
// symbol, sample) based on signal-to-noise-and-interference ratio (SNIR).
// Supports configurable symbol corruption modes to control how symbol errors are
// modeled. Serves as a foundation for more specific error model implementations
// like StochasticLayeredErrorModel that add specific error generation mechanisms.
//
module LayeredErrorModelBase like ILayeredErrorModel
{
    parameters:
        string symbolCorruptionMode @enum("1bit", "uniform") = default("1bit");
        @display("i=block/broadcast");
}

