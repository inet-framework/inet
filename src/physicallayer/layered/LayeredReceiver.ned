//
// Copyright (C) 2013 OpenSim Ltd
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

package inet.physicallayer.layered;

import inet.physicallayer.contract.IAnalogDigitalConverter;
import inet.physicallayer.contract.IDecoder;
import inet.physicallayer.contract.IDemodulator;
import inet.physicallayer.contract.IPulseFilter;
import inet.physicallayer.contract.IReceiver;

module LayeredReceiver like IReceiver
{
    parameters:
        string decoderType;
        string demodulatorType;
        string shaperFilterType;
        string analogDigitalConverterType;
        @class(LayeredReceiver);

    submodules:
        decoder: <decoderType> like IDecoder {
            @display("p=100,50");
        }
        demodulator: <demodulatorType> like IDemodulator {
            @display("p=100,150");
        }
        shaperFilter: <shaperFilterType> like IPulseFilter {
            @display("p=100,250");
        }
        analogDigitalConverter: <analogDigitalConverterType> like IAnalogDigitalConverter {
            @display("p=100,350");
        }
}
