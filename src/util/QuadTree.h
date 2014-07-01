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

#ifndef __INET_QUADTREE_H
#define __INET_QUADTREE_H

#include "INETDefs.h"
#include "Coord.h"
#include <algorithm>

namespace inet {

//
// It is a QuadTree implementation for efficient orthogonal range queries
//
class INET_API QuadTree
{
  public:
    class QuadTreeVisitor
    {
      public:
        // Must be implemented whenever you use QuadTree for your own purposes
        // Invoked from rangeQuery() and strictRangeQuery()
        virtual void visitor(const cObject *) = 0;
        virtual ~QuadTreeVisitor() {}
    };

  public:
    typedef std::vector<const cObject *> Points;

  protected:
    std::map<const cObject *, Coord> *lastPosition;    // the point position when we inserted to the QuadTree

  protected:
    unsigned int quadrantCapacity;    // maximum number of objects that can be contained in a quadrant
    Coord boundaryMin;    // boundary coordinates: two opposite coordinates of a rectangle,
    Coord boundaryMax;    // this uniquely determine a rectangle
    Points points;    // the points or "objects" in the quadrant
    QuadTree *quadrants[4];    // a quadrant may be divided into four congruent subquadrant
    QuadTree *parent;    // if we divide a quadrant then the divided quadrant will be the parent of it's four subquadrant

  protected:
    QuadTree *searchQuadrant(Coord lastPos);
    unsigned int whichQuadrant(Coord pos) const;
    bool hasChild() const;
    void setBoundary(Coord *minBoundaries, Coord *maxBoundaries);
    void splitPoints();
    void setToLeaf();
    bool isInRectangleRange(Coord pointCoord) const;
    bool doesIntersectWithQuadrant(Coord pos, double range) const;
    void tryToJoinChildQuadrants();

  public:
    bool move(const cObject *point, Coord newPos);    // move an object to newPos
    bool remove(const cObject *point);    // remove an object from the tree
    bool insert(const cObject *point, Coord pos);    // insert an object with position pos
    void rangeQuery(Coord pos, double range, QuadTreeVisitor *visitor);
    void strictRangeQuery(Coord pos, double range, QuadTreeVisitor *visitor);
    QuadTree(Coord boundaryMin, Coord boundaryMax, unsigned int quadrantCapacity, QuadTree *parent);
    ~QuadTree();
};

} // namespace inet

#endif /* QUADTREE_H_ */

