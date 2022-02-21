//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Units;
import inet.physicallayer.wireless.common.base.packetlevel.NarrowbandReceptionBase;

cplusplus {{
#include "inet/physicallayer/wireless/common/base/packetlevel/FlatReceptionBase.h"
}}

namespace inet::physicallayer;

class FlatReceptionBase extends NarrowbandReceptionBase
{
    @existingClass;
    @descriptor(readonly);
}
