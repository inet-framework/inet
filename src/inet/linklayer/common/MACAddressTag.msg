//
// This program is property of its copyright holder. All rights reserved.
//

cplusplus{{
#include "inet/linklayer/common/MACAddress.h"
}}

import inet.common.TagBase;

namespace inet;

class noncobject MACAddress;

class MacAddressTagBase extends TagBase
{
    MACAddress srcAddress;      // may be unspecified
    MACAddress destAddress; // always specified
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
