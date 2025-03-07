//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for packet sink components in the queueing framework.
// Packet sinks are terminal points in packet flow paths that consume
// and destroy packets, representing endpoints of communication.
//
// Packet sinks serve several important functions in network simulations:
// - Representing application endpoints that consume packets
// - Modeling packet drop points in the network
// - Collecting statistics on received or dropped packets
// - Terminating packet flows in simulation models
//
// This module provides comprehensive statistics collection for:
// - Dropped packet counts and lengths
// - Data rates of dropped packets
//
// Derived modules can implement specific sink behaviors by defining
// how packets are processed upon arrival and when they should be dropped.
//
// @see ~IPacketSink, ~ActivePacketSink, ~PassivePacketSink
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
