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

#ifndef SPATIALGRID_H_
#define SPATIALGRID_H_

#include "INETDefs.h"
#include "Coord.h"
#include "LineSegment.h"

namespace inet {

// This class implements a SpatialGrid data structure
// Spatial means three-dimensional grid, but it can be
// used with zero-length {x,y,z} sides, in this special
// cases, it operates in one- or two-dimensional mode.

// NOTE: With minimal effort, it can be extended to work
// in arbitrary dimension spaces.

class SpatialGrid
{
    public:
      class SpatialGridVisitor
      {
        public:
          virtual void visitor(const cObject *) const = 0;
          virtual ~SpatialGridVisitor() {}
      };

      template<typename T>
      class ThreeTuple
      {
          public:
              T x;
              T y;
              T z;
              ThreeTuple() : x(0), y(0), z(0) {}
              ThreeTuple(T xNum, T yNum, T zNum) :
                  x(xNum), y(yNum), z(zNum) {}
              T& operator[](int i)
              {
                switch (i)
                {
                    case 0: return x;
                    case 1: return y;
                    case 2: return z;
                    default:
                        throw cRuntimeError("Out of range with index: %d", i);
                }
              }
              const T& operator[](int i) const
              {
                switch (i)
                {
                    case 0: return x;
                    case 1: return y;
                    case 2: return z;
                    default:
                        throw cRuntimeError("Out of range with index: %d", i);
                }
              }
      };

    public:
        typedef std::list<const cObject *> Voxel;
        typedef std::vector<Voxel> Grid;

    protected:
        Grid grid;
        unsigned int gridVectorLength;
        Coord voxelSizes;
        Coord constraintAreaSideLengths;
        Coord constraintAreaMin, constraintAreaMax;
        ThreeTuple<int> numVoxels;

   protected:
        Coord calculateConstraintAreaSideLengths() const;
        ThreeTuple<int> calculateNumberOfVoxels() const;
        unsigned int calculateGridVectorLength() const;
        ThreeTuple<int> decodeRowMajorIndex(unsigned int ind) const;
        unsigned int rowMajorIndex(const ThreeTuple<int>& indices) const;
        unsigned int coordToRowMajorIndex(Coord pos) const;
        ThreeTuple<int> coordToMatrixIndices(Coord pos) const;

    public:
        bool insertPoint(const cObject *point, Coord pos);
        bool removePoint(const cObject *point);
        bool movePoint(const cObject *point, Coord newPos);
        void clearGrid();
        void rangeQuery(Coord pos, double range, const SpatialGridVisitor *visitor) const;
        void lineSegmentQuery(const LineSegment &lineSegment, const SpatialGridVisitor *visitor) const;
        SpatialGrid(Coord voxelSizes, Coord constraintAreaMin, Coord constraintAreaMax);
};

} /* namespace inet */

#endif /* SPATIALGRID_H_ */
