//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;

namespace inet::ieee80211;

enum FrameTransmissionStatus
{
    FRAME_TRANSMISSION_STATUS_ACKNOWLEDGED = 0;
    FRAME_TRANSMISSION_STATUS_RETRY_LIMIT_REACHED = 1;
    FRAME_TRANSMISSION_STATUS_DROPPED_BEFORE_TRANSMISSION = 2;
}

//
// Details payload emitted with frameTransmissionFinishedSignal.
// The payload is created synchronously by the MAC and is valid only for
// the duration of the notification.
//
class FrameTransmissionDetails extends cObject
{
    FrameTransmissionStatus status;
}
