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
    double tN = std::max(std::max(t1.x, t1.y), t1.z);
    double tF = std::min(std::min(t2.x, t2.y), t2.z);
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

void Cuboid::computeVisibleFaces(std::vector<std::vector<Coord> >& faces, const RotationMatrix& rotation, const RotationMatrix& viewRotation) const
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

