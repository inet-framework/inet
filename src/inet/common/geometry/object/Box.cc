//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/geometry/object/Box.h"

namespace inet {

const Box Box::NIL = Box(Coord::NIL, Coord::NIL);

Box::Box(const Coord& min, const Coord& max) :
    min(min),
    max(max)
{
}

Box inet::Box::computeBoundingBox(const std::vector<Coord>& points)
{
    Coord min = Coord::NIL;
    Coord max = Coord::NIL;
    if (points.begin() != points.end()) {
        min = *points.begin();
        max = min;
    }
    for (const auto& point : points) {
        min = min.min(point);
        max = max.max(point);
    }
    return Box(min, max);
}

} /* namespace inet */

