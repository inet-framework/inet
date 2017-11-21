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
    @existingClass;
    @primitive;
    @opaque;
    @tostring($.str());
}

class EulerAngles extends cObject
{
    @existingClass;
    @primitive;
    @opaque;
    @tostring($.str());
}
