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

class MACAddressTag extends TagBase
{
    MACAddress sourceAddress;      // may be unspecified
    MACAddress destinationAddress; // always specified
}

//
// Determines the source and destination MAC addresses for an outgoing packet.
//
class MACAddressReq extends MACAddressTag
{
}

//
// Specifies the source and destination MAC addresses for an incoming packet.
//
class MACAddressInd extends MACAddressTag
{
}
