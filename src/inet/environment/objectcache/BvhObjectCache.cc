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

#include "inet/common/ModuleAccess.h"
#include "inet/environment/objectcache/BvhObjectCache.h"

namespace inet {

namespace physicalenvironment {

Define_Module(BvhObjectCache);

BvhObjectCache::BvhObjectCache() :
    physicalEnvironment(nullptr),
    leafCapacity(0),
    axisOrder(nullptr),
    bvhTree(nullptr)
{
}

BvhObjectCache::~BvhObjectCache()
{
    delete bvhTree;
}

void BvhObjectCache::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        physicalEnvironment = getModuleFromPar<PhysicalEnvironment>(par("physicalEnvironmentModule"), this);
        leafCapacity = par("leafCapacity");
        axisOrder = par("axisOrder");
    }
    else if (stage == INITSTAGE_PHYSICAL_OBJECT_CACHE)
    {
        for (int i = 0; i < physicalEnvironment->getNumObjects(); i++)
            insertObject(physicalEnvironment->getObject(i));
    }
}

bool BvhObjectCache::insertObject(const IPhysicalObject *object)
{
    if (bvhTree) {
        delete bvhTree;
        bvhTree = nullptr;
    }
    objects.push_back(object);
    return true;
}

void BvhObjectCache::visitObjects(const IVisitor *visitor, const LineSegment& lineSegment) const
{
    if (!bvhTree)
#ifdef _OPENMP
#pragma omp critical
#endif
        bvhTree = new BvhTree(physicalEnvironment->getSpaceMin(), physicalEnvironment->getSpaceMax(), objects, 0, objects.size() - 1, BvhTree::Axis(axisOrder), leafCapacity);
    bvhTree->lineSegmentQuery(lineSegment, visitor);
}

} // namespace physicalenvironment

} // namespace inet

