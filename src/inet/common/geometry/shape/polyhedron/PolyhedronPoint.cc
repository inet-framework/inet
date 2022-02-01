//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/geometry/shape/polyhedron/PolyhedronPoint.h"

namespace inet {

PolyhedronPoint::PolyhedronPoint(const Coord& point)
{
    selected = false;
    x = point.x;
    y = point.y;
    z = point.z;
}

PolyhedronPoint::PolyhedronPoint()
{
    selected = false;
    x = y = z = 0;
}

} /* namespace inet */

