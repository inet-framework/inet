//
// This program is property of its copyright holder. All rights reserved.
//

cplusplus{{
#include "inet/common/TagBase_m.h"
}}

namespace inet;

class TagBase extends cObject;

class HopLimitTag extends TagBase
{
    int hopLimit = -1;
}

//
// Determines the initial hop count (time to live, ttl) for an outgoing packet.
//
class HopLimitReq extends HopLimitTag
{
}

//
// Specifies the remainder hop count (time to live, ttl) for an incoming packet.
//
class HopLimitInd extends HopLimitTag
{
}
