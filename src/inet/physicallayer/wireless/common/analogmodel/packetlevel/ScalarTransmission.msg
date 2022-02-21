//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Units;
import inet.physicallayer.wireless.common.base.packetlevel.FlatTransmissionBase;

cplusplus {{
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarTransmission.h"
}}

namespace inet::physicallayer;

class ScalarTransmission extends FlatTransmissionBase
{
    @existingClass;
    @descriptor(readonly);
    W power;
}
