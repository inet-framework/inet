//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.Units;

cplusplus {{
#include "inet/physicallayer/wireless/common/radio/packetlevel/BandListening.h"
}}

namespace inet::physicallayer;

class BandListening
{
    @existingClass;
    @descriptor(readonly);
    Hz centerFrequency;
    Hz bandwidth;
}
