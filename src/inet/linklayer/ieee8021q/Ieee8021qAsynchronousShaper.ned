//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.protocolelement.shaper.EligibilityTimeGate;

//
// IEEE 802.1Qcr asynchronous traffic shaping implementation that controls packet forwarding based on 
// eligibility times. Acts as a gate that opens only when a packet's eligibility time has been reached, 
// providing deterministic latency control without synchronized scheduling. Enables traffic shaping with 
// bounded latency while maintaining high network utilization.
//
// @see ~EligibilityTimeGate, ~ATSIeee8021qFilter, ~EligibilityTimeQueue, ~EligibilityTimeTag
//
module Ieee8021qAsynchronousShaper extends EligibilityTimeGate
{
}
