//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IOBJECTCACHE_H
#define __INET_IOBJECTCACHE_H

#include "inet/common/IVisitor.h"
#include "inet/common/geometry/object/LineSegment.h"
#include "inet/environment/contract/IPhysicalObject.h"

namespace inet {

namespace physicalenvironment {

/**
 * This interface provides abstractions for efficient physical object cache data
 * structures.
 */
class INET_API IObjectCache
{
  public:
    /**
     * Calls the visitor with at least all physical objects that intersect
     * with the provided line segment.
     */
    virtual void visitObjects(const IVisitor *visitor, const LineSegment& lineSegment) const = 0;
};

} // namespace physicalenvironment

} // namespace inet

#endif

