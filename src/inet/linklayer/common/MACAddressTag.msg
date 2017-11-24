//
// This program is property of its copyright holder. All rights reserved.
//

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.linklayer.common.MacAddress;

namespace inet;

class MacAddressTagBase extends TagBase
{
    MacAddress srcAddress;      // may be unspecified
    MacAddress destAddress; // always specified
}

//
// Determines the source and destination MAC addresses for an outgoing packet.
//
class MacAddressReq extends MacAddressTagBase
{
}

//
// Specifies the source and destination MAC addresses for an incoming packet.
//
class MacAddressInd extends MacAddressTagBase
{
}
