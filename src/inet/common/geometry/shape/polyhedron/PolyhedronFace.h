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

#ifndef __INET_POLYHEDRONFACE_H
#define __INET_POLYHEDRONFACE_H

#include <vector>
#include "inet/common/geometry/shape/polyhedron/PolyhedronPoint.h"
#include "inet/common/geometry/shape/polyhedron/PolyhedronEdge.h"
#include "inet/common/geometry/common/Coord.h"

namespace inet {

class PolyhedronEdge;
class PolyhedronPoint;

class INET_API PolyhedronFace
{
    public:
        typedef std::vector<PolyhedronEdge *> Edges;
        typedef std::vector<PolyhedronPoint *> Points;

    protected:
        Edges edges; // edges of this face
        Points pConflict; // visible points from that face
        Coord outwardNormalVector; // the outward normal vector with respect to the polyhedron that contain this face
        Coord normalVector; // an arbitrary normal vector of this face
        Coord centroid; // centroid of this face, note that, we can use (in convex polygons!) this point as an interior point, since for convex polygons the centroid is always an interior point
        bool wrapped; // is this face wrapped by an other face?

    public:
        bool isWrapped() const { return wrapped; }
        void setToWrapped() { wrapped = true; }
        std::vector<PolyhedronPoint *>& getConflictVector() { return pConflict; }
        const std::vector<PolyhedronPoint *>& getConflictVector() const { return pConflict; }
        void computeCentroid();
        bool isVisibleFrom(const PolyhedronPoint *point) const;
        PolyhedronEdge *getEdge(unsigned int i) const;
        Edges& getEdges() { return edges; }
        const Edges& getEdges() const { return edges; }
        bool hasConflicts() const { return !pConflict.empty(); }
        void addConflictPoint(PolyhedronPoint *point) { pConflict.push_back(point); }
        void computeNormalVector();
        PolyhedronEdge *findEdge(PolyhedronEdge *edge);
        Coord getNormalVector() const { return normalVector; }
        Coord getOutwardNormalVector() const { return outwardNormalVector; }
        void pushEdge(PolyhedronEdge *edge);
        Coord getCentroid() const { return centroid; }
        void setOutwardNormalVector(const Coord& outwardNormalVector) { this->outwardNormalVector = outwardNormalVector; }
        PolyhedronFace(PolyhedronPoint *p1, PolyhedronPoint *p2, PolyhedronPoint *p3);
        virtual ~PolyhedronFace();
};

} /* namespace inet */

#endif // ifndef __INET_POLYHEDRONFACE_H
