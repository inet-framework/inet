//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for active packet sink components in the queueing framework.
// Extends PacketSinkBase with functionality specific to active packet collection.
//
// Active packet sinks initiate packet flow by actively pulling packets from
// connected passive packet sources. This module provides the foundation for
// implementing various active packet sink behaviors with different collection
// strategies and timing mechanisms.
//
// Key features:
// - Implements the IActivePacketSink interface
// - Provides signals and statistics for pulled packets
// - Collects detailed metrics on packet processing, including:
//   * Packet counts and lengths
//   * Data rates
//   * Packet lifetime statistics
//   * Jitter and delay variation measurements
//
// Derived modules can implement specific collection strategies by defining
// when and how packets are pulled from sources.
//
// @see ~PacketSinkBase, ~IActivePacketSink
//
simple ActivePacketSinkBase extends PacketSinkBase
{
    parameters:
        @class(ActivePacketSinkBase);
        @signal[packetPulled](type=inet::Packet);
        // the statistical value is the packet
        @statistic[packets](title="packets"; source=packetPulled; record=count; unit=pk);
        // the statistical value is the length of the packet
        @statistic[packetLengths](title="packet lengths"; source=packetLength(packetPulled); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the data rate of the packets
        @statistic[dataRate](title="data rate"; source=throughput(packetPulled); record=vector; unit=bps; interpolationmode=linear);
        // the statistical value is the time difference of the current simulation time and the creation time of the bit
        @statistic[bitLifeTime](title="bit life time"; source=weightTimes(lengthWeightedValuePerRegion(lifeTimePerRegion(packetPulled))); record=histogram?; unit=s; interpolationmode=none);
        // the statistical value is the time difference of the current simulation time and the creation time of any bit in the region
        @statistic[meanBitLifeTimePerPacket](title="mean bit life time per packet"; source=weightedMeanPerGroup(groupRegionsPerPacket(lifeTimePerRegion(packetPulled))); record=vector,histogram; unit=s; interpolationmode=none);
        // the statistical value is the difference of subsequent values of the mean bit elapsed time per packet
        @statistic[packetJitter](title="packet jitter"; source=jitter(weightedMeanPerGroup(groupRegionsPerPacket(lifeTimePerRegion(packetPulled)))); record=vector,histogram; unit=s; interpolationmode=none);
        // the statistical value is the difference of the current and the mean bit elapsed time per packet
        @statistic[packetDelayDifferenceToMean](title="packet delay difference to mean"; source=differenceToMean(weightedMeanPerGroup(groupRegionsPerPacket(lifeTimePerRegion(packetPulled)))); record=vector,histogram; unit=s; interpolationmode=none);
        // the statistical value is the variation of the mean bit elapsed time per packet
        @statistic[packetDelayVariation](title="packet delay variation"; source=stddev(weightedMeanPerGroup(groupRegionsPerPacket(lifeTimePerRegion(packetPulled)))); record=vector,histogram; unit=s);
}
