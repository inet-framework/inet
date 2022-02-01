//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/environment/objectcache/GridObjectCache.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace physicalenvironment {

Define_Module(GridObjectCache);

GridObjectCache::GridObjectCache() :
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
        physicalEnvironment.reference(this, "physicalEnvironmentModule", true);
    else if (stage == INITSTAGE_PHYSICAL_OBJECT_CACHE) {
        double cellSizeX = par("cellSizeX");
        double cellSizeY = par("cellSizeY");
        double cellSizeZ = par("cellSizeZ");
        const Coord& spaceMin = physicalEnvironment->getSpaceMin();
        const Coord& spaceMax = physicalEnvironment->getSpaceMax();
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
    // TODO avoid dynamic cast
    grid->insertObject(dynamic_cast<const cObject *>(object), pos, boundingBoxSize);
    return true;
}

void GridObjectCache::visitObjects(const IVisitor *visitor, const LineSegment& lineSegment) const
{
    grid->lineSegmentQuery(lineSegment, visitor);
}

} // namespace physicalenvironment

} // namespace inet

