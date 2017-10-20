//
// This program is property of its copyright holder. All rights reserved.
//

cplusplus{{
#include "inet/common/INETDefs.h"
}}

import inet.common.TagBase;

namespace inet;

//
// Determines the fragmentation
//
class FragmentationReq extends TagBase
{
    bool dontFragment = false;   // "don't fragment" bit
}

