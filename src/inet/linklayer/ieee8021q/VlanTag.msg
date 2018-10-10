//
// This program is property of its copyright holder. All rights reserved.
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

//
// This is an abstract base class that should not be directly added as a tag.
//
class VlanTagBase extends TagBase
{
    int vid = -1;
}

//
// This request determines the vlan that should be used to send the packet.
// It may be present on a packet from the application to the mac protocol.
//
class VlanReq extends VlanTagBase
{
}

//
// This indication specifies the vlan that was used to receive the packet.
// It may be present on a packet from the mac protocol to the application.
//
class VlanInd extends VlanTagBase
{
}
