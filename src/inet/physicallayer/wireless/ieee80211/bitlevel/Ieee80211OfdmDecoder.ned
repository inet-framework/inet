//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee80211.bitlevel;

import inet.physicallayer.wireless.common.contract.bitlevel.IDecoder;
import inet.physicallayer.wireless.common.contract.bitlevel.IFecCoder;
import inet.physicallayer.wireless.common.contract.bitlevel.IInterleaver;
import inet.physicallayer.wireless.common.contract.bitlevel.IScrambler;

// This is the decoder module for the layered IEEE 802.11
// OFDM PHY infrastructure (IEEE 802.11-2012, Clause 18).
//
// This module integrates the bit domain specific components
// of the OFDM physical layer and has three main submodules:
//
// - Convolutional decoder (18.3.5.6 Convolutional encoder) (see: ~ConvolutionalCoder)
// - Additive descrambler (18.3.5.5 PLCP DATA scrambler and descrambler) (see: ~AdditiveScrambler)
// - Block deinterleaver (18.3.5.7 Data interleaving) (see: ~Ieee80211OfdmInterleaver)
//
module Ieee80211OfdmDecoder like IDecoder
{
    parameters:
        @display("i=block/blackboard");
        @class(Ieee80211OfdmDecoderModule);

    submodules:
        descrambler: <default("")> like IScrambler if typename != "" {
            @display("p=100,300");
        }
        fecDecoder: <default("")> like IFecCoder if typename != "" {
            @display("p=100,200");
        }
        deinterleaver: <default("")> like IInterleaver if typename != "" {
            @display("p=100,100");
        }
}

