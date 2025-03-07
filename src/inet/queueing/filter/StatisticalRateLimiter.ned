//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.filter;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// Probabilistic rate limiter that enforces configurable bandwidth and packet rate
// constraints using a statistical approach. This module provides traffic shaping
// with gradual packet dropping as rates exceed configured thresholds.
//
// Operation mechanism:
// - Examines the RateTag attached to each packet to determine current rates
// - Calculates drop probability based on how much rates exceed configured maximums
// - Higher excess rates result in higher drop probabilities
// - Packets exceeding limits are dropped with probability proportional to the excess
// - Packets within limits are always forwarded
//
// The module can limit based on data rate (bits per second), packet rate (packets
// per second), or both simultaneously. When both limits are configured, the more
// restrictive one (resulting in higher drop probability) is applied.
//
// @see ~ExponentialRateMeter, ~RedDropper, ~TokenBasedServer
//
simple StatisticalRateLimiter extends PacketFilterBase like IPacketFilter
{
    parameters:
        double maxDatarate @unit(bps) = default(nan);
        double maxPacketrate = default(nan);
        @class(StatisticalRateLimiter);
}
