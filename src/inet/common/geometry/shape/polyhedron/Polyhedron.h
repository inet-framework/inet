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

#ifndef __INET_POLYHEDRON_H
#define __INET_POLYHEDRON_H

#include "inet/common/geometry/shape/polyhedron/PolyhedronPoint.h"
#include "inet/common/geometry/shape/polyhedron/PolyhedronEdge.h"
#include "inet/common/geometry/shape/polyhedron/PolyhedronFace.h"
#include "inet/common/geometry/base/ShapeBase.h"
#include "inet/common/geometry/common/Rotation.h"

namespace inet {

/*
 * This class represents a convex polyhedron.
 * It takes a 3D point set and builds its convex hull.
 */
class INET_API Polyhedron : public ShapeBase
{
    public:
        typedef std::vector<PolyhedronPoint *> Points;
        typedef std::vector<PolyhedronFace *> Faces;
        typedef std::vector<PolyhedronEdge *> Edges;

    protected:
        Faces faces;
        Points points;

    protected:
        void purgeWrappedFaces();
        void buildConvexHull();
        bool areCollinear(const PolyhedronPoint *lineP1, const PolyhedronPoint *lineP2, const PolyhedronPoint *point) const;
        bool areCoplanar(const PolyhedronPoint *p1, const PolyhedronPoint *p2, const PolyhedronPoint *p3, const PolyhedronPoint *p4) const;
        bool areCoplanar(const PolyhedronFace *face1, const PolyhedronFace *face2) const;
        void mergeFaces(PolyhedronFace *newFace, PolyhedronFace *neighborFace, PolyhedronPoint *point);
        void createInitialTetrahedron();
        void initializeConflictGraph();
        void cleanConflictGraph(const Faces& conflictVector);
        void purgeConflictFaces(const Faces& conflictVector);
        void connectFaces(PolyhedronFace *newFace);
        void setContlictListForNewFace(PolyhedronFace *newFace, const PolyhedronFace *neighbor1, const PolyhedronFace *neighbor2);
        void generateAndAddTetrahedronFaces(const Points& tetrahedronPoints);
        PolyhedronPoint computeOutwardNormalVector(const PolyhedronFace *face) const;
        void addFace(PolyhedronFace *face);
        Edges computeHorizonEdges(const Faces& visibleFaces) const;
        bool isVisibleFromView(const PolyhedronFace *face, const Rotation& viewRotation, const Rotation& rotation) const;

    public:
        Polyhedron(const std::vector<Coord>& points);
        Coord computeBoundingBoxSize() const override;
        void computeVisibleFaces(std::vector<std::vector<Coord> >& faces, const Rotation& rotation, const Rotation& viewRotation) const;
        bool computeIntersection(const LineSegment& lineSegment, Coord& intersection1, Coord& intersection2, Coord& normal1, Coord& normal2) const override;
        const Faces& getFaces() const { return faces; }
        const Points& getPoints() const { return points; }
        virtual ~Polyhedron();
};

} /* namespace inet */

#endif // ifndef __INET_POLYHEDRON_H
