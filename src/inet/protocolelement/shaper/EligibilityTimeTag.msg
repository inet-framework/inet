//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.clock.contract.ClockTime;

namespace inet;

//
// This tag specifies the simulation time after which the packet becomes eligible
// for transmission according to the asynchronous shaper.
//
// This tag is used by the asynchronous shaper infrastructure.
//
// @see ~EligibilityTimeGate, ~EligibilityTimeFilter, ~EligibilityTimeMeter, ~EligibilityTimeQueue
//
class EligibilityTimeTag extends TagBase
{
    clocktime_t eligibilityTime; // the simulation time after which the packet is eligible for transmission
}
