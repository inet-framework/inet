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

#include "ConvexPolytopeTest.h"

namespace inet {

Define_Module(ConvexPolytopeTest);

ConvexPolytopeTest::ConvexPolytopeTest()
{
    polytope = NULL;
}

void ConvexPolytopeTest::parsePoints(const char* strPoints)
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

void ConvexPolytopeTest::printFaces() const
{
    const ConvexPolytope::Faces& faces = polytope->getFaces();
    for (ConvexPolytope::Faces::const_iterator fit = faces.begin(); fit != faces.end(); fit++)
    {
        std::cout << "Face with edges: " << endl;
        ConvexPolytope::Edges edges = (*fit)->getEdges();
        for (ConvexPolytope::Edges::iterator eit = edges.begin(); eit != edges.end(); eit++)
        {
            Edge *edge = *eit;
            std::cout << "P1 = " << *edge->getP1() << " P2 = " << *edge->getP2() << endl;
        }
    }
}

void ConvexPolytopeTest::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        const char *strPoints = par("points").stringValue();
        parsePoints(strPoints);
        polytope = new ConvexPolytope(points);
        printFaces();
    }
}

ConvexPolytopeTest::~ConvexPolytopeTest()
{
    delete polytope;
}

} /* namespace inet */
