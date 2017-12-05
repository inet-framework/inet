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
import inet.physicallayer.contract.packetlevel.RadioControlInfo;
import inet.physicallayer.ieee80211.mode.IIeee80211Mode;
import inet.physicallayer.ieee80211.mode.Ieee80211Channel;

namespace inet::physicallayer;

//
// Control info attached to a configure command that is sent to the ~Radio.
//
class Ieee80211ConfigureRadioCommand extends ConfigureRadioCommand
{
    string opMode;                         // new default operation mode or "" if not set.
    const Ieee80211ModeSet *modeSet = nullptr; // new default mode set or nullptr if not set.
    const IIeee80211Mode *mode = nullptr; // new default transmission mode or nullptr if not set.
    IIeee80211Band *band = nullptr; // new default band or nullptr if not set.
    Ieee80211Channel *channel = nullptr; // new default band and channel or nullptr if not set.
    int channelNumber = -1;                // new default channel number in the range [0, numChannels] or -1 if not set.
}

