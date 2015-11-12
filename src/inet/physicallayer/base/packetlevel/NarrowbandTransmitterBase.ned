//
// Copyright (C) 2013 OpenSim Ltd.
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

package inet.physicallayer.base.packetlevel;

import inet.physicallayer.contract.packetlevel.ITransmitter;

//
// This module servces as a base module for narrowband transmitter models.
//
module NarrowbandTransmitterBase like ITransmitter
{
    parameters:
        double preambleDuration @unit(s);  // preamble duration
        double headerBitLength @unit(b);   // number of header bits added to the length of the MAC frame
        double bitrate @unit(bps);         // the bitrate of the transmitter is used to compute the duration of the transmission
        double power @unit(W);             // transmission power
        double carrierFrequency @unit(Hz); // center frequency of the band where the transmitter transmits on the medium
        double bandwidth @unit(Hz);        // bandwidth of the band where the transmitter transmits on the medium
        string modulation;                 // "BPSK", "16-QAM", "256-QAM"
        @display("i=block/wtx");
}
