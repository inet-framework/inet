//
// This program is property of its copyright holder. All rights reserved.
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

enum Direction
{
    DIRECTION_UNDEFINED = -1;
    DIRECTION_INBOUND   = 0;
    DIRECTION_OUTBOUND  = 1;
}

class DirectionTag extends TagBase
{
    Direction direction = DIRECTION_UNDEFINED;
}
