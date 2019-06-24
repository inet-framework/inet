//
// This program is property of its copyright holder. All rights reserved.
//

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.common.Units;

namespace inet::inetmanet;

class DeepFirstForwardTag extends TagBase
{
    uint64_t seqNumber;
    bool dupFlag;
    bool retFlag;
}