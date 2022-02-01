//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/geometry/container/SpatialGrid.h"

#include "inet/common/stlutils.h"

namespace inet {

bool SpatialGrid::insertObject(const cObject *object, const Coord& pos, const Coord& boundingBoxSize)
{
    Triplet<int> start, end;
    computeBoundingVoxels(pos, Triplet<double>(boundingBoxSize.x / 2, boundingBoxSize.y / 2, boundingBoxSize.z / 2), start, end);
    for (int i = start[0]; i <= end[0]; i++) {
        for (int j = start[1]; j <= end[1]; j++) {
            for (int k = start[2]; k <= end[2]; k++) {
                int voxelIndex = rowMajorIndex(Triplet<int>(i, j, k));
                Voxel& neighborVoxel = grid[voxelIndex];
                neighborVoxel.push_back(check_and_cast<const cObject *>(object));
            }
        }
    }
    return true;
}

void SpatialGrid::computeBoundingVoxels(const Coord& pos, const Triplet<double>& boundings, Triplet<int>& start, Triplet<int>& end) const
{
    int xVoxel = constraintAreaSideLengths.x == 0 ? 0 : ceil((boundings.x * numVoxels[0]) / constraintAreaSideLengths.x);
    int yVoxel = constraintAreaSideLengths.y == 0 ? 0 : ceil((boundings.y * numVoxels[1]) / constraintAreaSideLengths.y);
    int zVoxel = constraintAreaSideLengths.z == 0 ? 0 : ceil((boundings.z * numVoxels[2]) / constraintAreaSideLengths.z);
    Triplet<int> voxels(xVoxel, yVoxel, zVoxel);
    Triplet<int> matrixIndices = coordToMatrixIndices(pos);
    for (unsigned int i = 0; i < 3; i++) {
        start[i] = matrixIndices[i] - voxels[i] < 0 ? 0 : matrixIndices[i] - voxels[i];
        int endCell = matrixIndices[i] + voxels[i] >= numVoxels[i] ? numVoxels[i] - 1 : matrixIndices[i] + voxels[i];
        if (endCell < 0)
            end[i] = 0;
        else
            end[i] = endCell;
    }
}

SpatialGrid::SpatialGrid(const Coord& voxelSizes, const Coord& constraintAreaMin, const Coord& constraintAreaMax)
{
    this->voxelSizes = Triplet<double>(voxelSizes.x, voxelSizes.y, voxelSizes.z);
    this->constraintAreaMin = constraintAreaMin;
    this->constraintAreaMax = constraintAreaMax;
    constraintAreaSideLengths = computeConstraintAreaSideLengths();
    numVoxels = computeNumberOfVoxels();
    gridVectorLength = computeGridVectorLength();
    if (gridVectorLength <= 0)
        throw cRuntimeError("Invalid gridVectorLength = %d", gridVectorLength);
    grid.resize(gridVectorLength);
}

bool SpatialGrid::insertPoint(const cObject *point, const Coord& pos)
{
    unsigned int ind = coordToRowMajorIndex(pos);
    if (ind >= gridVectorLength) {
        throw cRuntimeError("Out of range with index: %d", ind);
        return false;
    }
    grid[ind].push_back(point);
    return true;
}

bool SpatialGrid::removePoint(const cObject *point)
{
    throw cRuntimeError("Unimplemented");
}

bool SpatialGrid::movePoint(const cObject *point, const Coord& newPos)
{
    throw cRuntimeError("Unimplemented");
}

void SpatialGrid::rangeQuery(const Coord& pos, double range, const IVisitor *visitor) const
{
    Triplet<int> start, end;
    computeBoundingVoxels(pos, Triplet<double>(range, range, range), start, end);
    for (int i = start[0]; i <= end[0]; i++) {
        for (int j = start[1]; j <= end[1]; j++) {
            for (int k = start[2]; k <= end[2]; k++) {
                int voxelIndex = rowMajorIndex(Triplet<int>(i, j, k));
                const Voxel& neighborVoxel = grid[voxelIndex];
                for (const auto& elem : neighborVoxel)
                    visitor->visit(elem);
            }
        }
    }
}

void SpatialGrid::lineSegmentQuery(const LineSegment& lineSegment, const IVisitor *visitor) const
{
    std::set<const cObject *> visited;
    for (LineSegmentIterator it(this, lineSegment, voxelSizes, numVoxels); !it.end(); ++it) {
        Triplet<int> ind = it.getMatrixIndices();
        unsigned int voxelIndex = rowMajorIndex(ind);
        if (voxelIndex >= gridVectorLength)
            throw cRuntimeError("Out of index, gridVectorLength = %d, voxelIndex = %d", gridVectorLength, voxelIndex);
        const Voxel& intersectedVoxel = grid[voxelIndex];
        for (const auto& elem : intersectedVoxel) {
            if (!contains(visited, elem)) {
                visitor->visit(elem);
                visited.insert(elem);
            }
        }
    }
}

Coord SpatialGrid::computeConstraintAreaSideLengths() const
{
    return Coord(constraintAreaMax.x - constraintAreaMin.x, constraintAreaMax.y - constraintAreaMin.y,
            constraintAreaMax.z - constraintAreaMin.z);
}

SpatialGrid::Triplet<int> SpatialGrid::computeNumberOfVoxels() const
{
    return Triplet<int>(
            constraintAreaSideLengths.x == 0 ? 0 : ceil(constraintAreaSideLengths.x / voxelSizes.x),
            constraintAreaSideLengths.y == 0 ? 0 : ceil(constraintAreaSideLengths.y / voxelSizes.y),
            constraintAreaSideLengths.z == 0 ? 0 : ceil(constraintAreaSideLengths.z / voxelSizes.z));
}

unsigned int SpatialGrid::computeGridVectorLength() const
{
    unsigned int gridVectorLength = 1;
    for (unsigned int i = 0; i < 3; i++)
        if (numVoxels[i] != 0)
            gridVectorLength *= numVoxels[i];
    return gridVectorLength;
}

SpatialGrid::Triplet<int> SpatialGrid::decodeRowMajorIndex(unsigned int ind) const
{
    Triplet<int> indices;
    for (unsigned int k = 0; k < 3; k++) {
        unsigned int prodDim = 1;
        for (unsigned int l = k + 1; l < 3; l++)
            if (numVoxels[l] > 0)
                prodDim *= numVoxels[l];
        indices[k] = ind / prodDim;
        ind %= prodDim;
    }
    return indices;
}

unsigned int SpatialGrid::rowMajorIndex(const Triplet<int>& indices) const
{
    int ind = 0;
    for (unsigned int k = 0; k < 3; k++) {
        unsigned int prodDim = 1;
        for (unsigned int l = k + 1; l < 3; l++)
            if (numVoxels[l] > 0)
                prodDim *= numVoxels[l];
        ind += prodDim * indices[k];
    }
    return ind;
}

unsigned int SpatialGrid::coordToRowMajorIndex(const Coord& pos) const
{
    return rowMajorIndex(coordToMatrixIndices(pos));
}

void SpatialGrid::clearGrid()
{
    for (unsigned int i = 0; i < gridVectorLength; i++)
        grid[i].clear();
}

SpatialGrid::Triplet<int> SpatialGrid::coordToMatrixIndices(const Coord& pos) const
{
    int xCoord = numVoxels[0] == 0 ? 0 : std::min((int)floor((pos.x - constraintAreaMin.x) / voxelSizes.x), numVoxels[0] - 1);
    int yCoord = numVoxels[1] == 0 ? 0 : std::min((int)floor((pos.y - constraintAreaMin.y) / voxelSizes.y), numVoxels[1] - 1);
    int zCoord = numVoxels[2] == 0 ? 0 : std::min((int)floor((pos.z - constraintAreaMin.z) / voxelSizes.z), numVoxels[2] - 1);
    return Triplet<int>(xCoord, yCoord, zCoord);
}

SpatialGrid::LineSegmentIterator::LineSegmentIterator(const SpatialGrid *spatialGrid, const LineSegment& lineSegment, const Triplet<double>& voxelSizes, const Triplet<int>& numVoxels)
{
    reachedEnd = false;
    Coord p0 = lineSegment.getPoint1();
    Coord p1 = lineSegment.getPoint2();
    Coord segmentDirection = p1 - p0;
    Triplet<double> point0 = Triplet<double>(p0.x - spatialGrid->constraintAreaMin.x, p0.y - spatialGrid->constraintAreaMin.y, p0.z - spatialGrid->constraintAreaMin.z);
    Triplet<double> direction = Triplet<double>(segmentDirection.x, segmentDirection.y, segmentDirection.z);
    index = spatialGrid->coordToMatrixIndices(p0);
    endPoint = spatialGrid->coordToMatrixIndices(p1);
    for (int i = 0; i < 3; i++) {
        tDelta[i] = voxelSizes[i] / std::abs(direction[i]);
        double ithdirection = direction[i];
        if (ithdirection > 0)
            step[i] = 1;
        else if (ithdirection < 0)
            step[i] = -1;
        else
            step[i] = 0;
        if (ithdirection != 0) {
            double d = point0[i] - index[i] * voxelSizes[i];
            if (step[i] > 0)
                d = voxelSizes[i] - d;
            ASSERT(d >= 0 && d <= voxelSizes[i]);
            tExit[i] = d / std::abs(ithdirection);
        }
        else
            tExit[i] = std::numeric_limits<double>::max();
    }
}

SpatialGrid::LineSegmentIterator& SpatialGrid::LineSegmentIterator::operator++()
{
    if (index.x != endPoint.x || index.y != endPoint.y || index.z != endPoint.z) {
        int axis = 0;
        if (tExit.x < tExit.y) {
            if (tExit.x < tExit.z)
                axis = 0;
            else
                axis = 2;
        }
        else if (tExit.y < tExit.z)
            axis = 1;
        else
            axis = 2;
        index[axis] += step[axis];
        tExit[axis] += tDelta[axis];
    }
    else
        reachedEnd = true;
    return *this;
}

} /* namespace inet */

