//
// Copyright (C) 2013 OpenSim Ltd.
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

