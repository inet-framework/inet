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

#include "inet/common/geometry/object/LineSegment.h"

namespace inet {

const LineSegment LineSegment::NIL(Coord::NIL, Coord::NIL);

LineSegment::LineSegment() :
    point1(Coord::NIL),
    point2(Coord::NIL)
{
}

LineSegment::LineSegment(const Coord& point1, const Coord& point2) :
    point1(point1),
    point2(point2)
{
}

bool LineSegment::computeIntersection(const LineSegment& lineSegment, Coord &intersection1, Coord &intersection2)
{
    // TODO:
    throw cRuntimeError("Not yet implemented");
}

} // namespace inet

