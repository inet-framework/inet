//
// This program is property of its copyright holder. All rights reserved.
//

cplusplus{{
#include "inet/common/TagBase_m.h"
}}

namespace inet;

class TagBase extends cObject;

class PortsTagBase extends TagBase
{
    int srcPort;      // may be unspecified
    int destPort;     // may be unspecified
}

//
// Determines the source and destination network addresses for an outgoing packet.
//
class PortsReq extends PortsTagBase
{
}

//
// Specifies the source and destination network addresses for an incoming packet.
//
class PortsInd extends PortsTagBase
{
}
