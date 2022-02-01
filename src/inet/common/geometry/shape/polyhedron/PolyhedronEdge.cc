//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/geometry/shape/polyhedron/PolyhedronEdge.h"

namespace inet {

PolyhedronPoint PolyhedronEdge::getEdgeVector() const
{
    return *point2 - *point1;
}

bool PolyhedronEdge::operator==(const PolyhedronEdge& rhs) const
{
    return (point1 == rhs.point1 && point2 == rhs.point2) ||
           (point1 == rhs.point2 && point2 == rhs.point1);
}

} /* namespace inet */

