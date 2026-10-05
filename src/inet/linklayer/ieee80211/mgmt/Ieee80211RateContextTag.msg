//
// Copyright (C) 2026 OpenSim Ltd.
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.linklayer.common.MacAddress;

namespace inet::ieee80211;

// Sender-local BSS reference. The receiver must resolve its own context.
class Ieee80211RateContextTag extends TagBase
{
    int contextKind = 2; // BssRateContextRef::NONE
    MacAddress bssid;
    uint64_t transactionId = 0;
    uint64_t generation = 0;
}
