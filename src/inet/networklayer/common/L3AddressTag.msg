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
}

//
// Specifies the source and destination network addresses for an incoming packet.
//
class L3AddressInd extends L3AddressTagBase
{
}


cplusplus{{
Register_Tag(L3AddressReq);
Register_Tag(L3AddressInd);
}}
