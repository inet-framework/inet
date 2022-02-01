//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// This is a base module for various packet sink modules.
//
// @see ~IPacketSink
//
simple PacketSinkBase extends PacketProcessorBase
{
    parameters:
        displayStringTextFormat = default("received %p pk (%l)");
        @class(PacketSinkBase);
        @display("i=block/sink");
        @signal[packetDropped](type=inet::Packet);
        // the statistical value is the dropped packet
        @statistic[droppedPackets](title="dropped packets"; source=packetDropped; record=count; unit=pk);
        // the statistical value is the length of the dropped packet
        @statistic[droppedPacketLengths](title="dropped packet lengths"; source=packetLength(packetDropped); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the data rate of the dropped packets
        @statistic[droppedDataRate](title="dropped data rate"; source=throughput(packetDropped); record=vector; unit=bps; interpolationmode=linear);
}
