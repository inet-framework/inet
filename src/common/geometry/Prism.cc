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

#include "Prism.h"

namespace inet {

Prism::Prism(double height, const Polygon& base) :
    height(height),
    base(base)
{
    if (height != 0)
    {
        genereateFaces();
        computeOutwardNormalVectors();
    }
}

Coord Prism::computeSize() const
{
    Coord min;
    Coord max;
    for (std::vector<Coord>::const_iterator it = base.getPoints().begin(); it != base.getPoints().end(); it++) {
        min = min.min(*it);
        max = max.max(*it);
    }
    return max - min + Coord(0, 0, height);
}

void Prism::genereateFaces()
{
    if (height == 0)
        throw cRuntimeError("A polygon has no faces");
    if (base.getPoints().size() > 0)
    {
        faces.clear();
        faces.push_back(base);
        Coord baseNormalUnitVector = base.getNormalUnitVector();
        std::vector<Coord> basePoints = base.getPoints();
        std::vector<Coord> translatedCopyPoints;
        for (unsigned int i = 0; i < basePoints.size(); i++)
        {
            Coord point = basePoints[i];
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
}

Coord Prism::computeOutwardNormalVectorForFace(unsigned int i) const
{
    Polygon face = faces[i];
    Coord facePoint = face.getPoints()[0];
    Coord faceNormal = face.getNormalVector();
    double mult = 1;
    for (unsigned int j = 0; j < faces.size(); j++)
    {
        if (j == i)
            continue;
        Coord fPoint = faces[j].getPoints()[0];
        int tp = 0;
        while (fPoint == facePoint)
        {
            tp++;
            fPoint = faces[j].getPoints()[tp];
        }
        if ((fPoint - facePoint) * faceNormal > 0)
        {
            mult = -1;
            break;
        }
    }
    return faceNormal * mult;
}

void Prism::computeOutwardNormalVectors()
{
    normalVectorsForFaces.clear();
    for (unsigned int i = 0; i < faces.size(); i++)
        normalVectorsForFaces.push_back(computeOutwardNormalVectorForFace(i));
}

bool Prism::computeIntersection(const LineSegment& lineSegment, Coord& intersection1, Coord& intersection2, Coord& normal1, Coord& normal2) const
{
    if (height > 0)
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
            std::vector<Coord> pointList = face.getPoints();
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
    else
        return base.computeIntersection(lineSegment, intersection1, intersection2, normal1, normal2);
}

void inet::Prism::setHeight(double height)
{
    if (height != this->height)
    {
        this->height = height;
        if (height != 0)
        {
            genereateFaces();
            computeOutwardNormalVectors();
        }
    }
}

void inet::Prism::setBase(const Polygon& base)
{
    if (base.getPoints() != this->base.getPoints())
    {
        this->base = base;
        if (height != 0)
        {
            genereateFaces();
            computeOutwardNormalVectors();
        }
    }
}

} // namespace inet
