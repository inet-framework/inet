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

#ifndef __INET_GRIDOBJECTCACHE_H_
#define __INET_GRIDOBJECTCACHE_H_

#include "IObjectCache.h"
#include "SpatialGrid.h"
#include "PhysicalEnvironment.h"
#include "PhysicalObject.h"
#include "IVisitor.h"

namespace inet {

class GridObjectCache : public IObjectCache, public cSimpleModule
{
    protected:
        SpatialGrid *grid;
        PhysicalEnvironment *physicalEnvironment;

    protected:
        virtual int numInitStages() const { return NUM_INIT_STAGES; }
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg) { throw cRuntimeError("This module doesn't handle self messages"); }

    public:
        bool insertObject(const PhysicalObject *object);
        void visitObjects(const IVisitor *visitor, const LineSegment& lineSegment) const;
        void buildCache() {}
        GridObjectCache();
        virtual ~GridObjectCache();
};

} /* namespace inet */

#endif /* __INET_GRIDOBJECTCACHE_H_ */
