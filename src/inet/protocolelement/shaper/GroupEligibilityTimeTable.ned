// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.protocolelement.shaper;

//
// This module is a table containing the group eligibility times needed
// for the `GroupEligibilityTimeMeter`. 
// Groups are dynamically added when a frame with a new combination of ingress port and priority
// values is seen. 
//
// This module is part of the asynchronous shaper infrastructure.
//
// @see ~ATS8021qFilter, ~GroupEligibilityTimeMeter, ~EligibilityTimeGate, ~EligibilityTimeFilter, ~EligibilityTimeQueue, ~EligibilityTimeTag

simple GroupEligibilityTimeTable
{
    parameters:
        @display("i=block/table");
        @class(GroupEligibilityTimeTable);
}
