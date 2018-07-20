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

#include "inet/environment/ground/FlatGround.h"

namespace inet {

namespace physicalenvironment {

Define_Module(FlatGround);

void FlatGround::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        elevation = par("elevation");
}

Coord FlatGround::computeGroundProjection(const Coord &position) const
{
    return Coord(position.x, position.y, elevation);
}

Coord FlatGround::computeGroundNormal(const Coord &position) const
{
    return Coord(0, 0, 1);
}

} // namespace physicalenvironment

} // namespace inet

