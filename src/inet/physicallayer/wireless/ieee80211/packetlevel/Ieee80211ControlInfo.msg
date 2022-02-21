//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Units;
import inet.physicallayer.wireless.common.contract.packetlevel.RadioControlInfo;
import inet.physicallayer.wireless.ieee80211.mode.IIeee80211Mode;
import inet.physicallayer.wireless.ieee80211.mode.Ieee80211Channel;

namespace inet::physicallayer;

//
// Control info attached to a configure command that is sent to the ~Radio.
//
class Ieee80211ConfigureRadioCommand extends ConfigureRadioCommand
{
    string opMode;                         // new default operation mode or "" if not set.
    const Ieee80211ModeSet *modeSet;       // new default mode set or nullptr if not set.
    const IIeee80211Mode *mode;            // new default transmission mode or nullptr if not set.
    IIeee80211Band *band;                  // new default band or nullptr if not set.
    Ieee80211Channel *channel;             // new default band and channel or nullptr if not set.
    int channelNumber = -1;                // new default channel number in the range [0, numChannels] or -1 if not set.
}
