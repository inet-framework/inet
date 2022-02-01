//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// This is a base module for various active packet sink modules.
//
// @see ~PacketSinkBase
//
simple PassivePacketSinkBase extends PacketSinkBase
{
    parameters:
        @class(PassivePacketSinkBase);
        @signal[packetPushed](type=inet::Packet);
        // the statistical value is the packet
        @statistic[packets](title="packets"; source=packetPushed; record=count; unit=pk);
        // the statistical value is the length of the packet
        @statistic[packetLengths](title="packet lengths"; source=packetLength(packetPushed); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the data rate of the packets
        @statistic[dataRate](title="data rate"; source=throughput(packetPushed); record=vector; unit=bps; interpolationmode=linear);
        // the statistical value is the time difference of the current simulation time and the creation time of the bit
        @statistic[bitLifeTime](title="bit life time"; source=weightTimes(lengthWeightedValuePerRegion(lifeTimePerRegion(packetPushed))); record=histogram?; unit=s; interpolationmode=none);
        // the statistical value is the time difference of the current simulation time and the creation time of any bit in the region
        @statistic[meanBitLifeTimePerPacket](title="mean bit life time per packet"; source=weightedMeanPerGroup(groupRegionsPerPacket(lifeTimePerRegion(packetPushed))); record=vector,histogram; unit=s; interpolationmode=none);
        // the statistical value is the difference of subsequent values of the mean bit elapsed time per packet
        @statistic[packetJitter](title="packet jitter"; source=jitter(weightedMeanPerGroup(groupRegionsPerPacket(lifeTimePerRegion(packetPushed)))); record=vector,histogram; unit=s; interpolationmode=none);
        // the statistical value is the difference of the current and the mean bit elapsed time per packet
        @statistic[packetDelayDifferenceToMean](title="packet delay difference to mean"; source=differenceToMean(weightedMeanPerGroup(groupRegionsPerPacket(lifeTimePerRegion(packetPushed)))); record=vector,histogram; unit=s; interpolationmode=none);
        // the statistical value is the variation of the mean bit elapsed time per packet
        @statistic[packetDelayVariation](title="packet delay variation"; source=stddev(weightedMeanPerGroup(groupRegionsPerPacket(lifeTimePerRegion(packetPushed)))); record=vector,histogram; unit=s);
}
