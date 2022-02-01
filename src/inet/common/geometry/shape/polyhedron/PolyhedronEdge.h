//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_POLYHEDRONEDGE_H
#define __INET_POLYHEDRONEDGE_H

#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/shape/polyhedron/PolyhedronFace.h"
#include "inet/common/geometry/shape/polyhedron/PolyhedronPoint.h"

namespace inet {

class PolyhedronFace;
class PolyhedronPoint;

class INET_API PolyhedronEdge
{
  protected:
    PolyhedronPoint *point1; // start point
    PolyhedronPoint *point2; // end point
    PolyhedronFace *parentFace; // the face that contains this edge
    PolyhedronFace *jointFace; // the face that shares this edge with the parent face
    PolyhedronEdge *jointEdge; // the common edge between the parentFace and jointFace (this points to the joint face's edge)
    PolyhedronEdge *next; // the edge that follows the current edge (in a face)
    PolyhedronEdge *prev; // the edge that precedes the current edge (in a face)

  public:
    PolyhedronEdge(PolyhedronPoint *point1, PolyhedronPoint *point2, PolyhedronFace *parentFace) :
        point1(point1), point2(point2), parentFace(parentFace), jointFace(nullptr),
        jointEdge(nullptr), next(nullptr), prev(nullptr) {};
    PolyhedronPoint *getP1() { return point1; }
    PolyhedronPoint *getP2() { return point2; }
    const PolyhedronPoint *getP1() const { return point1; }
    const PolyhedronPoint *getP2() const { return point2; }
    void setNextEdge(PolyhedronEdge *next) { this->next = next; }
    void setPrevEdge(PolyhedronEdge *prev) { this->prev = prev; }
    void setJointEdge(PolyhedronEdge *jointEdge) { this->jointEdge = jointEdge; }
    PolyhedronEdge *getjointEdge() { return jointEdge; }
    PolyhedronEdge *getNextEdge() { return next; }
    PolyhedronEdge *getPrevEdge() { return prev; }
    PolyhedronFace *getParentFace() { return parentFace; }
    void setJointFace(PolyhedronFace *jointFace) { this->jointFace = jointFace; }
    PolyhedronFace *getJointFace() { return jointFace; }
    PolyhedronPoint getEdgeVector() const;
    bool operator==(const PolyhedronEdge& rhs) const;
};

} /* namespace inet */

#endif

