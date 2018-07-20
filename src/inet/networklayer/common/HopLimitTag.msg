//
// This program is property of its copyright holder. All rights reserved.
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

class HopLimitTagBase extends TagBase
{
    int hopLimit = -1;
}

//
// Determines the initial hop count (time to live, ttl) for an outgoing packet.
//
class HopLimitReq extends HopLimitTagBase
{
}

//
// Specifies the remainder hop count (time to live, ttl) for an incoming packet.
//
class HopLimitInd extends HopLimitTagBase
{
}
