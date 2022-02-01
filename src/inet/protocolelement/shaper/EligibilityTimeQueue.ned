//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.shaper;

import inet.queueing.queue.PacketQueue;

//
// This module is a packet queue that keeps the packets in ascending order based
// on the eligibility time in the attached ~EligibilityTimeTag of the packets.
//
// This module is part of the asynchronous shaper infrastructure.
//
// @see ~EligibilityTimeGate, ~EligibilityTimeFilter, ~EligibilityTimeMeter, ~EligibilityTimeTag
//
simple EligibilityTimeQueue extends PacketQueue
{
    parameters:
        comparatorClass = default("inet::PacketEligibilityTimeComparator"); // specifies the class that compares the eligibility time in the ~EligibilityTimeTag of two packets
}
