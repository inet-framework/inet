//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.meter;

import inet.queueing.base.PacketMeterBase;
import inet.queueing.contract.IPacketMeter;

//
// This module attaches a ~RateTag to each packet specifying the datarate and
// packetrate of the received stream of packets. The algorithm takes older
// packets into account with an exponentially decreasing weight.
//
// @see ~StatisticalRateLimiter
//
simple ExponentialRateMeter extends PacketMeterBase like IPacketMeter
{
    parameters:
        double alpha; // parameter in the range of (0, 1) for the exponential weight decrease
        @class(ExponentialRateMeter);
}
