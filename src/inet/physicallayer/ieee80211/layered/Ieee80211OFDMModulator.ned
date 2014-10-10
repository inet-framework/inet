//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.physicallayer.ieee80211.layered;

import inet.physicallayer.contract.IModulator;


simple Ieee80211OFDMModulator like IModulator
{
    parameters:
        string modulationScheme; // QAM-16, QAM-64, BPSK, QPSK
        @class(Ieee80211OFDMModulator);
}
