//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PLACEMENT_H
#define __INET_PLACEMENT_H

#include "inet/common/INETDefs.h"

namespace inet {

namespace visualizer {

enum Placement {
    PLACEMENT_NONE          = 0x0000,
    PLACEMENT_TOP_LEFT      = 0x0001,
    PLACEMENT_TOP_CENTER    = 0x0002,
    PLACEMENT_TOP_RIGHT     = 0x0004,
    PLACEMENT_CENTER_LEFT   = 0x0008,
    PLACEMENT_CENTER_CENTER = 0x0010,
    PLACEMENT_CENTER_RIGHT  = 0x0020,
    PLACEMENT_BOTTOM_LEFT   = 0x0040,
    PLACEMENT_BOTTOM_CENTER = 0x0080,
    PLACEMENT_BOTTOM_RIGHT  = 0x0100,
    PLACEMENT_ANY           = 0xFFFF
};

Placement parsePlacement(const char *s);

} // namespace visualizer

} // namespace inet

#endif

