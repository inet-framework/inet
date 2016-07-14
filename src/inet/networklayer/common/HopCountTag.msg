//
// This program is property of its copyright holder. All rights reserved.
//

cplusplus{{
#include "inet/common/TagBase_m.h"
#include "inet/networklayer/common/L3Address.h"
}}

namespace inet;

class noncobject L3Address;
class TagBase extends cObject;

class HopCountTag extends TagBase
{
    int hopCount;
}

//
// Determines the initial hop count (time to live, ttl) for an outgoing packet.
//
class HopCountReq extends HopCountTag
{
}

//
// Specifies the remainder hop count (time to live, ttl) for an incoming packet.
//
class HopCountInd extends HopCountTag
{
}
