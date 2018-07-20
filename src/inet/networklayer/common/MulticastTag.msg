//
// This program is property of its copyright holder. All rights reserved.
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

//
// Determines the multicast loop mode
//
class MulticastReq extends TagBase
{
    bool multicastLoop = false;   // if true, sent multicast datagrams will be looped back
}

