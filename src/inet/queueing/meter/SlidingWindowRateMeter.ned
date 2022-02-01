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
// packetrate of the received stream of packets. The algorithm takes packets
// into account only in the given time window.
//
simple SlidingWindowRateMeter extends PacketMeterBase like IPacketMeter
{
    parameters:
        double timeWindow @unit(s); // amount of time to look back from the current simulation time
        @class(SlidingWindowRateMeter);
}
