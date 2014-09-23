//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_IOBJECTCACHE_H
#define __INET_IOBJECTCACHE_H

#include "inet/common/IVisitor.h"
#include "inet/environment/PhysicalObject.h"
#include "inet/common/geometry/object/LineSegment.h"

namespace inet {

/**
 * This interface provides abstractions for efficient physical object cache data
 * structures.
 */
class IObjectCache
{
  public:
    /**
     * Inserts a new physical object into the cache data structure.
     */
    virtual bool insertObject(const PhysicalObject *object) = 0;

    /**
     * Calls the visitor with at least all physical objects that intersect
     * with the provided line segment.
     */
    virtual void visitObjects(const IVisitor *visitor, const LineSegment& lineSegment) const = 0;
};

} // namespace inet

#endif // ifndef __INET_IOBJECTCACHE_H
