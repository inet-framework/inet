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
simple ActivePacketSourceBase extends PacketSourceBase
{
    parameters:
        @class(ActivePacketSourceBase);
        @signal[packetPushed](type=inet::Packet);
        // the statistical value is the packet
        @statistic[packets](title="packets"; source=packetPushed; record=count; unit=pk);
        // the statistical value is the length of the packet
        @statistic[packetLengths](title="packet lengths"; source=packetLength(packetPushed); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the data rate of the packets
        @statistic[dataRate](title="data rate"; source=throughput(packetPushed); record=vector; unit=bps; interpolationmode=linear);
}
