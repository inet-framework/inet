//
// This program is property of its copyright holder. All rights reserved.
//

cplusplus{{
#include "inet/common/TagBase_m.h"
}}

namespace inet;

class TagBase extends cObject;

//
// Determines the fragmentation
//
class FragmentationReq extends TagBase
{
    bool dontFragment = false;   // "don't fragment" bit
}

cplusplus{{
Register_Tag(FragmentationReq);
}}
