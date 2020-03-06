//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee80211.bitlevel;

import inet.physicallayer.wireless.common.contract.bitlevel.IModulator;

// This is an IEEE 802.11 OFDM modulator module, the
// implementation is based on 18.3.5.8 Subcarrier modulation
// mapping section in IEEE 802.11-2012 Std.
//
// It has only one parameter: the subcarrier modulation,
// in compliant mode, is one of the following constellation
// mappings (Table 18-7.):
// - BPSK
// - QPSK
// - 16-QAM
// - 64-QAM
//
// However, it can be used with an arbitrary OFDM compatible
// constellation mapping.
//
simple Ieee80211OfdmModulator like IModulator
{
    parameters:
        int numSubcarriers = default(52);
        string subcarrierModulation;
        int pilotSubcarrierPolarityVectorOffset;
        @display("i=block/mac");
        @class(Ieee80211OfdmModulatorModule);
}

