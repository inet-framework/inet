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
    genereateFaces();
    calculateOutwardNormalVectors();
}

Coord Prism::computeSize() const
{
    Coord min;
    Coord max;
    for (std::vector<Coord>::const_iterator it = base.getPoints().begin(); it != base.getPoints().end(); it++) {
        min = min.min(*it);
        max = max.max(*it);
    }
    return max - min;
}

void Prism::genereateFaces()
{
    faces.push_back(base);
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

Coord Prism::calculateOutwardNormalVectorForFace(unsigned int i) const
{
    Polygon face = faces[i];
    std::vector<Coord> facePoints = face.getPoints();
    // Note that, vectorA and vectorB are not parallel vectors
    // See the order of the points in generateFaces()
    Coord vectorA = facePoints[0] - facePoints[1];
    Coord vectorB = facePoints[2] - facePoints[1];
    // Then the cross product vectorC of vectorA and vectorB
    // is normal to the plane that vectorA and vectorB generate
    Coord vectorC(vectorA.y * vectorB.z - vectorA.z * vectorB.y,
                 vectorA.z * vectorB.x - vectorA.x * vectorB.z,
                 vectorA.x * vectorB.y - vectorA.y * vectorB.x);
    // The outward direction normal will be -vectorC for the translatedCopy
    if (i == 1)
        return vectorC * (-1);
    return vectorC;
}

void Prism::calculateOutwardNormalVectors()
{
    for (unsigned int i = 0; i < faces.size(); i++)
        normalVectorsForFaces.push_back(calculateOutwardNormalVectorForFace(i));
}

bool Prism::computeIntersection(const LineSegment& lineSegment, Coord& intersection1, Coord& intersection2, Coord& normal1, Coord& normal2) const
{
    // Note: based on http://geomalgorithms.com/a13-_intersect-4.html
    Coord p0 = lineSegment.getPoint1();
    Coord p1 = lineSegment.getPoint2();
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
        normal1 = Coord(0,0,0);
    if (tL == 1)
        normal2 = Coord(0,0,0);
    intersection1 = p0 + segmentDirection * tE;
    intersection2 = p0 + segmentDirection * tL;
    return true;
}

} // namespace inet

