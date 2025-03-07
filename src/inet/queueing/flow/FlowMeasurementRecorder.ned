//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.flow;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Records detailed packet flow statistics and optionally terminates flow measurements
// that were previously started by a FlowMeasurementStarter module. Provides comprehensive
// bit-level tracking of packets as they move through the network.
//
// Flow measurement capabilities:
// - Tracks individual bits through their entire journey in the network
// - Records multiple timing statistics for each bit (queueing, processing, transmission, etc.)
// - Calculates aggregate statistics per packet, per region, and across the entire flow
// - Supports filtering to measure specific packet types or parts of packets
// - Can terminate measurements or allow them to continue through the network
//
// Key features:
// - Bit-level granularity for precise measurements
// - Comprehensive timing statistics collection
// - Flexible packet filtering
// - Optional measurement termination
// - JSON output for detailed analysis
// - Extensive statistical recording options
//
// Applications:
// - Performance analysis of network protocols
// - Bottleneck identification in complex networks
// - End-to-end delay measurement and jitter analysis
// - Queue behavior and processing time studies
// - Protocol optimization based on detailed timing data
// - Validation of QoS mechanisms
//
// The recorder collects statistics for various timing components:
// - Elapsed time: Time since a bit entered the flow
// - Delaying time: Time spent in delayers
// - Queueing time: Time spent in queues
// - Processing time: Time spent in processors
// - Transmission time: Time spent in transmitters
// - Propagation time: Time spent in channels
//
// Statistics are collected at multiple levels:
// - Per bit: Individual timing for each bit
// - Per region: Timing for groups of bits within a packet
// - Per packet: Aggregate timing across all bits in a packet
//
// Advanced metrics include:
// - Flow data rate (throughput)
// - Packet jitter (variation in delay between packets)
// - Delay variation (standard deviation of delays)
//
// @see ~FlowMeasurementStarter
//
simple FlowMeasurementRecorder extends PacketFlowBase like IPacketFlow
{
    parameters:
        object packetFilter = default("*"); // Which packets are considered, matches all packets by default
        int offset @unit(b) = default(0b); // Designates the packet part to be measured
        int length @unit(b) = default(-1b); // Designates the packet part to be measured
        string flowName = default(""); // Use packet flow to separate measurement from other measurements
        string measure = default("not packetEvent"); // Match expression for a combination of: packetEvent, elapsedTime, delayingTime, queueingTime, processingTime, transmissionTime, propagationTime
        bool endMeasurement = default(true); // Whether to terminate the flow measurement at this point
        string packetEventFileName = default("results/" + expand("${configname}-") + fullPath() + "-packetEvent.json"); // File to record packet events
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
