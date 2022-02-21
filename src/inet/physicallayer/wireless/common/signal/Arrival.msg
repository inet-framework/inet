//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.geometry.Geometry;

cplusplus {{
#include "inet/physicallayer/wireless/common/signal/Arrival.h"
}}

namespace inet::physicallayer;

class Arrival
{
    @existingClass;
    @descriptor(readonly);
    simtime_t startTime;
    Coord startPosition;
    Quaternion startOrientation;
    simtime_t endTime;
    Coord endPosition;
    Quaternion endOrientation;
}
