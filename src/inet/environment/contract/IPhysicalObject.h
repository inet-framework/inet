//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPHYSICALOBJECT_H
#define __INET_IPHYSICALOBJECT_H

#include "inet/common/geometry/base/ShapeBase.h"
#include "inet/common/geometry/common/Quaternion.h"
#include "inet/environment/contract/IMaterial.h"

namespace inet {

namespace physicalenvironment {

class INET_API IPhysicalObject
{
  public:
    virtual const Coord& getPosition() const = 0;
    virtual const Quaternion& getOrientation() const = 0;

    virtual const ShapeBase *getShape() const = 0;
    virtual const IMaterial *getMaterial() const = 0;

    virtual double getLineWidth() const = 0;
    virtual const cFigure::Color& getLineColor() const = 0;
    virtual const cFigure::Color& getFillColor() const = 0;
    virtual double getOpacity() const = 0;
    virtual const char *getTexture() const = 0;
    virtual const char *getTags() const = 0;
};

} // namespace physicalenvironment

} // namespace inet

#endif

