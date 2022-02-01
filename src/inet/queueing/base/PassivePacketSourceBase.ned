//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// This is a base module for various active packet source modules.
//
// @see ~PacketSourceBase
//
simple PassivePacketSourceBase extends PacketSourceBase
{
    parameters:
        @class(PassivePacketSourceBase);
        @signal[packetPulled](type=inet::Packet);
        // the statistical value is the packet
        @statistic[packets](title="packets"; source=packetPulled; record=count; unit=pk);
        // the statistical value is the length of the packet
        @statistic[packetLengths](title="packet lengths"; source=packetLength(packetPulled); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the data rate of the packets
        @statistic[dataRate](title="data rate"; source=throughput(packetPulled); record=vector; unit=bps; interpolationmode=linear);
}
