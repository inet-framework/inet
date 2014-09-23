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

#ifndef __INET_LINESEGMENT_H
#define __INET_LINESEGMENT_H

#include "inet/common/geometry/base/GeometricObjectBase.h"
#include "inet/common/geometry/common/Coord.h"

namespace inet {

/**
 * This class represents a 3 dimensional line segment between two points.
 */
class INET_API LineSegment : public GeometricObjectBase
{
  public:
    static const LineSegment NIL;

  protected:
    Coord point1;
    Coord point2;

  public:
    LineSegment();
    LineSegment(const Coord& point1, const Coord& point2);

    const Coord& getPoint1() const { return point1; }
    void setPoint1(const Coord& point1) { this->point1 = point1; }
    const Coord& getPoint2() const { return point2; }
    void setPoint2(const Coord& point2) { this->point2 = point2; }
    double length() const { return point2.distance(point1); }

    virtual bool isNil() const { return this == &NIL; }
    virtual bool isUnspecified() const { return point1.isUnspecified() || point2.isUnspecified(); }

    bool computeIntersection(const LineSegment &lineSegment, Coord &intersection1, Coord &intersection2);
};

} // namespace inet

#endif // ifndef __INET_LINESEGMENT_H

