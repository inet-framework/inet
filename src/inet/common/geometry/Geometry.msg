//
// This program is property of its copyright holder. All rights reserved.
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
