//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Units;
import inet.physicallayer.wireless.common.base.packetlevel.FlatReceptionBase;

cplusplus {{
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarReception.h"
}}

namespace inet::physicallayer;

class ScalarReception extends FlatReceptionBase
{
    @existingClass;
    @descriptor(readonly);
    W power;
}
