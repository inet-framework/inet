//
// This program is property of its copyright holder. All rights reserved.
//

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.networklayer.common.L3Address;

namespace inet;

class NextHopAddressReq extends TagBase
{
    L3Address nextHopAddress;      // may be unspecified
}
