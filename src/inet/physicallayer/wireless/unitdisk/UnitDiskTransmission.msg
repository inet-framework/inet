//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Units;
import inet.physicallayer.wireless.common.base.packetlevel.TransmissionBase;

cplusplus {{
#include "inet/physicallayer/wireless/unitdisk/UnitDiskTransmission.h"
}}

namespace inet::physicallayer;

class UnitDiskTransmission extends TransmissionBase
{
    @existingClass;
    @descriptor(readonly);
    m communicationRange;
    m interferenceRange;
    m detectionRange;
}
