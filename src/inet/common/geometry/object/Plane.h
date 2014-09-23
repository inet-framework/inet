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

#ifndef __INET_PLANE_H
#define __INET_PLANE_H

#include "inet/common/geometry/base/GeometricObjectBase.h"
#include "inet/common/geometry/object/LineSegment.h"

namespace inet {

/**
 * This class represents a 2 dimensional plane in the 3 dimensional space.
 */
class INET_API Plane : public GeometricObjectBase
{
  public:
    static const Plane NIL;

  protected:
    Coord basePoint;
    Coord normalVector;

  public:
    Plane();
    Plane(const Coord& basePoint, const Coord& normalVector);

    const Coord& getBasePoint() { return basePoint; }
    void setBasePoint(const Coord& basePoint) { this->basePoint = basePoint; }
    const Coord& getNormalVector() { return normalVector; }
    void setNormalVector(const Coord& normalVector) { this->normalVector = normalVector; }

    virtual bool isNil() const { return this == &NIL; }
    virtual bool isUnspecified() const { return basePoint.isUnspecified() || normalVector.isUnspecified(); }

    Coord computeIntersection(const LineSegment& lineSegment);
};

} // namespace inet

#endif // ifndef __INET_PLANE_H

