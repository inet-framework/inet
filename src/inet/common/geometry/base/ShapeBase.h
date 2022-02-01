//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SHAPEBASE_H
#define __INET_SHAPEBASE_H

#include "inet/common/geometry/object/LineSegment.h"

namespace inet {

/**
 * This class represents a 3 dimensional geometric shape independently of its
 * position and orientation. Moving a shape around, rotating it, or reflecting
 * it in a mirror is the same shape as the original, and not a distinct shape.
 */
class INET_API ShapeBase
{
  public:
    ShapeBase() {}
    virtual ~ShapeBase() {}

    /**
     * Computes the 3 dimensional size of the shapes's bounding box.
     */
    virtual Coord computeBoundingBoxSize() const = 0;

    /**
     * Computes the intersection with the given line segment in the shape's
     * coordinate system
     */
    virtual bool computeIntersection(const LineSegment& lineSegment, Coord& intersection1, Coord& intersection2, Coord& normal1, Coord& normal2) const = 0;
};

} // namespace inet

#endif

