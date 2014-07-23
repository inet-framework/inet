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

#include "BVHObjectCache.h"
#include "ModuleAccess.h"

namespace inet {

Define_Module(BVHObjectCache);

BVHObjectCache::BVHObjectCache() :
        bvhTree(NULL),
        physicalEnvironment(NULL)
{

}

void BVHObjectCache::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        physicalEnvironment = getModuleFromPar<PhysicalEnvironment>(par("physicalEnvironmentModule"), this);
}

BVHObjectCache::~BVHObjectCache()
{
    delete bvhTree;
}

bool BVHObjectCache::insertObject(const PhysicalObject *object)
{
    objects.push_back(object);
    return true;
}

void BVHObjectCache::visitObjects(const IVisitor *visitor, const LineSegment& lineSegment) const
{
    bvhTree->lineSegmentQuery(lineSegment, visitor);
}

void BVHObjectCache::buildCache()
{
    bvhTree = new BVHTree(physicalEnvironment->getSpaceMin(), physicalEnvironment->getSpaceMax(), objects, 0, objects.size() - 1, BVHTree::Axis::X);
    //bvhTree->traverse();
}

} /* namespace inet */
