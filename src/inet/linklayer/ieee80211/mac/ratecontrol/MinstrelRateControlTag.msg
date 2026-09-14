//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.physicallayer.wireless.ieee80211.mode.IIeee80211Mode;

namespace inet::ieee80211;

// Sender-local retry plan. Packet ownership also owns cancellation of the plan.
// The PHY removes sender tags at the transmission boundary.
class MinstrelRateControlTag extends TagBase
{
    int controllerId = -1;
    uint64_t generation = 0;
    const inet::physicallayer::IIeee80211Mode *modes[3];
    int attempts[3]; // allowed data attempts in each stage, including the first
    int stage = 0;
    int stageAttempts = 0; // completed data attempts in this stage
    bool probe = false;
}
