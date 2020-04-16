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
#include "inet/environment/objectcache/GridObjectCache.h"

namespace inet {

namespace physicalenvironment {

Define_Module(GridObjectCache);

GridObjectCache::GridObjectCache() :
    physicalEnvironment(nullptr),
    grid(nullptr)
{
}

GridObjectCache::~GridObjectCache()
{
    delete grid;
}

void GridObjectCache::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        physicalEnvironment = getModuleFromPar<PhysicalEnvironment>(par("physicalEnvironmentModule"), this);
    else if (stage == INITSTAGE_PHYSICAL_OBJECT_CACHE) {
        double cellSizeX = par("cellSizeX");
        double cellSizeY = par("cellSizeY");
        double cellSizeZ = par("cellSizeZ");
        const Coord spaceMin = physicalEnvironment->getSpaceMin();
        const Coord spaceMax = physicalEnvironment->getSpaceMax();
        const Coord spaceSize = spaceMax - spaceMin;
        if (std::isnan(cellSizeX))
            cellSizeX = spaceSize.x / par("cellCountX").intValue();
        if (std::isnan(cellSizeY))
            cellSizeY = spaceSize.y / par("cellCountY").intValue();
        if (std::isnan(cellSizeZ))
            cellSizeZ = spaceSize.z / par("cellCountZ").intValue();
        Coord voxelSizes(cellSizeX, cellSizeY, cellSizeZ);
        grid = new SpatialGrid(voxelSizes, spaceMin, spaceMax);
        for (int i = 0; i < physicalEnvironment->getNumObjects(); i++)
            insertObject(physicalEnvironment->getObject(i));
    }
}

bool GridObjectCache::insertObject(const IPhysicalObject *object)
{
    Coord pos = object->getPosition();
    Coord boundingBoxSize = object->getShape()->computeBoundingBoxSize();
    // TODO: avoid dynamic cast
    grid->insertObject(dynamic_cast<const cObject *>(object), pos, boundingBoxSize);
    return true;
}

void GridObjectCache::visitObjects(const IVisitor *visitor, const LineSegment& lineSegment) const
{
    grid->lineSegmentQuery(lineSegment, visitor);
}

} // namespace physicalenvironment

} // namespace inet

