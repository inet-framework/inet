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

#include "Cuboid.h"
#include "Polygon.h"

namespace inet {

Cuboid::Cuboid(const Coord& size) :
    size(size), prism(NULL)
{
    std::vector<Coord> polygonPoints;
    polygonPoints.push_back(Coord(-size.x / 2, -size.y / 2, -size.z / 2));
    polygonPoints.push_back(Coord(-size.x / 2, size.y / 2, -size.z / 2));
    polygonPoints.push_back(Coord(size.x / 2, size.y / 2, -size.z / 2));
    polygonPoints.push_back(Coord(size.x / 2, -size.y / 2, -size.z / 2));
    Polygon base(polygonPoints);
    prism = new Prism(size.z, base);
}

bool Cuboid::computeIntersection(const LineSegment& lineSegment,Coord& intersection1, Coord& intersection2, Coord& normal1, Coord& normal2) const
{
    return prism->computeIntersection(lineSegment, intersection1, intersection2, normal1, normal2);
}

Cuboid::~Cuboid()
{
    delete prism;
}

} // namespace inet

