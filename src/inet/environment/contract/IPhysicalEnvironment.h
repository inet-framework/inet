//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPHYSICALENVIRONMENT_H
#define __INET_IPHYSICALENVIRONMENT_H

#include "inet/common/IVisitor.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/Quaternion.h"
#include "inet/common/geometry/common/RotationMatrix.h"
#include "inet/environment/contract/IGround.h"
#include "inet/environment/contract/IMaterialRegistry.h"
#include "inet/environment/contract/IObjectCache.h"
#include "inet/environment/contract/IPhysicalObject.h"

namespace inet {

namespace physicalenvironment {

class INET_API IPhysicalEnvironment
{
  public:
    virtual IObjectCache *getObjectCache() const = 0;
    virtual IGround *getGround() const = 0;

    virtual const Coord& getSpaceMin() const = 0;
    virtual const Coord& getSpaceMax() const = 0;
    virtual const IMaterialRegistry *getMaterialRegistry() const = 0;

    virtual int getNumObjects() const = 0;
    virtual const IPhysicalObject *getObject(int index) const = 0;
    virtual const IPhysicalObject *getObjectById(int id) const = 0;

    virtual void visitObjects(const IVisitor *visitor, const LineSegment& lineSegment) const = 0;
};

} // namespace physicalenvironment

} // namespace inet

#endif

