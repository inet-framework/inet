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

#ifndef __INET_FACE_H_
#define __INET_FACE_H_

#include <vector>
#include "PolytopePoint.h"
#include "Edge.h"
#include "Coord.h"

namespace inet {

class Edge;
class PolytopePoint;

class Face
{
    public:
        typedef std::vector<Edge *> Edges;
        typedef std::vector<PolytopePoint *> Points;

    protected:
        Edges edges; // edges of this face
        Points pConflict; // visible points from that face
        Coord outwardNormalVector; // the outward normal vector with respect to the polytope that contain this face
        Coord normalVector; // an arbitrary normal vector of this face
        Coord centroid; // centroid of this face, note that, we can use (in convex polygons!) this point as an interior point, since for convex polygons the centroid is always an interior point
        bool wrapped; // is this face wrapped by an other face?

    public:
        bool isWrapped() const { return wrapped; }
        void setToWrapped() { wrapped = true; }
        std::vector<PolytopePoint *>& getConflictVector() { return pConflict; }
        const std::vector<PolytopePoint *>& getConflictVector() const { return pConflict; }
        void computeCentroid();
        bool isVisibleFrom(const PolytopePoint *point) const;
        Edge *getEdge(unsigned int i) const;
        Edges& getEdges() { return edges; }
        const Edges& getEdges() const { return edges; }
        bool hasConflicts() const { return !pConflict.empty(); }
        void addConflictPoint(PolytopePoint *point) { pConflict.push_back(point); }
        void computeNormalVector();
        Edge *findEdge(Edge *edge);
        Coord getNormalVector() const { return normalVector; }
        Coord getOutwardNormalVector() const { return outwardNormalVector; }
        void pushEdge(Edge *edge);
        Coord getCentroid() const { return centroid; }
        void setOutwardNormalVector(const Coord& outwardNormalVector) { this->outwardNormalVector = outwardNormalVector; }
        Face(PolytopePoint *p1, PolytopePoint *p2, PolytopePoint *p3);
        virtual ~Face();
};

} /* namespace inet */

#endif /* __INET_FACE_H_ */
