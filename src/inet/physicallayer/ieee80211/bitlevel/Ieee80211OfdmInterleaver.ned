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

import inet.physicallayer.contract.bitlevel.IInterleaver;

// The ~Ieee80211OfdmInterleaver is defined by a two-step permutation.
// The first permutation ensures that adjacent coded bits are mapped
// onto nonadjacent subcarriers. The second ensures that adjacent coded
// bits are mapped alternately onto less and more significant bits of the
// constellation and, thereby, long runs of low reliability (LSB) bits are
// avoided. (IEEE 802.11, 18.3.5.7 Data interleaving)
//
// The module has two modulation dependent parameters: number of coded bits
// per subcarrier (or constellation size, N_{BPSC}) and number of coded bits
// per symbol (N_{CBPS}).
//
// The latter is simply N_{BPSC} * number of subcarriers in a symbol.
//
module Ieee80211OfdmInterleaver like IInterleaver
{
    parameters:
        int numberOfCodedBitsPerSymbol;
        int numberOfCodedBitsPerSubcarrier;
        @class("Ieee80211OfdmInterleaverModule");
        @display("i=block/switch");
}

