//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
        void addConflictFace(PolyhedronFace * face) { fConflict.push_back(face); }
        PolyhedronPoint(const Coord& point);
        PolyhedronPoint();
};

} /* namespace inet */

#endif

