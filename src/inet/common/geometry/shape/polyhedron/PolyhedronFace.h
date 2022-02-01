//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_POLYHEDRONFACE_H
#define __INET_POLYHEDRONFACE_H

#include <vector>

#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/shape/polyhedron/PolyhedronEdge.h"
#include "inet/common/geometry/shape/polyhedron/PolyhedronPoint.h"

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

#endif

