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

#include "Prism.h"

namespace inet {

Prism::Prism(double height, const Polygon& base) :
    height(height),
    base(base)
{
}

Coord Prism::computeSize() const
{
    Coord min;
    Coord max;
    for (std::vector<Coord>::const_iterator it = base.getPoints().begin(); it != base.getPoints().end(); it++) {
        min = min.min(*it);
        max = max.max(*it);
    }
    return max - min;
}

bool Prism::computeIntersection(const LineSegment& lineSegment, Coord& intersection1, Coord& intersection2, Coord& normal1, Coord& normal2) const
{
    throw cRuntimeError("Unimplemented");
}

} // namespace inet

