//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Units;
import inet.physicallayer.wireless.common.base.packetlevel.ReceptionBase;

cplusplus {{
#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandReceptionBase.h"
}}

namespace inet::physicallayer;

class NarrowbandReceptionBase extends ReceptionBase
{
    @existingClass;
    @descriptor(readonly);
    Hz centerFrequency;
    Hz bandwidth;
}
