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

#include "Cuboid.h"
#include "Plane.h"

namespace inet {

Cuboid::Cuboid(const Coord& size) :
    size(size)
{
}

bool Cuboid::computeIntersection(const LineSegment& lineSegment, Coord& intersection1, Coord& intersection2, Coord& normal1, Coord& normal2) const
{
    int i = 0;
    Coord points[2];
    Coord min = size / -2;
    Coord max = size / 2;
    const Coord& point1 = lineSegment.getPoint1();
    const Coord& point2 = lineSegment.getPoint2();
    if (isInsideX(point1) && isInsideY(point1) && isInsideZ(point1))
        points[i++ % 2] = point1;
    if (isInsideX(point2) && isInsideY(point2) && isInsideZ(point2))
        points[i++ % 2] = point2;
    Coord xMin = Plane(min, Coord(1, 0, 0)).computeIntersection(lineSegment);
    if (!xMin.isUnspecified() && isInsideY(xMin) && isInsideZ(xMin))
        points[i++ % 2] = xMin;
    Coord xMax = Plane(max, Coord(-1, 0, 0)).computeIntersection(lineSegment);
    if (!xMax.isUnspecified() && isInsideY(xMax) && isInsideZ(xMax))
        points[i++ % 2] = xMax;
    Coord yMin = Plane(min, Coord(0, 1, 0)).computeIntersection(lineSegment);
    if (!yMin.isUnspecified() && isInsideX(yMin) && isInsideZ(yMin))
        points[i++ % 2] = yMin;
    Coord yMax = Plane(max, Coord(0, -1, 0)).computeIntersection(lineSegment);
    if (!yMax.isUnspecified() && isInsideX(yMax) && isInsideZ(yMax))
        points[i++ % 2] = yMax;
    Coord zMin = Plane(min, Coord(0, 0, 1)).computeIntersection(lineSegment);
    if (!zMin.isUnspecified() && isInsideX(zMin) && isInsideY(zMin))
        points[i++ % 2] = zMin;
    Coord zMax = Plane(max, Coord(0, 0, -1)).computeIntersection(lineSegment);
    if (!zMax.isUnspecified() && isInsideX(zMax) && isInsideY(zMax))
        points[i++ % 2] = zMax;
    // TODO: what about the degenerate cases when there are more than two points?
    if (i < 2)
        return false;
    else {
        // TODO: face normal vectors
        intersection1 = points[0];
        intersection2 = points[1];
        return true;
    }
}

} // namespace inet

