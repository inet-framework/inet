//
// Copyright (C) 2006 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//

cplusplus {{
#include "inet/physicallayer/base/PhysicalLayerDefs.h"
}}

namespace inet;

class noncobject bps;
class noncobject W;

//
// Message kinds for sending commands to the radio.
//
enum RadioCommandCode
{
    RADIO_C_CONFIGURE = 1;
}

//
// Control info attached to a command that is sent to the ~Radio.
//
class RadioConfigureCommand
{
    int channelNumber = -1;  // new channel number in the range [0, numChannels] or -1 if not set.
    bps bitrate = bps(NaN); // new default bitrate in the range (0, +infinity) or NaN if not set.
    W power = W(NaN);       // new default transmission power in the range (0, +infinity) or NaN if not set.
}

//
// Control info attached to a mac frame that is sent down to the ~Radio.
//
class RadioTransmissionRequest
{
    bps bitrate = bps(NaN); // override default bitrate in the range (0, +infinity) or NaN if not set.
    W power = W(NaN);       // override default transmission power in the range (0, +infinity) or NaN if not set.
}

//
// Control info attached to a mac frame that is sent up from the ~Radio.
//
class RadioReceptionIndication
{
    int bitErrorCount = -1;       // number of erroneous bits in the range [0, +infinity) or -1 if unknown.
    int symbolErrorCount = -1;    // number of erroneous symbols in the range [0, +infinity) or -1 if unknown.
    double packetErrorRate = NaN; // packet error rate (probability) in the range [0, 1] or NaN if unknown.
    double bitErrorRate = NaN;    // bit error rate (probability) in the range [0, 1] or NaN if unknown.
    double symbolErrorRate = NaN; // symbol error rate (probability) in the range [0, 1] or NaN if unknown.
    W minRSSI = W(NaN);          // minimum receive signal strength indication in the range (0, +infinity) or NaN if unknown.
    double minSNIR = NaN;        // minimum signal to noise plus interference ratio in the range (0, +infinity) or NaN if unknown.
}
