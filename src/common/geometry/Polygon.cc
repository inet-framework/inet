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

#include "Polygon.h"

namespace inet {

Polygon::Polygon(const std::vector<Coord>& points)
{
    if (points.size() < 3)
        throw cRuntimeError("A Euclidean polygon has at least three points");
    this->points = points;
}

Coord Polygon::getNormalUnitVector() const
{
    Coord point1 = points[0];
    Coord point2 = points[1];
    Coord point3 = points[2];
    Coord vectorA = point2 - point1;
    Coord vectorB = point3 - point1;
    Coord vectorC(vectorA.y * vectorB.z - vectorA.z * vectorB.y,
                 vectorA.z * vectorB.x - vectorA.x * vectorB.z,
                 vectorA.x * vectorB.y - vectorA.y * vectorB.x);
    return vectorC / vectorC.length();
}

} // namespace inet

