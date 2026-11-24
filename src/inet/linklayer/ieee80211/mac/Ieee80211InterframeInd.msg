// SPDX-License-Identifier: LGPL-3.0-or-later
// Author: Miguel González-López

import inet.common.TagBase;

namespace inet::ieee80211;

// Rx replaces this local indication after FCS acceptance.
// False means absent or rejected interval evidence, not frame rejection.
class Ieee80211InterframeInd extends inet::TagBase
{
    bool related = false;
}
