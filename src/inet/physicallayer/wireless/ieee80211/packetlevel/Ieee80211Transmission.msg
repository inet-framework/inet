//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.physicallayer.wireless.common.analogmodel.packetlevel.ScalarTransmission;
import inet.physicallayer.wireless.ieee80211.mode.IIeee80211Mode;
import inet.physicallayer.wireless.ieee80211.mode.Ieee80211Channel;

cplusplus {{
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211ScalarTransmission.h"
}}

namespace inet::physicallayer;

class Ieee80211ScalarTransmission extends ScalarTransmission
{
    @existingClass;
    @descriptor(readonly);
    const IIeee80211Mode *mode;
    const Ieee80211Channel *channel;
}
