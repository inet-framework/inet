//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.filter;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// This module limit the datarate or the packetrate of a stream of packets based
// on the ~RateTag attached to the packets.
//
// @see ~ExponentialRateMeter
//
simple StatisticalRateLimiter extends PacketFilterBase like IPacketFilter
{
    parameters:
        double maxDatarate @unit(bps) = default(nan);
        double maxPacketrate = default(nan);
        @class(StatisticalRateLimiter);
}
