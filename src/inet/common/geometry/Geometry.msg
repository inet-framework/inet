//
// Copyright (C) 2020 OpenSim Ltd.
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

import inet.common.INETDefs;
import inet.common.Units;

cplusplus{{
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/EulerAngles.h"
#include "inet/common/geometry/common/Quaternion.h"
}}

namespace inet;

class Coord
{
    @existingClass;
    double x;
    double y;
    double z;
}

class EulerAngles
{
    @existingClass;
    rad alpha;
    rad beta;
    rad gamma;
}

class Quaternion
{
    @existingClass;
    double s;
    Coord v;
}
