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

#ifndef BVHTREE_H_
#define BVHTREE_H_

#include "INETDefs.h"
#include "IVisitor.h"
#include "LineSegment.h"
#include "PhysicalObject.h"

namespace inet {

/*
 * A bounding volume hierarchy (BVH) is a tree structure on a set of geometric objects.
 * All geometric objects are wrapped in bounding volumes that form the leaf nodes of the tree.
 * See: http://en.wikipedia.org/wiki/Bounding_volume_hierarchy
 * Implementation based on this sketch:
 * http://www.cs.utah.edu/~bes/papers/fastRT/paper-node8.html
 */
class BVHTree
{
    public:
        enum Axis
        {
            X,
            Y,
            Z
        };

    public:
      class BVHTreeVisitor : public IVisitor
      {
        public:
          virtual void visit(const cObject *) const = 0;
          virtual LineSegment getLineSegment() const = 0;
          virtual ~BVHTreeVisitor() {}
      };

    protected:
        struct AxisComparator
        {
            Axis axis;
            AxisComparator(Axis axis) : axis(axis) {}
            bool operator()(const PhysicalObject *left, const PhysicalObject *right) const
            {
                Coord leftPos = left->getPosition() + left->getShape()->computeSize() / 2;
                Coord rightPos = right->getPosition() + right->getShape()->computeSize() / 2;
                switch (axis)
                {
                    case X: return leftPos.x < rightPos.x;
                    case Y: return leftPos.y < rightPos.y;
                    case Z: return leftPos.z < rightPos.z;
                    default: throw cRuntimeError("Unknown axis");
                }
            }
        };
        typedef std::vector<const PhysicalObject *>::iterator ObjVecIterator;

    protected:
        Coord boundingMin, boundingMax;
        Coord center;
        BVHTree *left;
        BVHTree *right;
        const PhysicalObject *object;

    protected:
        Axis switchAxis(Axis axis) const;
        bool isLeaf() const;
        void buildHierarchy(std::vector<const PhysicalObject *>& objects, unsigned int start, unsigned int end, Axis axis);
        void computeBoundingBox(Coord& boundingMin, Coord& boundingMax, std::vector<const PhysicalObject *>& objects, unsigned int start, unsigned int end) const;
        bool intersectWithLineSegment(const LineSegment& lineSegment) const;

    public:
        BVHTree(const Coord& boundingMin, const Coord& boundingMax, std::vector<const PhysicalObject *>& objects, unsigned int start, unsigned int end, Axis axis);
        void traverse() const;
        virtual ~BVHTree();
        void lineSegmentQuery(const LineSegment& lineSegment,  const IVisitor *visitor) const;
};

} /* namespace inet */

#endif /* BVHTREE_H_ */
