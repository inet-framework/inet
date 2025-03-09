//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.shaper;

import inet.queueing.queue.PacketQueue;

//
// Stores packets in ascending order of their eligibility times, providing
// priority-based queuing for asynchronous traffic shaping.
//
// Operation:
// - Sorts packets based on their EligibilityTimeTag values
// - Maintains packets in earliest-eligibility-time-first order
// - Uses a specialized comparator to determine packet ordering
// - Ensures packets with earlier eligibility times are processed first
// - Preserves packet order within the same eligibility time
//
// Particularly useful for:
// - Implementing IEEE 802.1Qcr asynchronous traffic shaping
// - Creating priority queues based on eligibility times
// - Supporting time-sensitive networking applications
// - Building deterministic packet scheduling systems
// - Implementing earliest deadline first scheduling
//
// @see ~EligibilityTimeGate, ~EligibilityTimeFilter, ~EligibilityTimeMeter, ~EligibilityTimeTag
//
simple EligibilityTimeQueue extends PacketQueue
{
    parameters:
        comparatorClass = default("inet::PacketEligibilityTimeComparator"); // Specifies the class that compares the eligibility time in the ~EligibilityTimeTag of two packets
}
