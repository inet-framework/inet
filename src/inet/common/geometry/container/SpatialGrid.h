//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SPATIALGRID_H
#define __INET_SPATIALGRID_H

#include "inet/common/IVisitor.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/object/LineSegment.h"

namespace inet {

/**
 * This class implements a spatial grid data structure using a 3 dimensional grid.
 * It can also be used with zero-length {X, Y, Z} sides, and then it operates in
 * 1 or 2 dimensional mode.
 *
 * NOTE: With minimal effort, it can be extended to work in arbitrary dimension spaces.
 */
class INET_API SpatialGrid
{
  public:
    template<typename T>
    class INET_API Triplet {
      public:
        T x;
        T y;
        T z;
        Triplet() : x(0), y(0), z(0) {}
        Triplet(T xNum, T yNum, T zNum) :
            x(xNum), y(yNum), z(zNum) {}
        T& operator[](int i)
        {
            switch (i) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                default:
                    throw cRuntimeError("Out of range with index: %d", i);
            }
        }

        const T& operator[](int i) const
        {
            switch (i) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                default:
                    throw cRuntimeError("Out of range with index: %d", i);
            }
        }
    };

  protected:
    // This class implements a LineSegmentIterator based on the G3D library's RayGridIterator class
    // http://graphics.cs.williams.edu/courses/cs371/f10/G3D/manual/class_g3_d_1_1_ray_grid_iterator.html
    class INET_API LineSegmentIterator {
      protected:
        Triplet<int> index;
        Triplet<int> endPoint;
        Triplet<int> step;
        Triplet<double> tDelta;
        Triplet<double> tExit;
        bool reachedEnd;

      public:
        LineSegmentIterator(const SpatialGrid *spatialGrid, const LineSegment& lineSegment, const Triplet<double>& voxelSizes, const Triplet<int>& numVoxels);
        LineSegmentIterator& operator++();
        const Triplet<int>& getMatrixIndices() const { return index; }
        bool end() const { return reachedEnd; }
    };

  public:
    typedef std::list<const cObject *> Voxel;
    typedef std::vector<Voxel> Grid;

  protected:
    Grid grid;
    unsigned int gridVectorLength;
    Triplet<double> voxelSizes;
    Coord constraintAreaSideLengths;
    Coord constraintAreaMin, constraintAreaMax;
    Triplet<int> numVoxels;

  protected:
    Coord computeConstraintAreaSideLengths() const;
    Triplet<int> computeNumberOfVoxels() const;
    unsigned int computeGridVectorLength() const;
    Triplet<int> decodeRowMajorIndex(unsigned int ind) const;
    unsigned int rowMajorIndex(const Triplet<int>& indices) const;
    unsigned int coordToRowMajorIndex(const Coord& pos) const;
    Triplet<int> coordToMatrixIndices(const Coord& pos) const;
    void computeBoundingVoxels(const Coord& pos, const Triplet<double>& boundings, Triplet<int>& start, Triplet<int>& end) const;

  public:
    bool insertObject(const cObject *object, const Coord& pos, const Coord& boundingBoxSize);
    bool insertPoint(const cObject *point, const Coord& pos);
    bool removePoint(const cObject *point);
    bool movePoint(const cObject *point, const Coord& newPos);
    void clearGrid();
    void rangeQuery(const Coord& pos, double range, const IVisitor *visitor) const;
    void lineSegmentQuery(const LineSegment& lineSegment, const IVisitor *visitor) const;
    SpatialGrid(const Coord& voxelSizes, const Coord& constraintAreaMin, const Coord& constraintAreaMax);
};

} /* namespace inet */

#endif

