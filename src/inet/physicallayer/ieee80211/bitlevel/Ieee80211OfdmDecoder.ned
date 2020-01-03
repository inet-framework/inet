//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.physicallayer.ieee80211.bitlevel;

import inet.physicallayer.contract.bitlevel.IDecoder;
import inet.physicallayer.contract.bitlevel.IFecCoder;
import inet.physicallayer.contract.bitlevel.IInterleaver;
import inet.physicallayer.contract.bitlevel.IScrambler;

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

