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

#include "inet/common/geometry/container/BVHTree.h"
#include <limits>
#include <algorithm>
#include "inet/common/geometry/shape/Cuboid.h"

namespace inet {

bool BVHTree::isLeaf() const
{
    return objects.size() != 0;
}

BVHTree::BVHTree(const Coord& boundingMin, const Coord& boundingMax, std::vector<const PhysicalObject*>& objects, unsigned int start, unsigned int end, Axis axis, unsigned int leafCapacity)
{
    this->left = NULL;
    this->right = NULL;
    this->boundingMin = boundingMin;
    this->boundingMax = boundingMax;
    this->center = (boundingMax - boundingMin) / 2 + boundingMin;
    this->leafCapacity = leafCapacity;
    buildHierarchy(objects, start, end, axis);
}

void BVHTree::buildHierarchy(std::vector<const PhysicalObject*>& objects, unsigned int start, unsigned int end, Axis axis)
{
    if (end - start + 1 <= leafCapacity)
    {
        for (unsigned int i = start; i <= end; i++)
           this->objects.push_back(objects[i]);
    }
    else
    {
        ObjVecIterator s = objects.begin();
        ObjVecIterator e = s;
        std::advance(s, start);
        std::advance(e, end + 1);
        sort(s, e, AxisComparator(axis.getCurrentAxis()));
        axis.getNextAxis();
        Coord boundingMin, boundingMax;
        computeBoundingBox(boundingMin, boundingMax, objects, start, (start + end) / 2);
        left = new BVHTree(boundingMin, boundingMax, objects, start, (start + end) / 2, axis, leafCapacity);
        computeBoundingBox(boundingMin, boundingMax, objects, 1 + (start + end) / 2, end);
        right = new BVHTree(boundingMin, boundingMax, objects, 1 + (start + end) / 2, end, axis, leafCapacity);
    }
}

void BVHTree::computeBoundingBox(Coord& boundingMin, Coord& boundingMax, std::vector<const PhysicalObject*>& objects, unsigned int start, unsigned int end) const
{
    double xMin = std::numeric_limits<double>::max();
    double yMin = xMin;
    double zMin = xMin;
    double xMax = -std::numeric_limits<double>::max();
    double yMax = xMax;
    double zMax = xMax;
    for (unsigned int i = start; i <= end; i++)
    {
        const PhysicalObject *phyObj = objects[i];
        Coord pos = phyObj->getPosition();
        Coord size = phyObj->getShape()->computeBoundingBoxSize();
        size /= 2;
        Coord objMaxBounding = pos + size;
        Coord objMinBounding = pos - size;
        if (objMaxBounding.x > xMax)
            xMax = objMaxBounding.x;
        if (objMaxBounding.y > yMax)
            yMax = objMaxBounding.y;
        if (objMaxBounding.z > zMax)
            zMax = objMaxBounding.z;
        if (objMinBounding.x < xMin)
            xMin = objMinBounding.x;
        if (objMinBounding.y < yMin)
            yMin = objMinBounding.y;
        if (objMinBounding.z < zMin)
            zMin = objMinBounding.z;
    }
    boundingMin = Coord(xMin, yMin, zMin);
    boundingMax = Coord(xMax, yMax, zMax);
}

bool BVHTree::intersectWithLineSegment(const LineSegment& lineSegment) const
{
    Coord size = Coord(boundingMax.x - boundingMin.x, boundingMax.y - boundingMin.y, boundingMax.z - boundingMin.z);
    Coord p0 = lineSegment.getPoint1() - center;
    Coord p1 = lineSegment.getPoint2() - center;
    Cuboid cuboid(size);
    LineSegment translatedLineSegment(p0, p1);
    Coord intersection1, intersection2, normal1, normal2; // TODO: implement a bool computeIntersection(lineSegment) function
    return cuboid.computeIntersection(translatedLineSegment, intersection1, intersection2, normal1, normal2);
}

void BVHTree::lineSegmentQuery(const LineSegment& lineSegment, const IVisitor *visitor) const
{
    if (isLeaf())
    {
        for (unsigned int i = 0; i < objects.size(); i++)
            visitor->visit(objects[i]);
    }
    else if (intersectWithLineSegment(lineSegment))
    {
        left->lineSegmentQuery(lineSegment, visitor);
        right->lineSegmentQuery(lineSegment, visitor);
    }
}

BVHTree::~BVHTree()
{
    delete left;
    delete right;
}

} /* namespace inet */
