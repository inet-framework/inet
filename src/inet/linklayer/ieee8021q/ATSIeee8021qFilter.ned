//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.ieee8021q;

import inet.linklayer.ieee8021q.SimpleIeee8021qFilter;
import inet.protocolelement.shaper.GroupEligibilityTimeTable;

//
// Specialized IEEE 802.1Q filter for Asynchronous Traffic Shaping (ATS) that manages traffic streams 
// based on eligibility time groups. Extends the simple IEEE 802.1Q filter with group-based eligibility 
// time metering and filtering capabilities. Implements traffic shaping according to IEEE 802.1Qcr, 
// allowing streams to be forwarded only when their eligibility time has been reached.
//
// @see ~SimpleIeee8021qFilter, ~GroupEligibilityTimeTable, ~GroupEligibilityTimeMeter, ~EligibilityTimeFilter
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
