//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Units;

cplusplus {{
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Band.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Channel.h"
}}

namespace inet::physicallayer;

class IIeee80211Band
{
    @existingClass;
    @descriptor(readonly);
    string name;
    Hz centerFrequency[] @sizeGetter(getNumChannels);
}

class Ieee80211EnumeratedBand extends IIeee80211Band
{
    @existingClass;
    @descriptor(readonly);
}

class Ieee80211ArithmeticalBand extends IIeee80211Band
{
    @existingClass;
    @descriptor(readonly);
}

class Ieee80211Channel
{
    @existingClass;
    @descriptor(readonly);
    IIeee80211Band *band;
    int channelNumber;
}
