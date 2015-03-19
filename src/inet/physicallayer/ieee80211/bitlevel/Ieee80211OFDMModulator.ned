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

import inet.physicallayer.contract.bitlevel.IModulator;

// This is an IEEE 802.11 OFDM modulator module, the
// implementation is based on 18.3.5.8 Subcarrier modulation
// mapping section in IEEE 802.11-2012 Std.
//
// It has only one parameter: the subcarrier modulation,
// in compliant mode, is one of the following constellation
// mappings (Table 18-7.):
//         - BPSK
//         - QPSK
//         - 16-QAM
//         - 64-QAM
//
// However, it can be used with an arbitrary OFDM compatible
// constellation mapping.
//
//
simple Ieee80211OFDMModulator like IModulator
{
    parameters:
        string subcarrierModulation;
        int pilotSubcarrierPolarityVectorOffset;
        @display("i=block/mac");
        @class(Ieee80211OFDMModulatorModule);
}
