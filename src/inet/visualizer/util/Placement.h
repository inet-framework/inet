//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_PLACEMENT_H
#define __INET_PLACEMENT_H

#include "inet/common/INETDefs.h"

namespace inet {

namespace visualizer {

enum Placement
{
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

#endif // ifndef __INET_PLACEMENT_H

