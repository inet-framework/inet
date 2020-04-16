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

import inet.common.INETDefs;
import inet.common.Units;
import inet.physicallayer.contract.packetlevel.IModulation;

namespace inet::physicallayer;

//
// Message kinds for sending commands to the radio.
//
enum RadioCommandCode
{
    RADIO_C_CONFIGURE = 1;
}

//
// Control info attached to a configure command that is sent to the ~Radio.
//
class ConfigureRadioCommand extends cObject
{
    int radioMode = -1;                  // new radio mode or -1 if not set.
    W power = W(NaN);                    // new default transmission power in the range (0, +infinity) or NaN if not set.
    bps bitrate = bps(NaN);              // new default bitrate in the range (0, +infinity) or NaN if not set.
    IModulation *modulation = nullptr;   // new default modulation or nullptr if not set.
    Hz centerFrequency = Hz(NaN);       // new default carrier frequency in the range (0, +infinity) or NaN if not set.
    Hz bandwidth = Hz(NaN);              // new default bandwidth in the rage (0, +infinity) or NaN if not set.
}
