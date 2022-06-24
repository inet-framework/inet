//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BVHTREE_H
#define __INET_BVHTREE_H

#include "inet/common/IVisitor.h"
#include "inet/common/geometry/object/LineSegment.h"
#include "inet/environment/contract/IPhysicalObject.h"

namespace inet {

/*
 * A bounding volume hierarchy (BVH) is a tree structure on a set of geometric objects.
 * All geometric objects are wrapped in bounding volumes that form the leaf nodes of the tree.
 * See: http://en.wikipedia.org/wiki/Bounding_volume_hierarchy
 * Implementation based on this sketch:
 * http://www.cs.utah.edu/~bes/papers/fastRT/paper-node8.html
 */
class INET_API BvhTree
{
  public:
    class INET_API Axis {
      protected:
        std::string axisOrder;
        unsigned int curr;

      public:
        Axis(const std::string& axisOrder) : axisOrder(axisOrder), curr(0) {}
        char getNextAxis()
        {
            curr = (curr + 1) % axisOrder.size();
            return axisOrder[curr];
        }

        char getCurrentAxis() const { return axisOrder[curr]; }
    };

  public:
    class INET_API BvhTreeVisitor : public IVisitor {
      public:
        virtual void visit(const cObject *) const = 0;
        virtual LineSegment getLineSegment() const = 0;
        virtual ~BvhTreeVisitor() {}
    };

  protected:
    class INET_API AxisComparator {
      public:
        char axis;
        AxisComparator(char axis) : axis(axis) {}
        bool operator()(const physicalenvironment::IPhysicalObject *left, const physicalenvironment::IPhysicalObject *right) const
        {
            Coord leftPos = left->getPosition() + left->getShape()->computeBoundingBoxSize() / 2;
            Coord rightPos = right->getPosition() + right->getShape()->computeBoundingBoxSize() / 2;
            switch (axis) {
                case 'X': return leftPos.x < rightPos.x;
                case 'Y': return leftPos.y < rightPos.y;
                case 'Z': return leftPos.z < rightPos.z;
                default: throw cRuntimeError("Unknown axis");
            }
        }
    };

  protected:
    unsigned int leafCapacity;
    std::string axisOrder;
    Coord boundingMin, boundingMax;
    Coord center;
    BvhTree *left;
    BvhTree *right;
    std::vector<const physicalenvironment::IPhysicalObject *> objects;

  protected:
    bool isLeaf() const;
    void buildHierarchy(std::vector<const physicalenvironment::IPhysicalObject *>& objects, unsigned int start, unsigned int end, Axis axis);
    void computeBoundingBox(Coord& boundingMin, Coord& boundingMax, std::vector<const physicalenvironment::IPhysicalObject *>& objects, unsigned int start, unsigned int end) const;
    bool intersectWithLineSegment(const LineSegment& lineSegment) const;

  public:
    BvhTree(const Coord& boundingMin, const Coord& boundingMax, std::vector<const physicalenvironment::IPhysicalObject *>& objects, unsigned int start, unsigned int end, Axis axis, unsigned int leafCapacity);
    virtual ~BvhTree();
    void lineSegmentQuery(const LineSegment& lineSegment, const IVisitor *visitor) const;
};

} /* namespace inet */

#endif

