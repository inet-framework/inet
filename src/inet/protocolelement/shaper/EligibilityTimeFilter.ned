//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.shaper;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// Filters packets based on their eligibility time tags, dropping packets that
// don't meet timing requirements for transmission according to asynchronous
// traffic shaping rules.
//
// Operation:
// - Examines the EligibilityTimeTag attached to each packet
// - Drops packets that are missing the required tag
// - Optionally enforces maximum residence time constraints
// - Verifies that eligibility times are within acceptable bounds
// - Provides timing-based admission control for packet flows
//
// Particularly useful for:
// - Implementing IEEE 802.1Qcr asynchronous traffic shaping
// - Enforcing timing constraints in time-sensitive networking
// - Preventing excessive queuing delays in network switches
// - Building deterministic packet scheduling systems
// - Creating bounded-latency communication channels
//
// @see ~EligibilityTimeGate, ~EligibilityTimeQueue, ~EligibilityTimeMeter, ~EligibilityTimeTag
//
simple EligibilityTimeFilter extends PacketFilterBase like IPacketFilter
{
    parameters:
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
        double maxResidenceTime @unit(s) = default(-1s); // Optional max residence time limit, not used by default
        @class(EligibilityTimeFilter);
}
