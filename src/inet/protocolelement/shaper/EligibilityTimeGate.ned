//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.shaper;

import inet.queueing.base.PacketGateBase;
import inet.queueing.contract.IPacketGate;

//
// Controls packet flow based on eligibility time tags, allowing packets to pass
// only when their scheduled transmission time has been reached according to
// asynchronous traffic shaping rules.
//
// Operation:
// - Examines the EligibilityTimeTag of the next packet in queue
// - Opens the gate when current time reaches or exceeds eligibility time
// - Keeps the gate closed while packets are not yet eligible for transmission
// - Tracks and reports remaining time until next packet becomes eligible
// - Provides time-based traffic shaping without packet reordering
//
// @see ~EligibilityTimeQueue, ~EligibilityTimeFilter, ~EligibilityTimeMeter, ~EligibilityTimeTag
//
simple EligibilityTimeGate extends PacketGateBase like IPacketGate
{
    parameters:
        string clockModule = default(""); // Relative path of the module that implements IClock; optional
        @signal[remainingEligibilityTimeChanged](type=simtime_t);
        @statistic[remainingEligibilityTime](title="remaining eligibility time"; source=remainingEligibilityTimeChanged; record=vector; unit=s; interpolationmode=linear);
        @class(EligibilityTimeGate);
}
