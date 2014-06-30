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

#include "Shape.h"

Shape::Shape()
{
}

Shape::~Shape()
{
}

bool Shape::isIntersecting(const LineSegment& lineSegment) const
{
    Coord intersection1;
    Coord intersection2;
    return computeIntersection(lineSegment, intersection1, intersection2);
}

double Shape::computeIntersectionDistance(const LineSegment& lineSegment) const
{
    Coord intersection1;
    Coord intersection2;
    if (computeIntersection(lineSegment, intersection1, intersection2))
        return intersection2.distance(intersection1);
    else
        return 0;
}
