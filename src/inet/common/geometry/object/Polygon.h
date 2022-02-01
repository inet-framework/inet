//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_POLYGON_H
#define __INET_POLYGON_H

#include "inet/common/geometry/base/GeometricObjectBase.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/object/LineSegment.h"

namespace inet {

class INET_API Polygon : public GeometricObjectBase
{
  public:
    static const Polygon NIL;

  protected:
    std::vector<Coord> points;

  protected:
    Coord getEdgeOutwardNormalVector(const Coord& edgeP1, const Coord& edgeP2) const;

  public:
    Polygon() {}
    Polygon(const std::vector<Coord>& points);

    const std::vector<Coord>& getPoints() const { return points; }

    virtual bool isNil() const override { return this == &NIL; }
    virtual bool isUnspecified() const override;

    virtual Coord computeSize() const;
    Coord getNormalUnitVector() const;
    Coord getNormalVector() const;
    virtual bool computeIntersection(const LineSegment& lineSegment, Coord& intersection1, Coord& intersection2, Coord& normal1, Coord& normal2) const;
};

} // namespace inet

#endif

