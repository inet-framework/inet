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

//
// This tag specifies the intended direction of the packet as one of inbound,
// outbound, or undefined. The direction should be set to:
//  - outbound when
//    - a new packet is generated (e.g. in an application or in a protocol like TCP)
//    - a packet is sent to the lower layer
//    - a packet is received from the upper layer
//  - inbound when
//    - a packet is forwarded (e.g. in a protocol like IP)
//    - a packet is received from the lower layer
//    - a packet is sent to the upper layer
//
class DirectionTag extends TagBase
{
    Direction direction = DIRECTION_UNDEFINED;
}
