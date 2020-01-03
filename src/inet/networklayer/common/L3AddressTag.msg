//
// This program is property of its copyright holder. All rights reserved.
//

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.networklayer.common.L3Address;

namespace inet;

class L3AddressTagBase extends TagBase
{
    L3Address srcAddress;      // may be unspecified
    L3Address destAddress; // always specified
}

//
// Determines the source and destination network addresses for an outgoing packet.
//
class L3AddressReq extends L3AddressTagBase
{
    bool nonLocalSrcAddress = false;
}

//
// Specifies the source and destination network addresses for an incoming packet.
//
class L3AddressInd extends L3AddressTagBase
{
}
