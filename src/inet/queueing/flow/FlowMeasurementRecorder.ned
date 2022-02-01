//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.flow;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// This module is part of the flow measurement infrastructure. It can be used
// to record the collected statistics and also to terminate the flow measurements.
//
// The flow measurement statistics are collected for every bit individually.
// The measurement starts from the moment the bit enters the flow and continues
// up to the point where the bit exits the flow. The collected statistics can
// be recorded multiple times during this time. The flow measurement tracks
// bits individually using their unique identity.
//
// @see ~FlowMeasurementStarter
//
simple FlowMeasurementRecorder extends PacketFlowBase like IPacketFlow
{
    parameters:
        object packetFilter = default("*"); // which packets are considered, matches all packets by default
        int offset @unit(b) = default(0b); // designates the packet part to be measured
        int length @unit(b) = default(-1b); // designates the packet part to be measured
        string flowName = default(""); // use packet flow to separate measurement from other measurements
        string measure = default("*"); // match expression for a combination of: packetEvent, elapsedTime, delayingTime, queueingTime, processingTime, transmissionTime, propagationTime
        bool endMeasurement = default(true);
        @class(FlowMeasurementRecorder);
        @display("i=block/timer");
        @signal[packetFlowMeasured](type=inet::Packet);
        @signal[packetFlowEnded](type=inet::Packet);
        // the statistic value is the throughput of the measured flow
        @statistic[flowDataRate](title="flow data rate"; source=throughput(packetLength(demuxFlow(packetFlowMeasured))); record=vector; unit=bps; interpolationmode=linear);
        // the statistical value is the time difference of the current simulation time and the creation time of the bit
        @statistic[bitLifeTime](title="bit life time"; source=weightTimes(lengthWeightedValuePerRegion(lifeTimePerRegion(demuxFlow(packetFlowMeasured)))); record=histogram?; unit=s; interpolationmode=none);
        // the statistical value is the elapsed time from the moment the bit entered the flow
        @statistic[bitElapsedTime](title="bit elapsed time"; source=weightTimes(lengthWeightedValuePerRegion(elapsedTimePerRegion(demuxFlow(packetFlowMeasured)))); record=histogram?; unit=s; interpolationmode=none);
        // the statistical value is the sum of the delaying time of the bit for all delayers where the bit was delayed
        @statistic[totalBitDelayingTime](title="total bit delaying time"; source=weightTimes(lengthWeightedValuePerRegion(delayingTimePerRegion(demuxFlow(packetFlowMeasured)))); record=histogram?; unit=s; interpolationmode=none);
        // the statistical value is the sum of the processing time of the bit for all processors where the bit was processed
        @statistic[totalBitProcessingTime](title="total bit processing time"; source=weightTimes(lengthWeightedValuePerRegion(processingTimePerRegion(demuxFlow(packetFlowMeasured)))); record=histogram?; unit=s; interpolationmode=none);
        // the statistical value is the sum of the queueing time of the bit for all queues where the bit was enqueued
        @statistic[totalBitQueueingTime](title="total bit queueing time"; source=weightTimes(lengthWeightedValuePerRegion(queueingTimePerRegion(demuxFlow(packetFlowMeasured)))); record=histogram?; unit=s; interpolationmode=none);
        // the statistical value is the sum of the propagation time of the bit for all channels where the bit was propagated
        @statistic[totalBitPropagationTime](title="total bit propagation time"; source=weightTimes(lengthWeightedValuePerRegion(propagationTimePerRegion(demuxFlow(packetFlowMeasured)))); record=histogram?; unit=s; interpolationmode=none);
        // the statistical value is the sum of the transmission duration of the bit for all transmitters where the bit was transmitted
        @statistic[totalBitTransmissionTime](title="total bit transmission time"; source=weightTimes(lengthWeightedValuePerRegion(transmissionTimePerRegion(demuxFlow(packetFlowMeasured)))); record=histogram?; unit=s; interpolationmode=none);
        // the statistical value is the sum of the transmission duration of the complete packets for all transmitters where the bit was transmitted
        @statistic[totalPacketTransmissionTimePerBit](title="total packet transmission time per bit"; source=weightTimes(lengthWeightedValuePerRegion(packetTransmssionTimePerRegion(demuxFlow(packetFlowMeasured)))); record=histogram?; unit=s; interpolationmode=none);
        // the statistical value (the same for all bits in the region) is the time difference of the current simulation time and the creation time of any bit in the region
        @statistic[bitLifeTimePerRegion](title="bit life time per region"; source=dropWeight(lengthWeightedValuePerRegion(lifeTimePerRegion(demuxFlow(packetFlowMeasured)))); record=vector?,histogram?; unit=s; interpolationmode=none);
        // the statistical value (the same for all bits in the region) is the elapsed time from the moment any bit in the region entered the flow
        @statistic[bitElapsedTimePerRegion](title="bit elapsed time per region"; source=dropWeight(lengthWeightedValuePerRegion(elapsedTimePerRegion(demuxFlow(packetFlowMeasured)))); record=vector?,histogram?; unit=s; interpolationmode=none);
        // the statistical value (the same for all bits in the region) is the sum of the delaying time of any bit in the region for all delayers where that particular bit was delayed
        @statistic[totalBitDelayingTimePerRegion](title="total bit delaying time per region"; source=dropWeight(lengthWeightedValuePerRegion(delayingTimePerRegion(demuxFlow(packetFlowMeasured)))); record=vector?,histogram?; unit=s; interpolationmode=none);
        // the statistical value (the same for all bits in the region) is the sum of the processing time of any bit in the region for all processors where that particular bit was processed
        @statistic[totalBitProcessingTimePerRegion](title="total bit processing time per region"; source=dropWeight(lengthWeightedValuePerRegion(processingTimePerRegion(demuxFlow(packetFlowMeasured)))); record=vector?,histogram?; unit=s; interpolationmode=none);
        // the statistical value (the same for all bits in the region) is the sum of the queueing time of any bit in the region for all queues where that particular bit was enqueued
        @statistic[totalBitQueueingTimePerRegion](title="total bit queueing time per region"; source=dropWeight(lengthWeightedValuePerRegion(queueingTimePerRegion(demuxFlow(packetFlowMeasured)))); record=vector?,histogram?; unit=s; interpolationmode=none);
        // the statistical value (the same for all bits in the region) is the sum of the propagation time of any bit in the region for all channels where that particular bit was propagated
        @statistic[totalBitPropagationTimePerRegion](title="total bit propagation time per region"; source=dropWeight(lengthWeightedValuePerRegion(propagationTimePerRegion(demuxFlow(packetFlowMeasured)))); record=vector?,histogram?; unit=s; interpolationmode=none);
        // the statistical value (the same for all bits in the region) is the sum of the transmission duration of any bit in the region for all transmitters where that particular bit was transmitted
        @statistic[totalBitTransmissionTimePerRegion](title="total bit transmission time per region"; source=dropWeight(lengthWeightedValuePerRegion(transmissionTimePerRegion(demuxFlow(packetFlowMeasured)))); record=vector?,histogram?; unit=s; interpolationmode=none);
        // the statistical value (the same for all bits in the region) is the sum of the transmission duration of the complete packets for all transmitters where the particular bit of the region was transmitted
        @statistic[totalPacketTransmissionTimePerRegion](title="total packet transmission time per region"; source=dropWeight(lengthWeightedValuePerRegion(packetTransmissionTimePerRegion(demuxFlow(packetFlowMeasured)))); record=vector?,histogram?; unit=s; interpolationmode=none);
        // the statistical value is the time difference of the current simulation time and the creation time of any bit in the region
        @statistic[meanBitLifeTimePerPacket](title="mean bit life time per packet"; source=weightedMeanPerGroup(groupRegionsPerPacket(lifeTimePerRegion(demuxFlow(packetFlowMeasured)))); record=vector,histogram; unit=s; interpolationmode=none);
        // the statistical value is the elapsed time from the moment any bit in the region entered the flow
        @statistic[meanBitElapsedTimePerPacket](title="mean bit elapsed time per packet"; source=weightedMeanPerGroup(groupRegionsPerPacket(elapsedTimePerRegion(demuxFlow(packetFlowMeasured)))); record=vector,histogram; unit=s; interpolationmode=none);
        // the statistical value is the sum of the delaying time of any bit in the region for all delayers where that particular bit was delayed
        @statistic[meanTotalBitDelayingTimePerPacket](title="mean total bit delaying time per packet"; source=weightedMeanPerGroup(groupRegionsPerPacket(delayingTimePerRegion(demuxFlow(packetFlowMeasured)))); record=vector,histogram; unit=s; interpolationmode=none);
        // the statistical value is the sum of the processing time of any bit in the region for all processors where that particular bit was processed
        @statistic[meanTotalBitProcessingTimePerPacket](title="mean total bit processing time per packet"; source=weightedMeanPerGroup(groupRegionsPerPacket(processingTimePerRegion(demuxFlow(packetFlowMeasured)))); record=vector,histogram; unit=s; interpolationmode=none);
        // the statistical value is the sum of the queueing time of any bit in the region for all queues where that particular bit was enqueued
        @statistic[meanTotalBitQueueingTimePerPacket](title="mean total bit queueing time per packet"; source=weightedMeanPerGroup(groupRegionsPerPacket(queueingTimePerRegion(demuxFlow(packetFlowMeasured)))); record=vector,histogram; unit=s; interpolationmode=none);
        // the statistical value is the sum of the propagation time of any bit in the region for all channels where that particular bit was propagated
        @statistic[meanTotalBitPropagationTimePerPacket](title="mean total bit propagation time per packet"; source=weightedMeanPerGroup(groupRegionsPerPacket(propagationTimePerRegion(demuxFlow(packetFlowMeasured)))); record=vector,histogram; unit=s; interpolationmode=none);
        // the statistical value is the sum of the transmission duration of any bit in the region for all transmitters where that particular bit was transmitted
        @statistic[meanTotalBitTransmissionTimePerPacket](title="mean total bit transmission time per packet"; source=weightedMeanPerGroup(groupRegionsPerPacket(transmissionTimePerRegion(demuxFlow(packetFlowMeasured)))); record=vector,histogram; unit=s; interpolationmode=none);
        // the statistical value is the sum of the transmission duration of the complete packets for all transmitters where the particular bit of the region was transmitted
        @statistic[meanTotalPacketTransmissionTime](title="mean total packet transmission time"; source=weightedMeanPerGroup(groupRegionsPerPacket(packetTransmissionTimePerRegion(demuxFlow(packetFlowMeasured)))); record=vector,histogram; unit=s; interpolationmode=none);
        // the statistical value is the difference of subsequent values of the mean bit elapsed time per packet
        @statistic[packetJitter](title="packet jitter"; source=jitter(weightedMeanPerGroup(groupRegionsPerPacket(elapsedTimePerRegion(demuxFlow(packetFlowMeasured))))); record=vector,histogram; unit=s; interpolationmode=none);
        // the statistical value is the difference of the current and the mean bit elapsed time per packet
        @statistic[packetDelayDifferenceToMean](title="packet delay difference to mean"; source=differenceToMean(weightedMeanPerGroup(groupRegionsPerPacket(elapsedTimePerRegion(demuxFlow(packetFlowMeasured))))); record=vector,histogram; unit=s; interpolationmode=none);
        // the statistical value is the variation of the mean bit elapsed time per packet
        @statistic[packetDelayVariation](title="packet delay variation"; source=stddev(weightedMeanPerGroup(groupRegionsPerPacket(elapsedTimePerRegion(demuxFlow(packetFlowMeasured))))); record=vector,histogram; unit=s);
}
