//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.shaper;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// This module is a packet filter that operates based on the ~EligibilityTimeTag
// attached to the packet that is passing through. Packets are dropped if the tag
// is missing. If the maxResidenceTime parameter is set, then the filter also drops
// the packets for which the eligibility time in the attached ~EligibilityTimeTag
// is greater than the current simulation time plus the maximum residence time.
//
// This module is part of the asynchronous shaper architecture.
//
// @see ~EligibilityTimeGate, ~EligibilityTimeQueue, ~EligibilityTimeMeter, ~EligibilityTimeTag
//
simple EligibilityTimeFilter extends PacketFilterBase like IPacketFilter
{
    parameters:
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        double maxResidenceTime @unit(s) = default(-1s); // optional max residence time limit, not used by default
        @class(EligibilityTimeFilter);
}
