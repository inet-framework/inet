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
