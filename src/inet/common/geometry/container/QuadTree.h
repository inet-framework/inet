//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_QUADTREE_H
#define __INET_QUADTREE_H

#include <algorithm>

#include "inet/common/IVisitor.h"
#include "inet/common/geometry/common/Coord.h"

namespace inet {

/*
 * It is a QuadTree implementation for efficient orthogonal range queries
 */
class INET_API QuadTree
{
  public:
    typedef std::vector<const cObject *> Points;

  protected:
    std::map<const cObject *, Coord> *lastPosition; // the point position when we inserted to the QuadTree

  protected:
    unsigned int quadrantCapacity; // maximum number of objects that can be contained in a quadrant
    Coord boundaryMin; // boundary coordinates: two opposite coordinates of a rectangle,
    Coord boundaryMax; // this uniquely determine a rectangle
    Points points; // the points or "objects" in the quadrant
    QuadTree *quadrants[4]; // a quadrant may be divided into four congruent subquadrant
    QuadTree *parent; // if we divide a quadrant then the divided quadrant will be the parent of it's four subquadrant

  protected:
    QuadTree *searchQuadrant(const Coord& lastPos);
    unsigned int whichQuadrant(const Coord& pos) const;
    bool hasChild() const;
    void setBoundary(Coord *minBoundaries, Coord *maxBoundaries) const;
    void splitPoints();
    void setToLeaf();
    bool isInRectangleRange(const Coord& pointCoord) const;
    bool doesIntersectWithQuadrant(const Coord& pos, double range) const;
    void tryToJoinChildQuadrants();

  public:
    bool move(const cObject *point, const Coord& newPos); // move an object to newPos
    bool remove(const cObject *point); // remove an object from the tree
    bool insert(const cObject *point, const Coord& pos); // insert an object with position pos
    void rangeQuery(const Coord& pos, double range, const IVisitor *visitor) const; // orthogonal range query from a point
    void strictRangeQuery(const Coord& pos, double range, const IVisitor *visitor) const; // query for points which lie in a circle with radius=range and center=pos
    QuadTree(const Coord& boundaryMin, const Coord& boundaryMax, unsigned int quadrantCapacity, QuadTree *parent);
    ~QuadTree();
};

} // namespace inet

#endif

