//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/common/geometry/shape/Prism.h"

namespace inet {

Prism::Prism(double height, const Polygon& base) :
    height(height),
    base(base)
{
    if (height > 0)
    {
        if (base.getPoints().size() >= 3)
        {
            genereateFaces();
            computeOutwardNormalVectors();
        }
        else
            throw cRuntimeError("We need at least three points to construct a prism");
    }
    else
        throw cRuntimeError("A prism has a positive height");
}

Coord Prism::computeBoundingBoxSize() const
{
    Coord min;
    Coord max;
    for (const auto & elem : base.getPoints()) {
        min = min.min(elem);
        max = max.max(elem);
    }
    return max - min + Coord(0, 0, height);
}

void Prism::genereateFaces()
{
    faces.clear();
    faces.push_back(base);
    Coord baseNormalUnitVector = base.getNormalUnitVector();
    const std::vector<Coord>& basePoints = base.getPoints();
    std::vector<Coord> translatedCopyPoints;
    for (auto & basePoint : basePoints)
    {
        Coord point = basePoint;
        point.z += height;
        translatedCopyPoints.push_back(point);
    }
    Polygon translatedCopy(translatedCopyPoints);
    faces.push_back(translatedCopy);
    unsigned int basePointsSize = basePoints.size();
    for (unsigned int i = 0; i < basePointsSize; i++)
    {
        std::vector<Coord> facePoints;
        facePoints.push_back(basePoints[i]);
        facePoints.push_back(translatedCopyPoints[i]);
        facePoints.push_back(translatedCopyPoints[(i+1) % basePointsSize]);
        facePoints.push_back(basePoints[(i+1) % basePointsSize]);
        Polygon face(facePoints);
        faces.push_back(face);
    }
}

Coord Prism::computeOutwardNormalVector(unsigned int faceId) const
{
    Polygon face = faces[faceId];
    Polygon testFace = faces[(faceId + 1) % faces.size()];
    const std::vector<Coord>& testPoints = testFace.getPoints();
    // This is a good test point: for convex polygons, the centroid is always an interior point.
    Coord testCentroid;
    for (auto & testPoint : testPoints)
        testCentroid += testPoint;
    testCentroid /= testPoints.size();
    Coord facePoint = face.getPoints()[0];
    Coord faceNormal = face.getNormalVector();
    if ((testCentroid - facePoint) * faceNormal > 0)
        return faceNormal * (-1);
    return faceNormal;
}

bool Prism::isVisibleFromPoint(unsigned int faceId, const Coord& point, const Rotation& rotation) const
{
    const std::vector<Coord>& polygonPoints = faces.at(faceId).getPoints();
    Coord facePoint = polygonPoints.at(0);
    Coord facePointPoint = point - facePoint;
    Coord rotatedFaceNormal = rotation.rotateVectorClockwise(normalVectorsForFaces.at(faceId));
    return facePointPoint * rotatedFaceNormal > 0;
}

bool Prism::isVisibleFromView(unsigned int faceId, const Rotation& viewRotation, const Rotation& rotation) const
{
    Coord zNormal(0,0,1);
    Coord rotatedFaceNormal = viewRotation.rotateVectorClockwise(rotation.rotateVectorClockwise(normalVectorsForFaces.at(faceId)));
    return rotatedFaceNormal * zNormal > 0;
}

void Prism::computeOutwardNormalVectors()
{
    normalVectorsForFaces.clear();
    for (unsigned int i = 0; i < faces.size(); i++)
        normalVectorsForFaces.push_back(computeOutwardNormalVector(i));
}

bool Prism::computeIntersection(const LineSegment& lineSegment, Coord& intersection1, Coord& intersection2, Coord& normal1, Coord& normal2) const
{
    // Note: based on http://geomalgorithms.com/a13-_intersect-4.html
    Coord p0 = lineSegment.getPoint1();
    Coord p1 = lineSegment.getPoint2();
    if (p0 == p1)
    {
        normal1 = normal2 = Coord::NIL;
        return false;
    }
    Coord segmentDirection = p1 - p0;
    double tE = 0;
    double tL = 1;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        Polygon face = faces[i];
        Coord normalVec = normalVectorsForFaces[i];
        const std::vector<Coord>& pointList = face.getPoints();
        Coord f0 = pointList[0];
        double N = (f0 - p0) * normalVec;
        double D = segmentDirection * normalVec;
        if (D < 0)
        {
            double t = N / D;
            if (t > tE)
            {
                tE = t;
                normal1 = normalVec;
                if (tE > tL)
                    return false;
            }
        }
        else if (D > 0)
        {
            double t = N / D;
            if (t < tL)
            {
                tL = t;
                normal2 = normalVec;
                if (tL < tE)
                    return false;
            }
        }
        else
        {
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
    if (intersection1 == intersection2)
    {
        normal1 = normal2 = Coord::NIL;
        return false;
    }
    return true;
}

void inet::Prism::setHeight(double height)
{
    if (height != this->height)
    {
        if (height > 0)
        {
            this->height = height;
            genereateFaces();
            computeOutwardNormalVectors();
        }
        else
            throw cRuntimeError("A prism has a positive height");
    }
}

void inet::Prism::setBase(const Polygon& base)
{
    if (base.getPoints() != this->base.getPoints())
    {
        if (base.getPoints().size() >= 3)
        {
            this->base = base;
            genereateFaces();
            computeOutwardNormalVectors();
        }
        else
            throw cRuntimeError("We need at least three points to construct a prism");
    }
}

void Prism::computeVisibleFaces(std::vector<std::vector<Coord> >& faces, const Rotation& rotation, const Rotation& viewRotation) const
{
    for (unsigned int i = 0; i < this->faces.size(); i++)
    {
        const Polygon& face = this->faces.at(i);
        if (isVisibleFromView(i, viewRotation, rotation))
        {
            const std::vector<Coord>& facePoints = face.getPoints();
            std::vector<Coord> points;
            for (const auto & facePoint : facePoints)
                points.push_back(facePoint);
            faces.push_back(points);
        }
    }
}

} // namespace inet
