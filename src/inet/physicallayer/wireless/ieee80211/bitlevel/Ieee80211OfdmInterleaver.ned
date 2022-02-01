//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee80211.bitlevel;

import inet.physicallayer.wireless.common.contract.bitlevel.IInterleaver;

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

