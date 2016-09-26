//
// This program is property of its copyright holder. All rights reserved.
//

cplusplus{{
#include "inet/common/TagBase_m.h"
}}

namespace inet;

class TagBase extends cObject;

class L4PortTagBase extends TagBase
{
    int srcPort;      // may be unspecified
    int destPort;     // may be unspecified
}

//
// Determines the source and destination L4 ports for an outgoing packet.
//
class L4PortReq extends L4PortTagBase
{
}

//
// Specifies the source and destination L4 ports for an incoming packet.
//
class L4PortInd extends L4PortTagBase
{
}
