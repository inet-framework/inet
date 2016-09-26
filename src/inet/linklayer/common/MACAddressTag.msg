//
// This program is property of its copyright holder. All rights reserved.
//

cplusplus{{
#include "inet/common/TagBase_m.h"
#include "inet/linklayer/common/MACAddress.h"
}}

namespace inet;

class noncobject MACAddress;
class TagBase extends cObject;

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
