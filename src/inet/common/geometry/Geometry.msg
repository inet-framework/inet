//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
    @toString(.str());
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
