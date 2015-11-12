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

package inet.physicallayer.idealradio;

import inet.physicallayer.contract.packetlevel.ITransmitter;

//
// This transmitter model produces transmissions that are parameterized with
// communication range, interference range, and detection range. It also
// supports an ideal communication channel with configuring the range parameters
// to infinity.
//
// See also ~IdealReceiver, ~IdealRadio, ~IdealRadioMedium, and
// ~IdealAnalogModel.
//
module IdealTransmitter like ITransmitter
{
    parameters:
        double preambleDuration @unit(s) = default(0s);                        // preamble added before the header
        double headerBitLength @unit(b) = default(0b);                         // number of header bits added to the length of the MAC frame
        double bitrate @unit(bps);                                             // the bitrate of the transmitter is used to compute the duration of the transmission
        double maxCommunicationRange @unit(m);                                 // the maximum range where the reception of transmissions produced by this transmitter is possible
        double maxInterferenceRange @unit(m) = default(maxCommunicationRange); // the maximum range where transmissions produced by this transmitter interferes with other transmissions
        double maxDetectionRange @unit(m) = default(maxInterferenceRange);     // the maximum range where transmissions produced by this transmitter can be detected at all
        @class(IdealTransmitter);
        @display("i=block/wtx");
}
