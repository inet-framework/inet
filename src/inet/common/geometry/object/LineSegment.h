//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

    virtual bool isNil() const override { return this == &NIL; }
    virtual bool isUnspecified() const override { return point1.isUnspecified() || point2.isUnspecified(); }

    bool computeIntersection(const LineSegment& lineSegment, Coord& intersection1, Coord& intersection2);
};

} // namespace inet

#endif

