//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.protocolelement.shaper;

//
// Extends eligibility time calculation to consider stream groups, enabling
// coordinated traffic shaping across multiple related packet flows.
//
// Operation:
// - Groups related streams based on ingress port and priority
// - Coordinates eligibility times across streams in the same group
// - Consults a shared eligibility time table for group information
// - Ensures consistent shaping behavior for grouped streams
// - Maintains per-group state for accurate eligibility calculation
//
// @see ~GroupEligibilityTimeTable, ~EligibilityTimeMeter, ~EligibilityTimeGate, ~EligibilityTimeFilter
//

simple GroupEligibilityTimeMeter extends EligibilityTimeMeter
{
	parameters:
        string groupEligibilityTimeTableModule; // Relative path to the eligibilityTimeTable
        @class(GroupEligibilityTimeMeter);
}
