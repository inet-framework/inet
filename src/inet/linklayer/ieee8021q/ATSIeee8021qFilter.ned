//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.ieee8021q;

import inet.linklayer.ieee8021q.SimpleIeee8021qFilter;
import inet.protocolelement.shaper.GroupEligibilityTimeTable;

// Implements a filtering module for the asynchronous traffic shaper
// taking scheduler groups into account.
//
// This module is part of the asynchronous shaper infrastructure.
//
// @see ~EligibilityTimeGate, ~GroupEligibilityTimeMeter, ~EligibilityTimeFilter, ~EligibilityTimeQueue, ~EligibilityTimeTag
//
module ATSIeee8021qFilter extends SimpleIeee8021qFilter
{
    parameters:
       **.groupEligibilityTimeTableModule = default(absPath(".groupEligibilityTimeTable"));

        meter[*].typename = "GroupEligibilityTimeMeter";
        filter[*].typename = "EligibilityTimeFilter";

    submodules:
        groupEligibilityTimeTable: GroupEligibilityTimeTable {
            @display("p=43,30");
        }
}
