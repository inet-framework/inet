//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
        string packetFilter = default("*"); // which packets are considered, matches all packets by default
        string packetDataFilter = default("*"); // which packets are considered based on the data they contain, matches all packets by default
        int offset @unit(b) = default(0b); // designates the packet part to be measured
        int length @unit(b) = default(-1b); // designates the packet part to be measured
        string flowName = default(""); // start packet flow to separate measurement from other measurements
        string measure = default("*"); // match expression for a combination of: packetEvent, elapsedTime, delayingTime, queueingTime, processingTime, transmissionTime, propagationTime
        bool endMeasurement = default(true);
        @class(FlowMeasurementRecorder);
        @display("i=block/timer");
        @signal[packetFlowEnded](type=inet::Packet);

        // TODO: merge these signals into one and move code from module into result filters
        @signal[bitLifeTime](type=simtime_t);
        @signal[bitElapsedTime](type=simtime_t);
        @signal[totalBitDelayingTime](type=simtime_t);
        @signal[totalBitQueueingTime](type=simtime_t);
        @signal[totalBitProcessingTime](type=simtime_t);
        @signal[totalBitTransmissionTime](type=simtime_t);
        @signal[totalBitPropagationTime](type=simtime_t);
        @signal[totalPacketProcessingTimePerBit](type=simtime_t);
        @signal[totalPacketTransmissionTimePerBit](type=simtime_t);

        @signal[bitLifeTimePerRegion](type=simtime_t);
        @signal[bitElapsedTimePerRegion](type=simtime_t);
        @signal[totalBitDelayingTimePerRegion](type=simtime_t);
        @signal[totalBitQueueingTimePerRegion](type=simtime_t);
        @signal[totalBitProcessingTimePerRegion](type=simtime_t);
        @signal[totalBitTransmissionTimePerRegion](type=simtime_t);
        @signal[totalBitPropagationTimePerRegion](type=simtime_t);
        @signal[totalPacketProcessingTimePerRegion](type=simtime_t);
        @signal[totalPacketTransmissionTimePerRegion](type=simtime_t);

//
//		Example table for transmission time statistics (applicable to the other statistics as well):
//        
//        +==========================================+===================================+=============================+
//        | Recording granularity \ What is totalled |       Bit transmission time       |  Packet transmission time   |
//        +==========================================+===================================+=============================+
//        | Statistic value per bit                  | totalBitTransmissionTime          |                             |
//        +------------------------------------------+-----------------------------------+-----------------------------+
//        | Statistic value per region               | totalBitTransmissionTimePerRegion | totalPacketTransmissionTime |
//        +------------------------------------------+-----------------------------------+-----------------------------+
//
//
//
//		Statistics recorded for every bit
//

        // the statistics value is the time difference of the current simulation time and the creation time of the bit
        @statistic[bitLifeTime](title="Total bit life time"; source=demux(bitLifeTime); record=histogram);
        // the statistics value is the elapsed time from the moment the bit entered the flow
        @statistic[bitElapsedTime](title="Total bit elapsed time"; source=demux(bitElapsedTime); record=histogram);
        // the statistics value is the sum of the delaying time of the bit for all delayers where the bit was delayed
        @statistic[totalBitDelayingTime](title="Total bit delaying time"; source=demux(totalBitDelayingTime); record=histogram);
        // the statistics value is the sum of the queueing time of the bit for all queues where the bit was enqueued
        @statistic[totalBitQueueingTime](title="Total bit queueing time"; source=demux(totalBitQueueingTime); record=histogram);
        // the statistics value is the sum of the processing time of the bit for all processors where the bit was processed
        @statistic[totalBitProcessingTime](title="Total bit processing time"; source=demux(totalBitProcessingTime); record=histogram);
        // the statistics value is the sum of the transmission duration of the bit for all transmitters where the bit was transmitted
        @statistic[totalBitTransmissionTime](title="Total bit transmission time"; source=demux(totalBitTransmissionTime); record=histogram);
        // the statistics value is the sum of the propagation time of the bit for all channels where the bit was propagated
        @statistic[totalBitPropagationTime](title="Total bit propagation time"; source=demux(totalBitPropagationTime); record=histogram);
        // the statistics value is the sum of the transmission duration of the complete packets for all transmitters where the bit was transmitted
        @statistic[totalPacketTransmissionTimePerBit](title="Total packet transmission time per bit"; source=demux(totalPacketTransmissionTimePerBit); record=histogram; interpolationmode=none);
        
//        
//        Statistics recorded for every region
//        

        // the statistics value (the same for all bits in the region) is the time difference of the current simulation time and the creation time of any bit in the region
        @statistic[bitLifeTimePerRegion](title="Total bit life time per region"; source=demux(bitLifeTimePerRegion); record=vector,histogram; interpolationmode=none);
        // the statistics value (the same for all bits in the region) is the elapsed time from the moment any bit in the region entered the flow
        @statistic[bitElapsedTimePerRegion](title="Total bit elapsed time per region"; source=demux(bitElapsedTimePerRegion); record=vector,histogram; interpolationmode=none);
        // the statistics value (the same for all bits in the region) is the sum of the delaying time of any bit in the region for all delayers where that particular bit was delayed
        @statistic[totalBitDelayingTimePerRegion](title="Total bit delaying time per region"; source=demux(totalBitDelayingTimePerRegion); record=vector,histogram; interpolationmode=none);
        // the statistics value (the same for all bits in the region) is the sum of the queueing time of any bit in the region for all queues where that particular bit was enqueued
        @statistic[totalBitQueueingTimePerRegion](title="Total bit queueing time per region"; source=demux(totalBitQueueingTimePerRegion); record=vector,histogram; interpolationmode=none);
        // the statistics value (the same for all bits in the region) is the sum of the processing time of any bit in the region for all processors where that particular bit was processed
        @statistic[totalBitProcessingTimePerRegion](title="Total bit processing time per region"; source=demux(totalBitProcessingTimePerRegion); record=vector,histogram; interpolationmode=none);
        // the statistics value (the same for all bits in the region) is the sum of the transmission duration of any bit in the region for all transmitters where that particular bit was transmitted
        @statistic[totalBitTransmissionTimePerRegion](title="Total bit transmission time per region"; source=demux(totalBitTransmissionTimePerRegion); record=vector,histogram; interpolationmode=none);
        // the statistics value (the same for all bits in the region) is the sum of the propagation time of any bit in the region for all channels where that particular bit was propagated
        @statistic[totalBitPropagationTimePerRegion](title="Total bit propagation time per region"; source=demux(totalBitPropagationTimePerRegion); record=vector,histogram; interpolationmode=none);
        // the statistics value (the same for all bits in the region) is the sum of the transmission duration of the complete packets for all transmitters where the particular bit of the region was transmitted
        @statistic[totalPacketTransmissionTimePerRegion](title="Total packet transmission time per region"; source=demux(totalPacketTransmissionTimePerRegion); record=vector,histogram; interpolationmode=none);
}
