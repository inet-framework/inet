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

#ifndef __INET_SHAPE_H
#define __INET_SHAPE_H

#include "LineSegment.h"

namespace inet {

/**
 * This class represents a 3 dimensional convex shape independently of its
 * position and orientation.
 */
class INET_API Shape : public cObject
{
  public:
    Shape();
    virtual ~Shape();

    virtual Coord computeSize() const = 0;
    virtual bool computeIntersection(const LineSegment& lineSegment, Coord& intersection1, Coord& intersection2, Coord& normal1, Coord& normal2) const = 0;
};

} // namespace inet

#endif // ifndef __INET_SHAPE_H

