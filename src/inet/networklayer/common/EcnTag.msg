//
// This program is property of its copyright holder. All rights reserved.
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

//
// This is an abstract base class that should not be directly added as a tag.
//
class EcnTagBase extends TagBase
{
    int explicitCongestionNotification = -1; // explicit congestion notification
}

//
// This request determines the explicit congestion notification that should be used to transmit the packet.
//
class EcnReq extends EcnTagBase
{
}

//
// This indication specifies the explicit congestion notification that was used to receive the packet.
//
class EcnInd extends EcnTagBase
{
}
