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

package inet.physicallayer.dimensional;

import inet.physicallayer.contract.ITransmitter;

module DimensionalTransmitter like ITransmitter
{
    parameters:
        double bitrate @unit(bps);
        double headerBitLength @unit(b);
        double power @unit(W);
        double carrierFrequency @unit(Hz);
        double bandwidth @unit(Hz);
        string modulation = default("NULL"); // "BPSK", "16-QAM", "256-QAM" or "NULL"
        string dimensions = default("time");
        string timeGains = default("0% 0 10ps 1 -10ps 1 100% 0"); // sequence of time and gain pairs, time in [%] or [s], negative time measures from the end, gain in [dB] or [0..1]
        string frequencyGains = default("0% 0 10% -3dB 45% 0dB 90% -3dB 100% 0"); // sequence of frequency and gain pairs, frequency in [%] or [Hz], negative frequency measures from the end, gain in [dB] or [0..1] 
        string interpolationMode @enum("linear", "sample-hold") = default("sample-hold");
        @class(DimensionalTransmitter);
}
