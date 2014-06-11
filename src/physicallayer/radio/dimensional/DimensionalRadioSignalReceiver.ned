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

package inet.physicallayer.radio.dimensional;

import inet.physicallayer.radio.contract.IRadioSignalReceiver;

module DimensionalRadioSignalReceiver like IRadioSignalReceiver
{
    parameters:
        double energyDetection @unit(dBm); // TODO: @unit(W) + dBm/dBW <--> W
        double sensitivity @unit(dBm); // TODO: @unit(W) + dBm/dBW <--> W
        double snirThreshold @unit(dB);
        double carrierFrequency @unit(Hz);
        double bandwidth @unit(Hz);
        string modulation = default("NULL"); // "BPSK", "16-QAM", "256-QAM" or "NULL"
        @class(DimensionalRadioSignalReceiver);
}
