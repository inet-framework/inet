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

#ifndef __INET_EDGE_H_
#define __INET_EDGE_H_

#include "PolytopePoint.h"
#include "Face.h"
#include "Coord.h"

namespace inet {

class Face;
class PolytopePoint;

class Edge
{
    protected:
        PolytopePoint *point1; // start point
        PolytopePoint *point2; // end point
        Face *parentFace; // the face that contains this edge
        Face *jointFace; // the face that shares this edge with the parent face
        Edge *jointEdge; // the common edge between the parentFace and jointFace (this points to the joint face's edge)
        Edge *next; // the edge that follows the current edge (in a face)
        Edge *prev; // the edge that precedes the current edge (in a face)

    public:
        Edge(PolytopePoint *point1, PolytopePoint *point2, Face *parentFace) :
             point1(point1), point2(point2), parentFace(parentFace), jointFace(NULL),
             jointEdge(NULL), next(NULL), prev(NULL) {};
        PolytopePoint *getP1() { return point1; }
        PolytopePoint *getP2() { return point2; }
        const PolytopePoint *getP1() const { return point1; }
        const PolytopePoint *getP2() const { return point2; }
        void setNextEdge(Edge *next) { this->next = next; }
        void setPrevEdge(Edge *prev) { this->prev = prev; }
        void setJointEdge(Edge *jointEdge) { this->jointEdge = jointEdge; }
        Edge *getjointEdge() { return jointEdge; }
        Edge *getNextEdge() { return next; }
        Edge *getPrevEdge() { return prev; }
        Face *getParentFace() { return parentFace; }
        void setJointFace(Face *jointFace) { this->jointFace = jointFace; }
        Face *getJointFace() { return jointFace; }
        PolytopePoint getEdgeVector() const;
        bool operator==(const Edge& rhs) const;
};

} /* namespace inet */

#endif /* __INET_EDGE_H_ */
