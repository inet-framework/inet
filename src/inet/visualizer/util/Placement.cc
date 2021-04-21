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

#include "inet/visualizer/util/Placement.h"

namespace inet {

namespace visualizer {

Placement parsePlacement(const char *s)
{
    int placement = PLACEMENT_NONE;
    cStringTokenizer tokenizer(s);
    while (tokenizer.hasMoreTokens()) {
        auto token = tokenizer.nextToken();
        if (!strcmp("left", token))
            placement |= PLACEMENT_TOP_LEFT | PLACEMENT_CENTER_LEFT | PLACEMENT_BOTTOM_LEFT;
        else if (!strcmp("right", token))
            placement |= PLACEMENT_TOP_RIGHT | PLACEMENT_CENTER_RIGHT | PLACEMENT_BOTTOM_RIGHT;
        else if (!strcmp("top", token))
            placement |= PLACEMENT_TOP_LEFT | PLACEMENT_TOP_CENTER | PLACEMENT_TOP_RIGHT;
        else if (!strcmp("bottom", token))
            placement |= PLACEMENT_BOTTOM_LEFT | PLACEMENT_BOTTOM_CENTER | PLACEMENT_BOTTOM_RIGHT;
        else if (!strcmp("topLeft", token))
            placement |= PLACEMENT_TOP_LEFT;
        else if (!strcmp("topCenter", token))
            placement |= PLACEMENT_TOP_CENTER;
        else if (!strcmp("topRight", token))
            placement |= PLACEMENT_TOP_RIGHT;
        else if (!strcmp("centerLeft", token))
            placement |= PLACEMENT_CENTER_LEFT;
        else if (!strcmp("centerCenter", token))
            placement |= PLACEMENT_CENTER_CENTER;
        else if (!strcmp("centerRight", token))
            placement |= PLACEMENT_CENTER_RIGHT;
        else if (!strcmp("bottomLeft", token))
            placement |= PLACEMENT_BOTTOM_LEFT;
        else if (!strcmp("bottomCenter", token))
            placement |= PLACEMENT_BOTTOM_CENTER;
        else if (!strcmp("bottomRight", token))
            placement |= PLACEMENT_BOTTOM_RIGHT;
        else if (!strcmp("any", token))
            placement |= PLACEMENT_ANY;
        else
            throw cRuntimeError("Unknown placement: %s", token);
    }
    return static_cast<Placement>(placement);
}

} // namespace visualizer

} // namespace inet

