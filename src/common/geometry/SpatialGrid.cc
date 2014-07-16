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

#include "SpatialGrid.h"

namespace inet {

bool SpatialGrid::insertObject(const PhysicalObject *object)
{
    Coord pos = object->getPosition();
    Coord boundingBoxSize = object->getShape()->computeSize();
    ThreeTuple<int> start, end;
    calculateBoundingVoxels(pos, ThreeTuple<double>(boundingBoxSize.x, boundingBoxSize.y, boundingBoxSize.z), start, end);
    for (int i = start[0]; i <= end[0]; i++) {
        for (int j = start[1]; j <= end[1]; j++) {
            for (int k = start[2]; k <= end[2]; k++) {
                int voxelIndex = rowMajorIndex(ThreeTuple<int>(i,j,k));
                Voxel& neighborVoxel = grid[voxelIndex];
                neighborVoxel.push_back(check_and_cast<const cObject*>(object));
            }
        }
    }
    return true;
}

void SpatialGrid::calculateBoundingVoxels(const Coord& pos, const ThreeTuple<double>& boundings, ThreeTuple<int>& start, ThreeTuple<int>& end) const
{
    int xVoxel = constraintAreaSideLengths.x == 0 ? 0 : ceil((boundings.x * numVoxels[0]) / constraintAreaSideLengths.x);
    int yVoxel = constraintAreaSideLengths.y == 0 ? 0 : ceil((boundings.y * numVoxels[1]) / constraintAreaSideLengths.y);
    int zVoxel = constraintAreaSideLengths.z == 0 ? 0 : ceil((boundings.z * numVoxels[2]) / constraintAreaSideLengths.z);
    ThreeTuple<int> voxels(xVoxel, yVoxel, zVoxel);
    ThreeTuple<int> matrixIndices = coordToMatrixIndices(pos);
    for (unsigned int i = 0; i < 3; i++)
    {
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
    this->voxelSizes = ThreeTuple<double>(voxelSizes.x, voxelSizes.y, voxelSizes.z);
    this->constraintAreaMin = constraintAreaMin;
    this->constraintAreaMax = constraintAreaMax;
    constraintAreaSideLengths = calculateConstraintAreaSideLengths();
    numVoxels = calculateNumberOfVoxels();
    gridVectorLength = calculateGridVectorLength();
    if (gridVectorLength <= 0)
        throw cRuntimeError("Invalid gridVectorLength = %d", gridVectorLength);
    grid.resize(gridVectorLength);
}

bool SpatialGrid::insertPoint(const cObject *point, const Coord& pos)
{
    unsigned int ind = coordToRowMajorIndex(pos);
    if (ind >= gridVectorLength)
    {
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

void SpatialGrid::rangeQuery(const Coord& pos, double range, const SpatialGridVisitor *visitor) const
{
    ThreeTuple<int> start, end;
    calculateBoundingVoxels(pos, ThreeTuple<double>(range, range, range), start, end);
    for (int i = start[0]; i <= end[0]; i++) {
        for (int j = start[1]; j <= end[1]; j++) {
            for (int k = start[2]; k <= end[2]; k++) {
                int voxelIndex = rowMajorIndex(ThreeTuple<int>(i,j,k));
                const Voxel& neighborVoxel = grid[voxelIndex];
                for (Voxel::const_iterator it = neighborVoxel.begin(); it != neighborVoxel.end(); it++)
                    visitor->visit(*it);
            }
        }
    }
}

void SpatialGrid::lineSegmentQuery(const LineSegment &lineSegment, const SpatialGridVisitor *visitor) const
{
    for (LineSegmentIterator it(lineSegment, voxelSizes, numVoxels); !it.end(); ++it)
    {
        ThreeTuple<int> ind = it.getMatrixIndices();
        unsigned int voxelIndex = rowMajorIndex(ind);
        const Voxel& intersectedVoxel = grid[voxelIndex];
        for (Voxel::const_iterator it = intersectedVoxel.begin(); it != intersectedVoxel.end(); it++)
            visitor->visit(*it);
    }
}

Coord SpatialGrid::calculateConstraintAreaSideLengths() const
{
    return Coord(constraintAreaMax.x - constraintAreaMin.x, constraintAreaMax.y - constraintAreaMin.y,
            constraintAreaMax.z - constraintAreaMin.z);
}

SpatialGrid::ThreeTuple<int> SpatialGrid::calculateNumberOfVoxels() const
{
    return ThreeTuple<int>(ceil(constraintAreaSideLengths.x / voxelSizes.x), ceil(constraintAreaSideLengths.y / voxelSizes.y),
            ceil(constraintAreaSideLengths.z / voxelSizes.z));
}

unsigned int SpatialGrid::calculateGridVectorLength() const
{
    unsigned int gridVectorLength = 1;
    for (unsigned int i = 0; i < 3; i++)
        if (numVoxels[i] != 0)
            gridVectorLength *= numVoxels[i];
    return gridVectorLength;
}

SpatialGrid::ThreeTuple<int> SpatialGrid::decodeRowMajorIndex(unsigned int ind) const
{
    ThreeTuple<int> indices;
    for (unsigned int k = 0; k < 3; k++)
    {
        unsigned int prodDim = 1;
        for (unsigned int l = k + 1; l < 3; l++)
            if (numVoxels[l] > 0)
                prodDim *= numVoxels[l];
        indices[k] = ind / prodDim;
        ind %= prodDim;
    }
    return indices;
}

unsigned int SpatialGrid::rowMajorIndex(const ThreeTuple<int>& indices) const
{
    int ind = 0;
    for (unsigned int k = 0; k < 3; k++)
    {
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

SpatialGrid::ThreeTuple<int> SpatialGrid::coordToMatrixIndices(const Coord& pos) const
{
    int xCoord = voxelSizes.x == 0 ? 0 : floor(pos.x / voxelSizes.x);
    int yCoord = voxelSizes.y == 0 ? 0 : floor(pos.y / voxelSizes.y);
    int zCoord = voxelSizes.z == 0 ? 0 : floor(pos.z / voxelSizes.z);
    return ThreeTuple<int>(xCoord, yCoord, zCoord);
}


SpatialGrid::LineSegmentIterator::LineSegmentIterator(const LineSegment &lineSegment, const ThreeTuple<double> &voxelSizes, const ThreeTuple<int>& numVoxels)
{
    reachedEnd = false;
    Coord p0 = lineSegment.getPoint1();
    Coord p1 = lineSegment.getPoint2();
    Coord segmentDirection = p1 - p0;
    ThreeTuple<double> point0 = ThreeTuple<double>(p0.x, p0.y, p0.z);
    ThreeTuple<double> point1 = ThreeTuple<double>(p1.x, p1.y, p1.z);
    ThreeTuple<double> direction = ThreeTuple<double>(segmentDirection.x, segmentDirection.y, segmentDirection.z);
    for (int i = 0; i < 3; i++)
    {
        index[i] = voxelSizes[i] == 0 ? 0 : floor(point0[i] / voxelSizes[i]);
        endPoint[i] = voxelSizes[i] == 0 ? 0 : floor(point1[i] / voxelSizes[i]);
        tDelta[i] = voxelSizes[i] / std::abs(direction[i]);
        double ithdirection = direction[i];
        if (ithdirection > 0)
            step[i] = 1;
        else if (ithdirection < 0)
            step[i] = -1;
        else
            step[i] = 0;
        double d = point0[i] - index[i] * voxelSizes[i];
        if (step[i] > 0)
        {
            d = voxelSizes[i] - d;
        }
        ASSERT(d >= 0 && d <= voxelSizes[i]);
        if (ithdirection != 0)
            tExit[i] = d / std::abs(ithdirection);
        else
            tExit[i] = std::numeric_limits<double>::max();
    }
}

SpatialGrid::LineSegmentIterator& SpatialGrid::LineSegmentIterator::operator++()
{
    if (index.x != endPoint.x || index.y != endPoint.y || index.z != endPoint.z)
    {
        int axis = 0;
        if (tExit.x < tExit.y)
        {
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
