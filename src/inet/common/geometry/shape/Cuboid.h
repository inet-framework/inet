//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CUBOID_H
#define __INET_CUBOID_H

#include "inet/common/geometry/base/ShapeBase.h"
#include "inet/common/geometry/common/RotationMatrix.h"

namespace inet {

/**
 * This class represents a 3 dimensional shape with 6 pairwise parallel faces.
 */
class INET_API Cuboid : public ShapeBase
{
  protected:
    Coord size;

  protected:
    bool isInsideX(const Coord& point) const { return -size.x / 2 <= point.x && point.x <= size.x / 2; }
    bool isInsideY(const Coord& point) const { return -size.y / 2 <= point.y && point.y <= size.y / 2; }
    bool isInsideZ(const Coord& point) const { return -size.z / 2 <= point.z && point.z <= size.z / 2; }

  public:
    Cuboid(const Coord& size);
    const Coord& getSize() const { return size; }
    void setSize(const Coord& size) { this->size = size; }
    virtual Coord computeBoundingBoxSize() const override { return size; }
    virtual bool computeIntersection(const LineSegment& lineSegment, Coord& intersection1, Coord& intersection2, Coord& normal1, Coord& normal2) const override;
    virtual void computeVisibleFaces(std::vector<std::vector<Coord>>& faces, const RotationMatrix& rotation, const RotationMatrix& viewRotation) const;
};

} // namespace inet

#endif

