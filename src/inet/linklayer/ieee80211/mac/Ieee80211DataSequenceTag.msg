//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet::ieee80211;

// Local allocation order before fragmentation. All fragments retain this index.
// Unlike the wire sequence number, the index does not wrap at 4096.
class Ieee80211DataSequenceTag extends TagBase
{
    int64_t allocationIndex = -1;
}
