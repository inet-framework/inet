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

#include "inet/common/geometry/shape/Sphere.h"
#include "inet/common/geometry/object/Plane.h"

namespace inet {

Sphere::Sphere(double radius) :
    radius(radius)
{
}

bool Sphere::computeIntersection(const LineSegment& lineSegment, Coord& intersection1, Coord& intersection2, Coord& normal1, Coord& normal2) const
{
    // NOTE: based on http://paulbourke.net/geometry/circlesphere/
    Coord point1 = lineSegment.getPoint1();
    Coord point2 = lineSegment.getPoint2();
    double a, b, c;
    double bb4ac;
    Coord dp = point2 - point1;
    a = dp * dp;
    b = 2 * (dp.x * point1.x + dp.y * point1.y + dp.z * point1.z);
    c = point1 * point1 - radius * radius;
    bb4ac = b * b - 4 * a * c;
    if (bb4ac >= 0) {
        double alpha1 = (-b + sqrt(bb4ac)) / (2 * a);
        double alpha2 = (-b - sqrt(bb4ac)) / (2 * a);
        bool inside1 = false;
        if (alpha1 < 0)
            alpha1 = 0;
        else if (alpha1 > 1)
            alpha1 = 1;
        else
            inside1 = true;
        bool inside2 = false;
        if (alpha2 < 0)
            alpha2 = 0;
        else if (alpha2 > 1)
            alpha2 = 1;
        else
            inside2 = true;
        if (alpha1 == alpha2)
            return false;
        else {
            intersection1 = point1 * (1 - alpha1) + point2 * alpha1;
            intersection2 = point1 * (1 - alpha2) + point2 * alpha2;
            if (inside1)
                normal1 = intersection1 / intersection1.length();
            if (inside2)
                normal2 = intersection2 / intersection2.length();
            return true;
        }
    }
    else
        return false;
}

} // namespace inet

