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

class NextHopAddressReq extends TagBase
{
    L3Address nextHopAddress;      // may be unspecified
}
