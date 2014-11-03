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

#include "PolyhedronTest.h"
#include "inet/common/geometry/shape/polyhedron/PolyhedronPoint.h"
#include "inet/common/geometry/shape/polyhedron/PolyhedronFace.h"

namespace inet {

Define_Module(PolyhedronTest);

PolyhedronTest::PolyhedronTest()
{
    polyhedron = NULL;
}

void PolyhedronTest::parsePoints(const char* strPoints)
{
    cStringTokenizer tokenizer(strPoints);
    double x, y, z;
    while (tokenizer.hasMoreTokens())
    {
        x = atof(tokenizer.nextToken());
        if (tokenizer.hasMoreTokens())
            y = atof(tokenizer.nextToken());
        if (tokenizer.hasMoreTokens())
        {
            z = atof(tokenizer.nextToken());
            Coord point(x,y,z);
            points.push_back(point);
        }
    }
}

void PolyhedronTest::printFaces() const
{
    const Polyhedron::Faces& faces = polyhedron->getFaces();
    for (Polyhedron::Faces::const_iterator fit = faces.begin(); fit != faces.end(); fit++)
    {
        std::cout << "Face with edges: " << endl;
        Polyhedron::Edges edges = (*fit)->getEdges();
        for (Polyhedron::Edges::iterator eit = edges.begin(); eit != edges.end(); eit++)
        {
            PolyhedronEdge *edge = *eit;
            std::cout << "P1 = " << *edge->getP1() << " P2 = " << *edge->getP2() << endl;
        }
    }
}

void PolyhedronTest::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        const char *strPoints = par("points").stringValue();
        bool testWithConvexCombations = par("convexCombinationTest").boolValue();
        parsePoints(strPoints);
        polyhedron = new Polyhedron(points);
        printFaces();
        if (testWithConvexCombations)
            test();
    }
}

void PolyhedronTest::test() const
{
    // Let P := {p_1,p_2,...,p_n} be a point set with finite number of elements then conv(P)
    // (donates the convex hull of P) is the minimal convex set containing P, and equivalently
    // is the set of all convex combinations of points in P.
    // So we can test our algorithm in the following way:
    // Generate a lot of convex combinations and test if the combination is in the convex hull.
    // If it is not, then the algorithm is incorrect.
    unsigned int numberOfPoints = points.size();
    // We are testing with 1000 random convex combination.
    const Polyhedron::Faces& faces = polyhedron->getFaces();
    for (unsigned int i = 1; i != 1000; i++)
    {
        // Create a convex combination:
        // lambda_1 p_1 + lambda_2 p_2 + ... + lambda_n p_n where lambda_1 +
        // lambda_2 + ... + lambda_n = 1 and lambda_k >= 0 for all k.
        double lambda = 1.0;
        unsigned int id = 0;
        Coord convexCombination;
        while (id < numberOfPoints - 1)
        {
            double randLambda = (rand() % 5000) / 10000.0;
            while (lambda - randLambda < 0)
                randLambda =  (rand() % 5000) / 10000.0;
            lambda -= randLambda;
            convexCombination += points[id++] * randLambda;
        }
        convexCombination += points[numberOfPoints - 1] * lambda;
        std::cout << "Testing with convex combination: " << convexCombination << endl;
        for (Polyhedron::Faces::const_iterator fit = faces.begin(); fit != faces.end(); fit++)
        {
            PolyhedronFace *face = *fit;
            PolyhedronPoint testPoint(convexCombination);
            // An arbitrary point is an inner point if and only if it can't see any faces.

            // KLUDGE: this check supposed to be face->isVisibleFrom(&testPoint) but it's too sensible due to >0
            PolyhedronPoint *facePoint = face->getEdge(0)->getP1();
            PolyhedronPoint facePointPoint = testPoint - *facePoint;
            if (facePointPoint * face->getOutwardNormalVector() > 1E-10) {
                std::cout << "The algorithm is incorrect!" << endl;
                return;
            }
        }
    }
}

PolyhedronTest::~PolyhedronTest()
{
    delete polyhedron;
}

} /* namespace inet */
