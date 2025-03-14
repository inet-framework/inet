//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.meter;

import inet.queueing.base.PacketMeterBase;
import inet.queueing.contract.IPacketMeter;

//
// This module implements an Exponentially Weighted Moving Average (EWMA) rate meter
// that measures the data rate and packet rate of a packet stream. It attaches a
// ~RateTag to each packet with the calculated rates.
//
// The meter uses an exponential smoothing algorithm that gives more weight to
// recent observations while still taking older observations into account with
// exponentially decreasing weights. This approach provides a responsive yet
// stable measurement of traffic rates that adapts to changing network conditions.
//
// The calculation works as follows:
// - For each new packet, the instantaneous rate is calculated
// - The smoothed rate is updated using:
//   smoothedRate = alpha * instantaneousRate + (1 - alpha) * previousSmoothedRate
// - Where alpha is a smoothing factor between 0 and 1
//
// Key features:
// - Provides both data rate (bits/sec) and packet rate (packets/sec) measurements
// - Adapts quickly to traffic changes while filtering out short-term fluctuations
// - Configurable alpha parameter to control the balance between responsiveness and stability
// - Lower alpha values (closer to 0) give more weight to history (more stable, less responsive)
// - Higher alpha values (closer to 1) give more weight to recent packets (more responsive, less stable)
//
// @see ~StatisticalRateLimiter, ~SlidingWindowRateMeter
//
simple ExponentialRateMeter extends PacketMeterBase like IPacketMeter
{
    parameters:
        double alpha; // Smoothing factor in the range (0, 1) that determines how quickly the EWMA responds to rate changes
                      // - Values closer to 0 give more weight to historical data (more stable)
                      // - Values closer to 1 give more weight to recent data (more responsive)
        @class(ExponentialRateMeter);
}
