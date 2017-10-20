//
// This program is property of its copyright holder. All rights reserved.
//

cplusplus{{
#include "inet/networklayer/common/L3Address.h"
}}

import inet.common.TagBase;

namespace inet;

class noncobject L3Address;

class NextHopAddressReq extends TagBase
{
    L3Address nextHopAddress;      // may be unspecified
}
