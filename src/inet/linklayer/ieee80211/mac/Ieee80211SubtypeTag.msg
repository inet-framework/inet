//
// This program is property of its copyright holder. All rights reserved.
//

cplusplus{{
#include "inet/common/INETDefs.h"
}}

import inet.common.TagBase;

namespace inet;

class TagBase extends cObject;

class Ieee80211SubtypeTagBase extends TagBase
{
    int subtype;
}

//
// Determines the MAC header subtype for an outgoing packet.
//
class Ieee80211SubtypeReq extends Ieee80211SubtypeTagBase
{
}

//
// Specifies the MAC header subtype for an incoming packet.
//
class Ieee80211SubtypeInd extends Ieee80211SubtypeTagBase
{
}
