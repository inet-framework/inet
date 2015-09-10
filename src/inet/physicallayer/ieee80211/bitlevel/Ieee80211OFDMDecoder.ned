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
import inet.physicallayer.contract.bitlevel.IFECCoder;
import inet.physicallayer.contract.bitlevel.IInterleaver;
import inet.physicallayer.contract.bitlevel.IScrambler;

// This is the decoder module for the layered IEEE 802.11
// OFDM PHY infrastructure (IEEE 802.11-2012, Clause 18).
//
// This module integrates the bit domain specific components
// of the OFDM physical layer and has three main submodules:
//
//       - Convolutional decoder (18.3.5.6 Convolutional encoder) @see ~ConvolutionalCoder
//       - Additive descrambler (18.3.5.5 PLCP DATA scrambler and descrambler) @see ~AdditiveScrambler
//       - Block deinterleaver (18.3.5.7 Data interleaving) @see ~Ieee80211OFDMInterleaver
//
//
module Ieee80211OFDMDecoder like IDecoder
{
    parameters:
        string fecType = default("");
        string deinterleaverType = default("");
        string descramblerType = default("");
        @display("i=block/blackboard");
        @class(Ieee80211OFDMDecoderModule);

    submodules:
        descrambler: <descramblerType> like IScrambler if descramblerType != "" {
            @display("p=72,196");
        }
        fecDecoder: <fecType> like IFECCoder if fecType != "" {
            @display("p=72,106");
        }
        deinterleaver: <deinterleaverType> like IInterleaver if deinterleaverType != "" {
            @display("p=72,29");
        }
}
