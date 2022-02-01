//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_POLYHEDRONPOINT_H
#define __INET_POLYHEDRONPOINT_H

#include <map>
#include <vector>

#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/shape/polyhedron/PolyhedronFace.h"

namespace inet {

class PolyhedronFace;

/*
 * This class extends Coord class with face conflict list.
 */
class INET_API PolyhedronPoint : public Coord
{
  protected:
    std::vector<PolyhedronFace *> fConflict; // visible faces from this point
    bool selected;

  public:
    std::vector<PolyhedronFace *>& getConflictVector() { return fConflict; }
    bool isSelected() const { return selected; }
    void setToSelected() { selected = true; }
    bool hasConflicts() const { return !fConflict.empty(); }
    void addConflictFace(PolyhedronFace *face) { fConflict.push_back(face); }
    PolyhedronPoint(const Coord& point);
    PolyhedronPoint();
};

} /* namespace inet */

#endif

