//
// This program is property of its copyright holder. All rights reserved.
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

class FragmentTag extends TagBase
{
    bool firstFragment = true;
    bool lastFragment = true;
    int fragmentNumber = 0; // -1 means unspecified
    int numFragments = 1;   // -1 means unspecified
}
