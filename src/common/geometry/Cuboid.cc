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
#include "Plane.h"

namespace inet {

Cuboid::Cuboid(const Coord& size) :
    size(size)
{
    std::vector<Coord> polygonPoints;
    polygonPoints.push_back(Coord(-size.x / 2, -size.y / 2, 0));
    polygonPoints.push_back(Coord(-size.x / 2, size.y / 2, 0));
    polygonPoints.push_back(Coord(size.x / 2, size.y / 2, 0));
    polygonPoints.push_back(Coord(size.x / 2, -size.y / 2, 0));
    setHeight(size.z);
    Polygon base(polygonPoints);
    setBase(base);
}

} // namespace inet

