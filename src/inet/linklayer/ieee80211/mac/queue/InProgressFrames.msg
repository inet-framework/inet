//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.Packet;

cplusplus {{
#include "inet/linklayer/ieee80211/mac/queue/InProgressFrames.h"
}}

namespace inet::ieee80211;

class InProgressFrames extends cOwnedObject
{
    @existingClass;
    @descriptor(readonly);
    int length;
    Packet *frames[] @sizeGetter(getLength);
}
