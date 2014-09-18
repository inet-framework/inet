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

#include "inet/common/geometry/polytope/Face.h"

namespace inet {

Face::Face(PolytopePoint *p1, PolytopePoint *p2, PolytopePoint  *p3)
{
    // Constructor for triangular face
    outwardNormalVector = Coord::NIL;
    normalVector = Coord::NIL;
    centroid = Coord::NIL;
    wrapped = false;
    Edge *edge1 = new Edge(p1, p2, this);
    Edge *edge2 = new Edge(p2, p3, this);
    Edge *edge3 = new Edge(p3, p1, this);
    edge1->setNextEdge(edge2);
    edge2->setNextEdge(edge3);
    edge3->setNextEdge(NULL);
    edge1->setPrevEdge(edge3);
    edge2->setPrevEdge(edge1);
    edge3->setPrevEdge(edge2);
    pushEdge(edge1);
    pushEdge(edge2);
    pushEdge(edge3);
    computeNormalVector();
}

void Face::computeCentroid()
{
    centroid = Coord(0,0,0);
    unsigned int numberOfPoints = edges.size();
    for (Edges::const_iterator eit = edges.begin(); eit != edges.end(); eit++)
    {
        PolytopePoint *point = (*eit)->getP1();
        centroid += *point;
    }
    ASSERT(numberOfPoints != 0);
    centroid /= numberOfPoints;
}

void Face::pushEdge(Edge* edge)
{
    edges.push_back(edge);
    // We need to recompute the centroid
    computeCentroid();
}

Face::~Face()
{
    for (Edges::iterator it = edges.begin(); it != edges.end(); it++)
        delete *it;
}

void Face::computeNormalVector()
{
    normalVector = edges[0]->getEdgeVector() % edges[0]->getNextEdge()->getEdgeVector();
}

Edge* Face::getEdge(unsigned int i) const
{
    if (i >= edges.size())
        throw cRuntimeError("Out of range with index = %d", i);
    return edges.at(i);
}

bool Face::isVisibleFrom(const PolytopePoint* point) const
{
    PolytopePoint *facePoint = getEdge(0)->getP1();
    PolytopePoint facePointPoint = *point - *facePoint;
    return facePointPoint * outwardNormalVector > 0;
}

Edge* Face::findEdge(Edge* edge)
{
    for (Edges::iterator it = edges.begin(); it != edges.end(); it++)
    {
        Edge *currEdge = *it;
        if (*currEdge == *edge)
            return currEdge;
    }
    return NULL;
}

} /* namespace inet */

