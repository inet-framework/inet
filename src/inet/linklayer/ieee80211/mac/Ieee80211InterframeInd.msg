// SPDX-License-Identifier: LGPL-3.0-or-later

import inet.common.TagBase;

cplusplus {{
#include "inet/linklayer/ieee80211/mac/contract/AmpduReception.h"
}}

namespace inet::ieee80211;

class BlockAckReceiveContext
{
    @existingClass;
    @descriptor(false);
    @opaque;
}

// Rx replaces this local indication after FCS acceptance.
// False means absent or rejected interval evidence, not frame rejection.
class Ieee80211InterframeInd extends inet::TagBase
{
    bool related = false;
    BlockAckReceiveContext blockAckContext;
}
