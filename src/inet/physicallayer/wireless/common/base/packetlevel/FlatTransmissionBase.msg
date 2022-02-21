//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Units;
import inet.physicallayer.wireless.common.base.packetlevel.NarrowbandTransmissionBase;

cplusplus {{
#include "inet/physicallayer/wireless/common/base/packetlevel/FlatTransmissionBase.h"
}}

namespace inet::physicallayer;

class FlatTransmissionBase extends NarrowbandTransmissionBase
{
    @existingClass;
    @descriptor(readonly);
    b headerLength;
    b dataLength;
    bps bitrate;
}
