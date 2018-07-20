//
// This program is property of its copyright holder. All rights reserved.
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

class L4PortTagBase extends TagBase
{
    int srcPort = -1;      // may be unspecified
    int destPort = -1;     // may be unspecified
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
