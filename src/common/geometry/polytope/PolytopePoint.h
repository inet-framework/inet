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

#ifndef __INET_POLYTOPEPOINT_H_
#define __INET_POLYTOPEPOINT_H_

#include <vector>
#include <map>
#include "Face.h"
#include "Coord.h"

namespace inet {

class Face;

/*
 * This class extends Coord class with face conflict list.
 */
class PolytopePoint : public Coord
{
    protected:
        std::vector<Face *> fConflict; // visible faces from this point
        bool selected;

    public:
        std::vector<Face *>& getConflictVector() { return fConflict; }
        bool isSelected() const { return selected; }
        void setToSelected() { selected = true; }
        bool hasConflicts() const { return !fConflict.empty(); }
        void addConflictFace(Face * face) { fConflict.push_back(face); }
        PolytopePoint(const Coord& point);
        PolytopePoint();
};

} /* namespace inet */

#endif /* __INET_POLYTOPEPOINT_H_ */
