//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SPHERE_H
#define __INET_SPHERE_H

#include "inet/common/geometry/base/ShapeBase.h"

namespace inet {

/**
 * This class represents a 3 dimensional sphere shape.
 */
class INET_API Sphere : public ShapeBase
{
  protected:
    double radius;

  public:
    Sphere(double radius);

    double getRadius() const { return radius; }
    void setRadius(double radius) { this->radius = radius; }

    virtual Coord computeBoundingBoxSize() const override { return Coord(radius * 2, radius * 2, radius * 2); }
    virtual bool computeIntersection(const LineSegment& lineSegment, Coord& intersection1, Coord& intersection2, Coord& normal1, Coord& normal2) const override;
};

} // namespace inet

#endif

