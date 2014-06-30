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

#ifndef __INET_SPHERE_H_
#define __INET_SPHERE_H_

#include "Shape.h"

namespace inet {

/**
 * This class represents a 3 dimensional sphere shape. The coordinate system
 * origin is in the center of the sphere.
 */
class INET_API Sphere : public Shape
{
    protected:
        double radius;

    public:
        Sphere(double radius);

        const double getRadius() const { return radius; }
        void setRadius(double radius) { this->radius = radius; }

        virtual bool computeIntersection(const LineSegment& lineSegment, Coord& intersection1, Coord& intersection2) const;
};

} // namespace inet

#endif
