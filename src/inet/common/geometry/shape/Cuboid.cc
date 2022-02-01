//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/geometry/shape/Cuboid.h"

#include "inet/common/geometry/object/Plane.h"
#include "inet/common/geometry/object/Polygon.h"
#include "inet/common/geometry/shape/Prism.h"

namespace inet {

Cuboid::Cuboid(const Coord& size) :
    size(size)
{
}

bool Cuboid::computeIntersection(const LineSegment& lineSegment, Coord& intersection1, Coord& intersection2, Coord& normal1, Coord& normal2) const
{
    // NOTE: based on https://www.iquilezles.org/www/articles/intersectors/intersectors.htm
    const auto p1 = lineSegment.getPoint1();
    const auto p2 = lineSegment.getPoint2();
    Coord direction = p1 == p2 ? Coord::ZERO : (p2 - p1).getNormalized();
    Coord m = Coord::ONE.getDividedElementwise(direction);
    m.clamp(-1E+100, 1E+100); // fix axis algined direction
    Coord n = m.getMultipliedElementwise(p1);
    Coord k = m.getAbs().getMultipliedElementwise(size / 2);
    Coord t1 = -n - k;
    Coord t2 = -n + k;
    double tN = math::maxnan(math::maxnan(t1.x, t1.y), t1.z);
    double tF = math::minnan(math::minnan(t2.x, t2.y), t2.z);
    double l = lineSegment.length();
    if (tN > tF || tF < 0.0 || tN > l) {
        intersection1 = intersection2 = normal1 = normal2 = Coord::NIL;
        return false;
    }
    else {
        if (tN < 0) {
            intersection1 = p1;
            normal1 = Coord::NIL;
        }
        else {
            intersection1 = p1 + direction * tN;
            normal1 = -direction.getSign().getMultipliedElementwise(t1.getYzx().getStep(t1).getMultipliedElementwise(t1.getZxy().getStep(t1)));
        }
        if (l < tF) {
            intersection2 = p2;
            normal2 = Coord::NIL;
        }
        else {
            intersection2 = p1 + direction * tF;
            normal2 = -direction.getSign().getMultipliedElementwise(t2.getYzx().getStep(t2).getMultipliedElementwise(t2.getZxy().getStep(t2)));
        }
        return true;
    }
}

void Cuboid::computeVisibleFaces(std::vector<std::vector<Coord>>& faces, const RotationMatrix& rotation, const RotationMatrix& viewRotation) const
{
    // TODO specialize
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

