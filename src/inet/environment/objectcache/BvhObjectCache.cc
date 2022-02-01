//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/environment/objectcache/BvhObjectCache.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace physicalenvironment {

Define_Module(BvhObjectCache);

BvhObjectCache::BvhObjectCache() :
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
    if (stage == INITSTAGE_LOCAL) {
        physicalEnvironment.reference(this, "physicalEnvironmentModule", true);
        leafCapacity = par("leafCapacity");
        axisOrder = par("axisOrder");
    }
    else if (stage == INITSTAGE_PHYSICAL_OBJECT_CACHE) {
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

