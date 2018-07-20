//
// This program is property of its copyright holder. All rights reserved.
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

//
// This is an abstract base class that should not be directly added as a tag.
//
class UserPriorityTagBase extends TagBase
{
    int userPriority = -1; // value is in the range of [0, 7]
}

//
// This request determines the user priority that should be used to send the packet.
// It may be present on a packet from the application to the mac protocol.
//
class UserPriorityReq extends UserPriorityTagBase
{
}

//
// This indication specifies the user priority that was used to receive the packet.
// It may be present on a packet from the mac protocol to the application.
//
class UserPriorityInd extends UserPriorityTagBase
{
}
