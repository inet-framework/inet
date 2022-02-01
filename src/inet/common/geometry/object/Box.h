//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BOX_H
#define __INET_BOX_H

#include "inet/common/geometry/base/GeometricObjectBase.h"
#include "inet/common/geometry/common/Coord.h"

namespace inet {

class INET_API Box : public GeometricObjectBase
{
  public:
    static const Box NIL;

  protected:
    Coord min;
    Coord max;

  public:
    Box(const Coord& min, const Coord& max);

    static Box computeBoundingBox(const std::vector<Coord>& points);

    const Coord& getMin() const { return min; }
    const Coord& getMax() const { return max; }

    Coord getSize() const { return max - min; }
    Coord getCenter() const { return (min + max) / 2; }

    virtual bool isNil() const override { return this == &NIL; }
    virtual bool isUnspecified() const override { return min.isUnspecified() || max.isUnspecified(); }
};
} /* namespace inet */

#endif

