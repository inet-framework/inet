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

#ifndef __INET_CUBOID_H
#define __INET_CUBOID_H

#include "Shape.h"

namespace inet {

/**
 * This class represents a 3 dimensional shape with 6 pairwise parallel faces.
 * The coordinate system origin is in the center of the cuboid and the faces
 * are parallel to the axes.
 */
class INET_API Cuboid : public Shape
{
  protected:
    Coord size;

  protected:
    bool isInsideX(const Coord& point) const { return -size.x / 2 <= point.x && point.x <= size.x / 2; }
    bool isInsideY(const Coord& point) const { return -size.x / 2 <= point.y && point.y <= size.y / 2; }
    bool isInsideZ(const Coord& point) const { return -size.x / 2 <= point.z && point.z <= size.z / 2; }

  public:
    Cuboid(const Coord& size);

    const Coord& getSize() const { return size; }
    void setSize(const Coord& size) { this->size = size; }

    virtual bool computeIntersection(const LineSegment& lineSegment, Coord& intersection1, Coord& intersection2) const;
};

} // namespace inet

#endif // ifndef __INET_CUBOID_H

