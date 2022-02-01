//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/geometry/object/Polygon.h"

namespace inet {

const Polygon Polygon::NIL = Polygon();

Polygon::Polygon(const std::vector<Coord>& points)
{
    if (points.size() < 3)
        throw cRuntimeError("A Euclidean polygon has at least three points");
    this->points = points;
}

bool Polygon::isUnspecified() const
{
    for (const auto& elem : points) {
        if ((elem).isUnspecified())
            return true;
    }
    return false;
}

Coord Polygon::getNormalUnitVector() const
{
    Coord normalVec = getNormalVector();
    return normalVec / normalVec.length();
}

Coord Polygon::getNormalVector() const
{
    Coord point1 = points[0];
    Coord point2 = points[1];
    Coord point3 = points[2];
    Coord vectorA = point2 - point1;
    Coord vectorB = point3 - point1;
    Coord vectorC(vectorA.y * vectorB.z - vectorA.z * vectorB.y,
                  vectorA.z * vectorB.x - vectorA.x * vectorB.z,
                  vectorA.x * vectorB.y - vectorA.y * vectorB.x);
    return vectorC;
}

Coord Polygon::computeSize() const
{
    Coord min;
    Coord max;
    for (const auto& elem : points) {
        min = min.min(elem);
        max = max.max(elem);
    }
    return max - min;
}

Coord Polygon::getEdgeOutwardNormalVector(const Coord& edgeP1, const Coord& edgeP2) const
{
    Coord polygonNormal = getNormalUnitVector();
    Coord vectorA = edgeP1 - polygonNormal;
    Coord vectorB = edgeP2 - polygonNormal;
    Coord vectorC(vectorA.y * vectorB.z - vectorA.z * vectorB.y,
                  vectorA.z * vectorB.x - vectorA.x * vectorB.z,
                  vectorA.x * vectorB.y - vectorA.y * vectorB.x);
    // The projection of a vector image v onto a plane with unit normal vector n is: p = v - (v*n)*n.
    return vectorC - polygonNormal * (vectorC * polygonNormal);
}

bool Polygon::computeIntersection(const LineSegment& lineSegment, Coord& intersection1, Coord& intersection2, Coord& normal1, Coord& normal2) const
{
    // Note: based on http://geomalgorithms.com/a13-_intersect-4.html
    Coord p0 = lineSegment.getPoint1();
    Coord p1 = lineSegment.getPoint2();
    if (p0 == p1) {
        normal1 = normal2 = Coord::NIL;
        return false;
    }
    Coord segmentDirection = p1 - p0;
    Coord polygonNormal = getNormalUnitVector();
    // The segment is not in the polygon's plane
    // The length of the intersection segment will be 0
    if (polygonNormal * segmentDirection != 0 || (p0 - points[0]) * polygonNormal != 0)
        return false;
    double tE = 0;
    double tL = 1;
    unsigned int pointSize = points.size();
    for (unsigned int i = 0; i < pointSize; i++) {
        Coord normalVec = getEdgeOutwardNormalVector(points[i], points[(i + 1) % pointSize]);
        double N = normalVec * (points[i] - p0);
        double D = normalVec * segmentDirection;
        if (D < 0) {
            double t = N / D;
            if (t > tE) {
                tE = t;
                normal1 = normalVec;
                if (tE > tL)
                    return false;
            }
        }
        else if (D > 0) {
            double t = N / D;
            if (t < tL) {
                tL = t;
                normal2 = normalVec;
                if (tL < tE)
                    return false;
            }
        }
        else {
            if (N < 0)
                return false;
        }
    }
    if (tE == 0)
        normal1 = Coord::NIL;
    if (tL == 1)
        normal2 = Coord::NIL;
    intersection1 = p0 + segmentDirection * tE;
    intersection2 = p0 + segmentDirection * tL;
    if (intersection1 == intersection2) {
        normal1 = normal2 = Coord::NIL;
        return false;
    }
    return true;
}

} // namespace inet

