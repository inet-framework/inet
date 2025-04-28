// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.protocolelement.shaper;

import inet.common.SimpleModule;

//
// A table containing the group eligibility times needed
// for the `GroupEligibilityTimeMeter`.
// Groups are dynamically added when a frame with a new combination of ingress port and priority
// values is seen. 
//
// This module is part of the asynchronous shaper infrastructure.
//
// @see ~ATS8021qFilter, ~GroupEligibilityTimeMeter, ~EligibilityTimeGate, ~EligibilityTimeFilter, ~EligibilityTimeQueue, ~EligibilityTimeTag

simple GroupEligibilityTimeTable extends SimpleModule
{
    parameters:
        @class(GroupEligibilityTimeTable);
        @display("i=block/table");
        @class(GroupEligibilityTimeTable);
}
