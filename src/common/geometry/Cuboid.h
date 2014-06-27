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

#ifndef __INET_CUBOID_H_
#define __INET_CUBOID_H_

#include "Shape.h"

class INET_API Cuboid : public Shape
{
    protected:
        Coord min;
        Coord max;

    protected:
        bool isInsideX(const Coord& point) const { return min.x <= point.x && point.x <= max.x; }
        bool isInsideY(const Coord& point) const { return min.y <= point.y && point.y <= max.y; }
        bool isInsideZ(const Coord& point) const { return min.z <= point.z && point.z <= max.z; }

    public:
        Cuboid(const Coord& min, const Coord& max);

        virtual bool isIntersectingLineSegment(const LineSegment& lineSegment) const;
        virtual double computeIntersectionDistance(const LineSegment& lineSegment) const;
};

#endif
