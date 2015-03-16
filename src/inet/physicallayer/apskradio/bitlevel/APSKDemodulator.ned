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

package inet.physicallayer.apskradio.bitlevel;

import inet.physicallayer.contract.bitlevel.IDemodulator;

//
// This module is part of a simple hypothetical layered receiver. It computes
// the bit domain representation from the symbol domain representation by applying
// the configured modulation.
//
// See also ~APSKModulator, ~APSKLayeredReceiver.
//
simple APSKDemodulator like IDemodulator
{
    parameters:
        string modulation; // BPSK, QPSK, QAM-16, QAM-64, QAM-256, etc. (see APSKModulationBase::findModulation)
        @display("i=block/mac");
}
