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

SpatialGrid::SpatialGrid(Coord voxelSizes, Coord constraintAreaMin, Coord constraintAreaMax)
{
    this->voxelSizes = voxelSizes;
    this->constraintAreaMin = constraintAreaMin;
    this->constraintAreaMax = constraintAreaMax;
    constraintAreaSideLengths = calculateConstraintAreaSideLengths();
    numVoxels = calculateNumberOfVoxels();
    gridVectorLength = calculateGridVectorLength();
    if (gridVectorLength <= 0)
        throw cRuntimeError("Invalid gridVectorLength = %d", gridVectorLength);
    grid.resize(gridVectorLength);
}

bool SpatialGrid::insert(const cObject *point, Coord pos)
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

bool SpatialGrid::remove(const cObject *point)
{
    throw cRuntimeError("Unimplemented");
}

bool SpatialGrid::move(const cObject *point, Coord newPos)
{
    throw cRuntimeError("Unimplemented");
}

void SpatialGrid::rangeQuery(Coord pos, double range, const SpatialGridVisitor *visitor) const
{
    int xVoxel = constraintAreaSideLengths.x == 0 ? 0 : ceil((range * numVoxels[0]) / constraintAreaSideLengths.x);
    int yVoxel = constraintAreaSideLengths.y == 0 ? 0 : ceil((range * numVoxels[1]) / constraintAreaSideLengths.y);
    int zVoxel = constraintAreaSideLengths.z == 0 ? 0 : ceil((range * numVoxels[2]) / constraintAreaSideLengths.z);
    Integer3Tuple voxels(xVoxel, yVoxel, zVoxel);
    Integer3Tuple matrixIndices = coordToMatrixIndices(pos);
    Integer3Tuple start, end;
    for (unsigned int i = 0; i < 3; i++)
    {
        start[i] = matrixIndices[i] - voxels[i] < 0 ? 0 : matrixIndices[i] - voxels[i];
        int endCell = matrixIndices[i] + voxels[i] >= numVoxels[i] ? numVoxels[i] - 1 : matrixIndices[i] + voxels[i];
        if (endCell < 0)
            end[i] = 0;
        else
            end[i] = endCell;
    }
    for (int i = start[0]; i <= end[0]; i++) {
        for (int j = start[1]; j <= end[1]; j++) {
            for (int k = start[2]; k <= end[2]; k++) {
                int voxelIndex = rowMajorIndex(Integer3Tuple(i,j,k));
                const Voxel& neighborVoxel = grid[voxelIndex];
                for (Voxel::const_iterator it = neighborVoxel.begin(); it != neighborVoxel.end(); it++)
                    visitor->visitor(*it);
            }
        }
    }
}

void SpatialGrid::lineSegmentQuery(const LineSegment &lineSegment, const SpatialGridVisitor *visitor) const
{
    throw cRuntimeError("Unimplemented");
}

Coord SpatialGrid::calculateConstraintAreaSideLengths() const
{
    return Coord(constraintAreaMax.x - constraintAreaMin.x, constraintAreaMax.y - constraintAreaMin.y,
            constraintAreaMax.z - constraintAreaMin.z);
}

SpatialGrid::Integer3Tuple SpatialGrid::calculateNumberOfVoxels() const
{
    return Integer3Tuple(constraintAreaSideLengths.x / voxelSizes.x, constraintAreaSideLengths.y / voxelSizes.y,
            constraintAreaSideLengths.z / voxelSizes.z);
}

unsigned int SpatialGrid::calculateGridVectorLength() const
{
    unsigned int gridVectorLength = 1;
    for (unsigned int i = 0; i < 3; i++)
        if (numVoxels[i] != 0)
            gridVectorLength *= numVoxels[i];
    return gridVectorLength;
}

SpatialGrid::Integer3Tuple SpatialGrid::decodeRowMajorIndex(unsigned int ind) const
{
    Integer3Tuple indices;
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

unsigned int SpatialGrid::rowMajorIndex(const Integer3Tuple& indices) const
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

unsigned int SpatialGrid::coordToRowMajorIndex(Coord pos) const
{
    return rowMajorIndex(coordToMatrixIndices(pos));
}

void SpatialGrid::clearGrid()
{
    for (unsigned int i = 0; i < gridVectorLength; i++)
        grid[i].clear();
}

SpatialGrid::Integer3Tuple SpatialGrid::coordToMatrixIndices(Coord pos) const
{
    int xCoord = voxelSizes.x == 0 ? 0 : floor(pos.x / voxelSizes.x);
    int yCoord = voxelSizes.y == 0 ? 0 : floor(pos.y / voxelSizes.y);
    int zCoord = voxelSizes.z == 0 ? 0 : floor(pos.z / voxelSizes.z);
    return Integer3Tuple(xCoord, yCoord, zCoord);
}

} /* namespace inet */
