//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/geometry/container/QuadTree.h"

#include "inet/common/stlutils.h"

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
    for (auto& elem : quadrants)
        if (elem->insert(point, pos))
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
    for (auto& elem : points) {
        auto it = lastPosition->find(elem);
        if (it == lastPosition->end())
            throw cRuntimeError("Last position not found for object: %s", elem->getFullName());
        Coord pos = it->second;
        unsigned int quadrantNum = whichQuadrant(pos);
        // We recursively call insert() for each points
        quadrants[quadrantNum]->insert(elem, pos);
    }
    // Now we can free the node's vector
    points.clear();
}

void QuadTree::setToLeaf()
{
    for (auto& elem : quadrants)
        elem = nullptr;
}

void QuadTree::rangeQuery(const Coord& pos, double range, const IVisitor *visitor) const
{
    // If our rectangle intersects with a quadrant then we insert its objects to the
    // neighbors vector
    // Note that, a node have points only if it is a leaf node
    if (!hasChild() && doesIntersectWithQuadrant(pos, range))
        for (auto& elem : points)
            visitor->visit(elem);
    else if (hasChild())
        for (auto& elem : quadrants)
            elem->rangeQuery(pos, range, visitor);
}

void QuadTree::strictRangeQuery(const Coord& pos, double range, const IVisitor *visitor) const
{
    if (!hasChild() && doesIntersectWithQuadrant(pos, range)) {
        for (auto& elem : points) {
            Coord otherPos = (*lastPosition)[elem];
            if (pos.sqrdist(otherPos) <= range * range)
                visitor->visit(elem);
        }
    }
    else if (hasChild()) {
        for (auto& elem : quadrants)
            elem->strictRangeQuery(pos, range, visitor);
    }
}

bool QuadTree::isInRectangleRange(const Coord& pointCoord) const
{
    return pointCoord.x <= boundaryMax.x && pointCoord.x >= boundaryMin.x &&
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
    auto lastIt = lastPosition->find(point);
    if (lastIt == lastPosition->end())
        return false;
    lastPos = lastIt->second;
    QuadTree *quadrant = searchQuadrant(lastPos);
    if (quadrant == nullptr)
        throw cRuntimeError("Quadrant not found for point: (%f, %f, %f)", lastPos.x, lastPos.y, lastPos.z);
    auto it = find(quadrant->points, point);
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
        for (auto& elem : quadrants)
            if (elem->isInRectangleRange(lastPos))
                return elem->searchQuadrant(lastPos);
        return nullptr;
    }
    else
        return nullptr;
}

bool QuadTree::hasChild() const
{
    return quadrants[0] != nullptr;
}

void QuadTree::tryToJoinChildQuadrants()
{
    unsigned int quadrantSum = 0;

    for (auto& elem : quadrants) {
        // We surely can't join quadrants if one quadrant has another
        // subquadrants
        if (elem->hasChild())
            return;
        quadrantSum += elem->points.size();
    }
    // If the child quadrants together contain no more
    // than quadrantCapacity objects then we can
    // join these quadrants
    if (quadrantSum <= quadrantCapacity) {
        // Copy the points to the parent node
        for (auto quadrant : quadrants) {

            for (auto& elem : quadrant->points)
                points.push_back(elem);
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
    if (quadrant == nullptr)
        throw cRuntimeError("Quadrant not found for point (%f %f %f)", newPos.x, newPos.y, newPos.z);
    auto it = find(quadrant->points, point);
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
    // so we only create it when we create a root (indicated by parent == nullptr)
    // node. Each subtree inherits this pointer and use its global
    // information
    if (parent == nullptr)
        lastPosition = new std::map<const cObject *, Coord>;
    else
        lastPosition = parent->lastPosition;
}

QuadTree::~QuadTree()
{
    for (auto& elem : quadrants)
        delete elem;
    // We clear lastPosition if and only if we delete the whole tree
    // Take a look at the constructor to see why we do this!
    if (parent == nullptr)
        delete lastPosition;
}

} // namespace inet

