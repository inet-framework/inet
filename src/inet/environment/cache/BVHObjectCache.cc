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

#include "inet/environment/cache/BVHObjectCache.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

Define_Module(BVHObjectCache);

BVHObjectCache::BVHObjectCache() :
    physicalEnvironment(NULL),
    leafCapacity(0),
    axisOrder(NULL),
    bvhTree(NULL)
{
}

BVHObjectCache::~BVHObjectCache()
{
    delete bvhTree;
}

void BVHObjectCache::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        physicalEnvironment = getModuleFromPar<PhysicalEnvironment>(par("physicalEnvironmentModule"), this);
        leafCapacity = par("leafCapacity");
        axisOrder = par("axisOrder");
    }
}

bool BVHObjectCache::insertObject(const PhysicalObject *object)
{
    if (bvhTree) {
        delete bvhTree;
        bvhTree = NULL;
    }
    objects.push_back(object);
    return true;
}

void BVHObjectCache::visitObjects(const IVisitor *visitor, const LineSegment& lineSegment) const
{
    if (!bvhTree)
        bvhTree = new BVHTree(physicalEnvironment->getSpaceMin(), physicalEnvironment->getSpaceMax(), objects, 0, objects.size() - 1, BVHTree::Axis(axisOrder), leafCapacity);
    bvhTree->lineSegmentQuery(lineSegment, visitor);
}

} // namespace inet

