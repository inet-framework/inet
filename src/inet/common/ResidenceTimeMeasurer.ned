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

package inet.common;

//
// This module measures the residence time of packet data in network nodes. The
// measurement is done by tracking every bit individually using their unique
// identity. For each bit the measurement starts when the incoming enclosing
// packet reception ends (or starts) in the network node. Similarly, for each
// bit the measurement ends when the outgoing enclosing packet transmission
// starts (or ends) in the network node.
//
simple ResidenceTimeMeasurer like IMeasurer
{
    parameters:
        string measurementStartSignal = default("receptionEnded");
        string measurementEndSignal = default("transmissionStarted");
        @signal[packetStayed](type=inet::Packet);
        // the statistics value is the maximum residence time for any bit in the outgoing packet
        @statistic[maxBitResidenceTimePerPacket](title="maximum bit residence time per packet"; source=maxPerGroup(groupRegionsPerPacket(residenceTimePerRegion(packetStayed))); record=vector?,histogram?; unit=s; interpolationmode=none);
        // the statistics value is the mean residence time for all bits in the outgoing packet
        @statistic[meanBitResidenceTimePerPacket](title="mean bit residence time per packet"; source=weightedMeanPerGroup(groupRegionsPerPacket(residenceTimePerRegion(packetStayed))); record=vector,histogram; unit=s; interpolationmode=none);
        // the statistics value (the same for all bits in the region) is the residence time of any bit in the region
        @statistic[bitResidenceTimePerRegion](title="bit residence time per region"; source=dropWeight(lengthWeightedValuePerRegion(residenceTimePerRegion(packetStayed))); record=vector?,histogram?; unit=s; interpolationmode=none);
        // the statistics value is the residence time for individual bits
        @statistic[bitResidenceTime](title="bit residence time"; source=weightTimes(lengthWeightedValuePerRegion(residenceTimePerRegion(packetStayed))); record=vector?,histogram?; unit=s; interpolationmode=none);
        // the statistics value is the flow specific maximum residence time for any bit in the outgoing packet
        @statistic[flowMaxBitResidenceTimePerPacket](title="flow maximum bit residence time per packet"; source=maxPerGroup(groupRegionsPerPacket(residenceTimePerRegion(demuxFlow(packetStayed)))); record=vector?,histogram?; unit=s; interpolationmode=none);
        // the statistics value is the flow specific mean residence time for all bits in the outgoing packet
        @statistic[flowMeanBitResidenceTimePerPacket](title="flow mean bit residence time per packet"; source=weightedMeanPerGroup(groupRegionsPerPacket(residenceTimePerRegion(demuxFlow(packetStayed)))); record=vector,histogram; unit=s; interpolationmode=none);
        // the statistics value (the same for all bits in the region) is the flow specific residence time of any bit in the region
        @statistic[flowBitResidenceTimePerRegion](title="flow bit residence time per region"; source=dropWeight(lengthWeightedValuePerRegion(residenceTimePerRegion(demuxFlow(packetStayed)))); record=vector?,histogram?; unit=s; interpolationmode=none);
        // the statistics value is the flow specific residence time for individual bits
        @statistic[flowBitResidenceTime](title="flow bit residence time"; source=weightTimes(lengthWeightedValuePerRegion(residenceTimePerRegion(demuxFlow(packetStayed)))); record=vector?,histogram?; unit=s; interpolationmode=none);
}
