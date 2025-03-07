//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.meter;

import inet.queueing.base.PacketMeterBase;
import inet.queueing.contract.IPacketMeter;

//
// This module implements a sliding window rate meter that measures the data rate
// and packet rate of a packet stream. It attaches a ~RateTag to each packet with
// the calculated rates based on packets observed within a configurable time window.
//
// The sliding window algorithm works by:
// - Maintaining a history of packets that passed through the meter
// - Considering only packets that arrived within the specified time window
// - Calculating the current data rate (bits/sec) and packet rate (packets/sec)
//   based on packets in the window
// - Attaching these rate measurements to each packet as a ~RateTag
//
// Unlike the ExponentialRateMeter which uses an exponential weighting scheme,
// this meter gives equal weight to all packets within the time window and
// completely ignores packets outside the window. This provides a more
// accurate representation of recent traffic patterns without being influenced
// by older traffic.
//
// Key features:
// - Precise measurement of recent traffic rates
// - Configurable time window to control measurement sensitivity
// - Equal weighting of all packets within the window
// - Automatic removal of outdated measurements
// - Immediate response to traffic pattern changes
//
// @see ~ExponentialRateMeter, ~TokenBucketMeter
//
simple SlidingWindowRateMeter extends PacketMeterBase like IPacketMeter
{
    parameters:
        double timeWindow @unit(s); // The time window over which to measure rates (only packets within this window from the current time are considered)
        @class(SlidingWindowRateMeter);
}
