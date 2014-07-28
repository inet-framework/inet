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

#include "GridObjectCache.h"
#include "ModuleAccess.h"

namespace inet {

Define_Module(GridObjectCache);

GridObjectCache::GridObjectCache() :
        grid(NULL),
        physicalEnvironment(NULL)
{

}

bool GridObjectCache::insertObject(const PhysicalObject *object)
{
    Coord pos = object->getPosition();
    Coord boundingBoxSize = object->getShape()->computeSize();
    grid->insertObject(object, pos, boundingBoxSize);
    return true;
}

void GridObjectCache::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        Coord voxelSizes(par("cellSizeX"), par("cellSizeY"), par("cellSizeZ"));
        physicalEnvironment = getModuleFromPar<PhysicalEnvironment>(par("physicalEnvironmentModule"), this);
        grid = new SpatialGrid(voxelSizes, physicalEnvironment->getSpaceMin(), physicalEnvironment->getSpaceMax());
    }
}

void GridObjectCache::visitObjects(const IVisitor *visitor, const LineSegment& lineSegment) const
{
    grid->lineSegmentQuery(lineSegment, visitor);
}

GridObjectCache::~GridObjectCache()
{
    delete grid;
}

} /* namespace inet */
