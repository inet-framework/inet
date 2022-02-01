//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

    virtual bool isNil() const override { return this == &NIL; }
    virtual bool isUnspecified() const override { return basePoint.isUnspecified() || normalVector.isUnspecified(); }

    Coord computeIntersection(const LineSegment& lineSegment);
};

} // namespace inet

#endif

