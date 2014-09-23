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

#include "inet/common/geometry/shape/Cuboid.h"
#include "inet/common/geometry/object/Polygon.h"
#include "inet/common/geometry/object/Plane.h"
#include "inet/common/geometry/shape/Prism.h"

namespace inet {

Cuboid::Cuboid(const Coord& size) :
    size(size)
{
}

bool Cuboid::computeIntersection(const LineSegment& lineSegment, Coord& intersection1, Coord& intersection2, Coord& normal1, Coord& normal2) const
{
    // TODO: heavily used function, optimize
    Coord minPoint = size / -2;
    Coord maxPoint = size / 2;
    intersection1 = intersection2 = normal1 = normal2 = Coord::NIL;
    const Coord& point1 = lineSegment.getPoint1();
    const Coord& point2 = lineSegment.getPoint2();
    bool isInside1 = isInsideX(point1) && isInsideY(point1) && isInsideZ(point1);
    bool isInside2 = isInsideX(point2) && isInsideY(point2) && isInsideZ(point2);
    if (isInside1)
        intersection1 = point1;
    if (isInside2)
        intersection2 = point2;
    if (isInside1 && isInside2)
        return true;
    // x min
    Coord xMinNormal(-1, 0, 0);
    Coord xMinIntersecion = Plane(minPoint, xMinNormal).computeIntersection(lineSegment);
    if (!xMinIntersecion.isNil() && isInsideY(xMinIntersecion) && isInsideZ(xMinIntersecion)) {
        if (point1.x < minPoint.x) {
            intersection1 = xMinIntersecion;
            normal1 = xMinNormal;
        }
        else {
            intersection2 = xMinIntersecion;
            normal2 = xMinNormal;
        }
    }
    // x max
    Coord xMaxNormal(1, 0, 0);
    Coord xMaxIntersection = Plane(maxPoint, xMaxNormal).computeIntersection(lineSegment);
    if (!xMaxIntersection.isNil() && isInsideY(xMaxIntersection) && isInsideZ(xMaxIntersection)) {
        if (point1.x > maxPoint.x) {
            intersection1 = xMaxIntersection;
            normal1 = xMaxNormal;
        }
        else {
            intersection2 = xMaxIntersection;
            normal2 = xMaxNormal;
        }
    }
    // y min
    Coord yMinNormal(0, -1, 0);
    Coord yMinIntersection = Plane(minPoint, yMinNormal).computeIntersection(lineSegment);
    if (!yMinIntersection.isNil() && isInsideX(yMinIntersection) && isInsideZ(yMinIntersection)) {
        if (point1.y < minPoint.y) {
            intersection1 = yMinIntersection;
            normal1 = yMinNormal;
        }
        else {
            intersection2 = yMinIntersection;
            normal2 = yMinNormal;
        }
    }
    // y max
    Coord yMaxNormal(0, 1, 0);
    Coord yMaxIntersection = Plane(maxPoint, yMaxNormal).computeIntersection(lineSegment);
    if (!yMaxIntersection.isNil() && isInsideX(yMaxIntersection) && isInsideZ(yMaxIntersection)) {
        if (point1.y > maxPoint.y) {
            intersection1 = yMaxIntersection;
            normal1 = yMaxNormal;
        }
        else {
            intersection2 = yMaxIntersection;
            normal2 = yMaxNormal;
        }
    }
    // z min
    Coord zMinNormal(0, 0, -1);
    Coord zMinIntersection = Plane(minPoint, zMinNormal).computeIntersection(lineSegment);
    if (!zMinIntersection.isNil() && isInsideX(zMinIntersection) && isInsideY(zMinIntersection)) {
        if (point1.z < minPoint.z) {
            intersection1 = zMinIntersection;
            normal1 = zMinNormal;
        }
        else {
            intersection2 = zMinIntersection;
            normal2 = zMinNormal;
        }
    }
    // z max
    Coord zMaxNormal(0, 0, 1);
    Coord zMaxIntersection = Plane(maxPoint, zMaxNormal).computeIntersection(lineSegment);
    if (!zMaxIntersection.isNil() && isInsideX(zMaxIntersection) && isInsideY(zMaxIntersection)) {
        if (point1.z > maxPoint.z) {
            intersection1 = zMaxIntersection;
            normal1 = zMaxNormal;
        }
        else {
            intersection2 = zMaxIntersection;
            normal2 = zMaxNormal;
        }
    }
    return !intersection1.isUnspecified() && !intersection2.isUnspecified();
}

void Cuboid::computeVisibleFaces(std::vector<std::vector<Coord> >& faces, const Rotation& rotation, const Rotation& viewRotation) const
{
    // TODO: specialize
    std::vector<Coord> polygonPoints;
    polygonPoints.push_back(Coord(-size.x / 2, -size.y / 2, -size.z / 2));
    polygonPoints.push_back(Coord(-size.x / 2, size.y / 2, -size.z / 2));
    polygonPoints.push_back(Coord(size.x / 2, size.y / 2, -size.z / 2));
    polygonPoints.push_back(Coord(size.x / 2, -size.y / 2, -size.z / 2));
    Polygon base(polygonPoints);
    Prism prism(size.z, base);
    prism.computeVisibleFaces(faces, rotation, viewRotation);
}

} // namespace inet

