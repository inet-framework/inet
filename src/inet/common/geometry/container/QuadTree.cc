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

#include "inet/common/geometry/container/QuadTree.h"

namespace inet {

bool QuadTree::insert(const cObject *point, const Coord& pos)
{
    if (!isInRectangleRange(pos))
        return false;
    if (points.size() < quadrantCapacity && !hasChild()) {
        (*lastPosition)[point] = pos;
        points.push_back(point);
        return true;
    }
    // If the rootNode is a leaf with a point set
    // with no free capacity, then we must split its
    // points into new quadrants
    if (!hasChild())
        splitPoints();
    for (unsigned int i = 0; i < 4; i++)
        if (quadrants[i]->insert(point, pos))
            return true;
    throw cRuntimeError("QuadTree insertion failed for object: %s with position: (%f, %f, %f)", point->getFullName(), pos.x, pos.y, pos.z);
    return false;
}

unsigned int QuadTree::whichQuadrant(const Coord& pos) const
{
    for (unsigned int i = 0; i < 4; i++)
        if (quadrants[i]->isInRectangleRange(pos))
            return i;
    throw cRuntimeError("QuadTree failed to determine to which quadrant point (%f, %f, %f) belongs to", pos.x, pos.y, pos.z);
    return 19920213;
}

void QuadTree::setBoundary(Coord *minBoundaries, Coord *maxBoundaries) const
{
    // We just divide a rectangle into four smaller congruent rectangle
    maxBoundaries[0].x = (boundaryMax.x - boundaryMin.x) / 2 + boundaryMin.x;
    minBoundaries[0].y = (boundaryMax.y - boundaryMin.y) / 2 + boundaryMin.y;
    minBoundaries[0].x = boundaryMin.x;
    maxBoundaries[0].y = boundaryMax.y;

    minBoundaries[1].x = (boundaryMax.x - boundaryMin.x) / 2 + boundaryMin.x;
    minBoundaries[1].y = (boundaryMax.y - boundaryMin.y) / 2 + boundaryMin.y;
    maxBoundaries[1].x = boundaryMax.x;
    maxBoundaries[1].y = boundaryMax.y;

    maxBoundaries[2].y = (boundaryMax.y - boundaryMin.y) / 2 + boundaryMin.y;
    maxBoundaries[2].x = (boundaryMax.x - boundaryMin.x) / 2 + boundaryMin.x;
    minBoundaries[2].x = boundaryMin.x;
    minBoundaries[2].y = boundaryMin.y;

    minBoundaries[3].x = (boundaryMax.x - boundaryMin.x) / 2 + boundaryMin.x;
    maxBoundaries[3].y = (boundaryMax.y - boundaryMin.y) / 2 + boundaryMin.y;
    maxBoundaries[3].x = boundaryMax.x;
    minBoundaries[3].y = boundaryMin.y;
}

void QuadTree::splitPoints()
{
    Coord minBoundaries[4], maxBoundaries[4];
    setBoundary(minBoundaries, maxBoundaries);
    // We make four new quadrants
    for (unsigned int i = 0; i < 4; i++)
        quadrants[i] = new QuadTree(minBoundaries[i], maxBoundaries[i], quadrantCapacity, this);
    // The node is not a leaf anymore
    // so we have to split its point
    for (unsigned int i = 0; i < points.size(); i++) {
        std::map<const cObject *, Coord>::iterator it = lastPosition->find(points[i]);
        Coord pos;
        if (it != lastPosition->end())
            pos = it->second;
        else
            throw cRuntimeError("Last position not found for object: %s", points[i]->getFullName());
        unsigned int quadrantNum = whichQuadrant(pos);
        // We recursively call insert() for each points
        quadrants[quadrantNum]->insert(points[i], pos);
    }
    // Now we can free the node's vector
    points.clear();
}

void QuadTree::setToLeaf()
{
    for (unsigned int i = 0; i < 4; i++)
        quadrants[i] = NULL;
}

void QuadTree::rangeQuery(const Coord& pos, double range, const IVisitor *visitor) const
{
    // If our rectangle intersects with a quadrant then we insert its objects to the
    // neighbors vector
    // Note that, a node have points only if it is a leaf node
    if (!hasChild() && doesIntersectWithQuadrant(pos, range))
        for (unsigned int i = 0; i < points.size(); i++)
            visitor->visit(points[i]);
    else if (hasChild())
        for (unsigned int i = 0; i < 4; i++)
            quadrants[i]->rangeQuery(pos, range, visitor);
}

void QuadTree::strictRangeQuery(const Coord& pos, double range, const IVisitor *visitor) const
{
    if (!hasChild() && doesIntersectWithQuadrant(pos, range)) {
        for (unsigned int i = 0; i < points.size(); i++) {
            Coord otherPos = (*lastPosition)[points[i]];
            if (pos.sqrdist(otherPos) <= range * range)
                visitor->visit(points[i]);
        }
    }
    else if (hasChild()) {
        for (unsigned int i = 0; i < 4; i++)
            quadrants[i]->strictRangeQuery(pos, range, visitor);
    }
}

bool QuadTree::isInRectangleRange(const Coord& pointCoord) const
{
    return  pointCoord.x <= boundaryMax.x && pointCoord.x >= boundaryMin.x &&
            pointCoord.y <= boundaryMax.y && pointCoord.y >= boundaryMin.y;
}

bool QuadTree::doesIntersectWithQuadrant(const Coord& pos, double range) const
{
    Coord minRectangleBoundary = pos;
    Coord maxRectangleBoundary = pos;
    minRectangleBoundary.x -= range;
    minRectangleBoundary.y -= range;
    maxRectangleBoundary.x += range;
    maxRectangleBoundary.y += range;
    return !((minRectangleBoundary.x > boundaryMax.x) || (maxRectangleBoundary.x < boundaryMin.x) ||
             (minRectangleBoundary.y > boundaryMax.y) || (maxRectangleBoundary.y < boundaryMin.y));
}

bool QuadTree::remove(const cObject *point)
{
    // We search for the quadrant that may contain our object
    // Note that, we need the last position of the object, that is, the position when we
    // inserted it into the QuadTree
    // This helps to searchRadioQuadrant(), since we don't have to traverse
    // the whole QuadTree and check each node's vector one by one.
    Coord lastPos;
    std::map<const cObject *, Coord>::iterator lastIt = lastPosition->find(point);
    if (lastIt != lastPosition->end())
        lastPos = lastIt->second;
    else
        return false;
    QuadTree *quadrant = searchQuadrant(lastPos);
    if (quadrant == NULL)
        throw cRuntimeError("Quadrant not found for point: (%f, %f, %f)", lastPos.x, lastPos.y, lastPos.z);
    Points::iterator it = find(quadrant->points.begin(), quadrant->points.end(), point);
    // If we find the object then we erase it from the quadrant's vector and lastPosition map
    if (it != quadrant->points.end()) {
        lastPosition->erase(lastIt);
        quadrant->points.erase(it);
    }
    else
        throw cRuntimeError("Point (%f, %f, %f) not found in its quadrant's vector", lastPos.x, lastPos.y, lastPos.z);
    quadrant->parent->tryToJoinChildQuadrants();
    return true;
}

QuadTree *QuadTree::searchQuadrant(const Coord& lastPos)
{
    // If lastPos is in the quadrant and that quadrant has no child,
    // then we found the quadrant which _may_ contain our object.
    // Note that, this can not guarantee that the object is in the quadrant's
    // vector, so you must check it yourself!
    if (!hasChild() && isInRectangleRange(lastPos))
        return this;
    else if (hasChild()) {
        for (int i = 0; i < 4; i++)
            if (quadrants[i]->isInRectangleRange(lastPos))
                return quadrants[i]->searchQuadrant(lastPos);
        return NULL;
    }
    else
        return NULL;
}

bool QuadTree::hasChild() const
{
    return quadrants[0] != NULL;
}

void QuadTree::tryToJoinChildQuadrants()
{
    unsigned int quadrantSum = 0;

    for (unsigned int i = 0; i < 4; i++) {
        // We surely can't join quadrants if one quadrant has another
        // subquadrants
        if (quadrants[i]->hasChild())
            return;
        quadrantSum += quadrants[i]->points.size();
    }
    // If the child quadrants together contain no more
    // than quadrantCapacity objects then we can
    // join these quadrants
    if (quadrantSum <= quadrantCapacity) {
        // Copy the points to the parent node
        for (unsigned int i = 0; i < 4; i++) {
            QuadTree *quadrant = quadrants[i];
            for (unsigned int j = 0; j < quadrant->points.size(); j++)
                points.push_back(quadrant->points[j]);
            // Delete the child quadrants
            delete quadrant;
        }
        // Then set to leaf
        setToLeaf();
        // Finally, we call it for the parent node
        if (parent)
            parent->tryToJoinChildQuadrants();
    }
}

bool QuadTree::move(const cObject *point, const Coord& newPos)
{
    QuadTree *quadrant = searchQuadrant(newPos);
    // It is an error! Our QuadTree must find an appropriate quadrant since the root node
    // boundary coordinates equal to the constraint area coordinates.
    if (quadrant == NULL)
        throw cRuntimeError("Quadrant not found for point (%f %f %f)", newPos.x, newPos.y, newPos.z);
    Points::iterator it = find(quadrant->points.begin(), quadrant->points.end(), point);
    // If we search for a quadrant with the object's current position and then we find
    // it in the quadrant's vector, then the move occurred inside this quadrant,
    // thus we have nothing to do with this case
    if (it != quadrant->points.end())
        return true;
    else // Otherwise, we remove the object and insert it again
        return remove(point) && insert(point, newPos);
}

QuadTree::QuadTree(const Coord& boundaryMin, const Coord& boundaryMax, unsigned int quadrantCapacity, QuadTree *parent)
{
    this->boundaryMax = boundaryMax;
    this->boundaryMin = boundaryMin;
    this->quadrantCapacity = quadrantCapacity;
    this->parent = parent;
    setToLeaf();
    // lastPosition containing information for all subtrees in a QuadTree
    // so we only create it when we create a root (indicated by parent == NULL)
    // node. Each subtree inherits this pointer and use its global
    // information
    if (parent == NULL)
        lastPosition = new std::map<const cObject *, Coord>;
    else
        lastPosition = parent->lastPosition;
}

QuadTree::~QuadTree()
{
    for (unsigned int i = 0; i < 4; i++)
        delete quadrants[i];
    // We clear lastPosition if and only if we delete the whole tree
    // Take a look at the constructor to see why we do this!
    if (parent == NULL)
        delete lastPosition;
}

} // namespace inet

