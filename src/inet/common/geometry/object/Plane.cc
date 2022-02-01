//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/geometry/object/Plane.h"

namespace inet {

const Plane Plane::NIL(Coord::NIL, Coord::NIL);

Plane::Plane() :
    basePoint(Coord::NIL),
    normalVector(Coord::NIL)
{
}

Plane::Plane(const Coord& basePoint, const Coord& normalVector) :
    basePoint(basePoint),
    normalVector(normalVector)
{
}

Coord Plane::computeIntersection(const LineSegment& lineSegment)
{
    // NOTE: based on http://paulbourke.net/geometry/pointlineplane/
    const Coord& point1 = lineSegment.getPoint1();
    const Coord& point2 = lineSegment.getPoint2();
    double denominator = normalVector * (point2 - point1);
    if (denominator == 0)
        return Coord::NIL;
    else {
        double numerator = normalVector * (basePoint - point1);
        double q = numerator / denominator;
        if (q < 0 || q > 1)
            return Coord::NIL;
        else
            return point1 * (1 - q) + point2 * q;
    }
}

} // namespace inet

