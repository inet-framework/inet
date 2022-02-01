//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

