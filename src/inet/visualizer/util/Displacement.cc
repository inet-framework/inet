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

#include "inet/visualizer/util/Displacement.h"

namespace inet {

namespace visualizer {

Displacement parseDisplacement(const char *s)
{
    int displacement = DISPLACEMENT_NONE;
    cStringTokenizer tokenizer(s);
    while (tokenizer.hasMoreTokens()) {
        auto token = tokenizer.nextToken();
        if (!strcmp("left", token))
            displacement |= DISPLACEMENT_TOP_LEFT | DISPLACEMENT_CENTER_LEFT | DISPLACEMENT_BOTTOM_LEFT;
        else if (!strcmp("right", token))
            displacement |= DISPLACEMENT_TOP_RIGHT | DISPLACEMENT_CENTER_RIGHT | DISPLACEMENT_BOTTOM_RIGHT;
        else if (!strcmp("top", token))
            displacement |= DISPLACEMENT_TOP_LEFT | DISPLACEMENT_TOP_CENTER | DISPLACEMENT_TOP_RIGHT;
        else if (!strcmp("bottom", token))
            displacement |= DISPLACEMENT_BOTTOM_LEFT | DISPLACEMENT_BOTTOM_CENTER | DISPLACEMENT_BOTTOM_RIGHT;
        else if (!strcmp("topLeft", token))
            displacement |= DISPLACEMENT_TOP_LEFT;
        else if (!strcmp("topCenter", token))
            displacement |= DISPLACEMENT_TOP_CENTER;
        else if (!strcmp("topRight", token))
            displacement |= DISPLACEMENT_TOP_RIGHT;
        else if (!strcmp("centerLeft", token))
            displacement |= DISPLACEMENT_CENTER_LEFT;
        else if (!strcmp("centerRight", token))
            displacement |= DISPLACEMENT_CENTER_RIGHT;
        else if (!strcmp("bottomLeft", token))
            displacement |= DISPLACEMENT_BOTTOM_LEFT;
        else if (!strcmp("bottomCenter", token))
            displacement |= DISPLACEMENT_BOTTOM_CENTER;
        else if (!strcmp("bottomRight", token))
            displacement |= DISPLACEMENT_BOTTOM_RIGHT;
        else if (!strcmp("any", token))
            displacement |= DISPLACEMENT_ANY;
        else
            throw cRuntimeError("Unknown displacement: %s", displacement);
    }
    return (Displacement)displacement;
}

} // namespace visualizer

} // namespace inet

