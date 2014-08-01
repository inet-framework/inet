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

#ifndef __INET_CONVEXPOLYTOPE_H_
#define __INET_CONVEXPOLYTOPE_H_

#include "PolytopePoint.h"
#include "Edge.h"
#include "Face.h"
#include "Shape3D.h"
#include "Rotation.h"

namespace inet {

/*
 * This class represents a convex polytope.
 * It takes a 3D point set and builds its convex hull.
 */
class ConvexPolytope : public Shape3D
{
    public:
        typedef std::vector<PolytopePoint *> Points;
        typedef std::vector<Face *> Faces;
        typedef std::vector<Edge *> Edges;

    protected:
        Faces faces;
        Points points;

    protected:
        void purgeWrappedFaces();
        void buildConvexHull();
        bool areCollinear(const PolytopePoint *lineP1, const PolytopePoint *lineP2, const PolytopePoint *point) const;
        bool areCoplanar(const PolytopePoint *p1, const PolytopePoint *p2, const PolytopePoint *p3, const PolytopePoint *p4) const;
        bool areCoplanar(const Face *face1, const Face *face2) const;
        void mergeFaces(Face *newFace, Face *neighborFace, PolytopePoint *point);
        void createInitialTetrahedron();
        void initializeConflictGraph();
        void cleanConflictGraph(const Faces& conflictVector);
        void purgeConflictFaces(const Faces& conflictVector);
        void connectFaces(Face *newFace);
        void setContlictListForNewFace(Face *newFace, const Face *neighbor1, const Face *neighbor2);
        void generateAndAddTetrahedronFaces(const Points& tetrahedronPoints);
        PolytopePoint computeOutwardNormalVector(const Face *face) const;
        void addFace(Face *face);
        Edges computeHorizonEdges(const Faces& visibleFaces) const;
        bool isVisibleFromPlane(const Face *face, const Coord& planeNormal, const Rotation& rotation) const;

    public:
        ConvexPolytope(const std::vector<Coord>& points);
        Coord computeSize() const;
        void computeVisibleFaces(std::vector<std::vector<Coord> >& faces, const Rotation& rotation, const Coord& viewNormal) const;
        bool computeIntersection(const LineSegment& lineSegment, Coord& intersection1, Coord& intersection2, Coord& normal1, Coord& normal2) const;
        const Faces& getFaces() const { return faces; }
        virtual ~ConvexPolytope();
};

} /* namespace inet */

#endif /* CONVEXPOLYTOPE_H_ */
