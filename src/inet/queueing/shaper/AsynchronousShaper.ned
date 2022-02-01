//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.shaper;

//
// This module implements an asynchronous shaper. The queue sorts packets based
// on the transmission eligibility time (~EligibilityTimeTag) attached to each
// packet. The gate prevents packets to be transmitter earlier than the attached
// transmission eligibility time.
//
// @see ~EligibilityTimeMeter, ~EligibilityTimeFilter, ~EligibilityTimeQueue, ~EligibilityTimeGate
//
module AsynchronousShaper extends PacketShaper
{
    parameters:
        queue.typename = default("EligibilityTimeQueue");
        gate.typename = default("EligibilityTimeGate");
}
