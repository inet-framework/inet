// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.protocolelement.shaper;

//
// Maintains a shared repository of eligibility times for stream groups in
// asynchronous traffic shaping, enabling coordinated transmission scheduling.
//
// Operation:
// - Stores eligibility times for stream groups
// - Dynamically creates groups based on ingress port and priority combinations
// - Provides lookup services for GroupEligibilityTimeMeter modules
// - Tracks the latest eligibility time for each group
// - Ensures consistent shaping behavior across related streams
//
// Particularly useful for:
// - Implementing IEEE 802.1Qcr asynchronous traffic shaping
// - Coordinating traffic flows with shared resources
// - Supporting hierarchical traffic shaping architectures
// - Building advanced time-sensitive networking systems
// - Enabling group-based traffic management policies
//
// @see ~GroupEligibilityTimeMeter, ~EligibilityTimeGate, ~EligibilityTimeFilter, ~EligibilityTimeQueue

simple GroupEligibilityTimeTable
{
    parameters:
        @display("i=block/table");
        @class(GroupEligibilityTimeTable);
}
