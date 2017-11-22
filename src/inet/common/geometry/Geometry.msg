//
// This program is property of its copyright holder. All rights reserved.
//

import inet.common.INETDefs;

cplusplus{{
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/EulerAngles.h"
}}

namespace inet;

class Coord extends cObject
{
    @existingClass(true);
    double x;
    double y;
    double z;
}

class EulerAngles extends cObject
{
    @existingClass(true);
    double alpha;
    double beta;
    double gamma;
}
