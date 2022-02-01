//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.shaper;

import inet.queueing.base.PacketGateBase;
import inet.queueing.contract.IPacketGate;

//
// This module is a packet gate that operates based on the ~EligibilityTimeTag
// attached to the next packet waiting to be pulled through. The gate is closed
// if the eligibility time is greater than the current simulation time, it is
// open otherwise.
//
// This module is part of the asynchronous shaper architecture.
//
// @see ~EligibilityTimeQueue, ~EligibilityTimeFilter, ~EligibilityTimeMeter, ~EligibilityTimeTag
//
simple EligibilityTimeGate extends PacketGateBase like IPacketGate
{
    parameters:
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        @signal[remainingEligibilityTimeChanged](type=simtime_t);
        @statistic[remainingEligibilityTime](title="remaining eligibility time"; source=remainingEligibilityTimeChanged; record=vector; unit=s; interpolationmode=linear);
        @class(EligibilityTimeGate);
}
