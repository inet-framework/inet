//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/visualizer/util/Placement.h"

namespace inet {

namespace visualizer {

Placement parsePlacement(cObject *arr)
{
    int placement = PLACEMENT_NONE;
    auto tokens = check_and_cast<cValueArray*>(arr)->asStringVector();
    for (const auto& token : tokens) {
        if (token == "left")
            placement |= PLACEMENT_TOP_LEFT | PLACEMENT_CENTER_LEFT | PLACEMENT_BOTTOM_LEFT;
        else if (token == "right")
            placement |= PLACEMENT_TOP_RIGHT | PLACEMENT_CENTER_RIGHT | PLACEMENT_BOTTOM_RIGHT;
        else if (token == "top")
            placement |= PLACEMENT_TOP_LEFT | PLACEMENT_TOP_CENTER | PLACEMENT_TOP_RIGHT;
        else if (token == "bottom")
            placement |= PLACEMENT_BOTTOM_LEFT | PLACEMENT_BOTTOM_CENTER | PLACEMENT_BOTTOM_RIGHT;
        else if (token == "topLeft")
            placement |= PLACEMENT_TOP_LEFT;
        else if (token == "topCenter")
            placement |= PLACEMENT_TOP_CENTER;
        else if (token == "topRight")
            placement |= PLACEMENT_TOP_RIGHT;
        else if (token == "centerLeft")
            placement |= PLACEMENT_CENTER_LEFT;
        else if (token == "centerCenter")
            placement |= PLACEMENT_CENTER_CENTER;
        else if (token == "centerRight")
            placement |= PLACEMENT_CENTER_RIGHT;
        else if (token == "bottomLeft")
            placement |= PLACEMENT_BOTTOM_LEFT;
        else if (token == "bottomCenter")
            placement |= PLACEMENT_BOTTOM_CENTER;
        else if (token == "bottomRight")
            placement |= PLACEMENT_BOTTOM_RIGHT;
        else if (token == "any")
            placement |= PLACEMENT_ANY;
        else
            throw cRuntimeError("Unknown placement: %s", token.c_str());
    }
    return static_cast<Placement>(placement);
}

} // namespace visualizer

} // namespace inet

