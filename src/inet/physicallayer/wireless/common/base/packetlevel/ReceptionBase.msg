//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.geometry.Geometry;
import inet.physicallayer.wireless.common.signal.WirelessSignal;

cplusplus {{
#include "inet/physicallayer/wireless/common/base/packetlevel/ReceptionBase.h"
}}

namespace inet::physicallayer;

class ReceptionBase extends cObject
{
    @existingClass;
    @descriptor(readonly);
    simtime_t startTime;
    Coord startPosition;
    Quaternion startOrientation;
    simtime_t endTime;
    Coord endPosition;
    Quaternion endOrientation;
    simtime_t duration;
    simtime_t preambleDuration;
    simtime_t headerDuration;
    simtime_t dataDuration;
    const ITransmission *transmission;
}
