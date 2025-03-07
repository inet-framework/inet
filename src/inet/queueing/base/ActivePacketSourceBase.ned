//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for active packet source components in the queueing framework.
// Extends PacketSourceBase with functionality specific to active packet generation.
//
// Active packet sources initiate packet flow by actively pushing packets to
// connected passive packet sinks. This module provides the foundation for
// implementing various active packet source behaviors with different generation
// strategies and timing mechanisms.
//
// Key features:
// - Implements the IActivePacketSource interface
// - Provides signals and statistics for pushed packets
// - Collects detailed metrics on packet generation, including:
//   * Packet counts and lengths
//   * Data rates
//
// Derived modules can implement specific generation strategies by defining
// when and how packets are created and pushed to sinks. Common implementations
// include periodic generators, bursty traffic sources, and trace-driven sources.
//
// @see ~PacketSourceBase, ~IActivePacketSource, ~IPassivePacketSink
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
