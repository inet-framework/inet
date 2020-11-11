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
        @signal[lifeTime](type=simtime_t);
        @signal[elapsedTime](type=simtime_t);
        @signal[delayingTime](type=simtime_t);
        @signal[queueingTime](type=simtime_t);
        @signal[processingTime](type=simtime_t);
        @signal[transmissionTime](type=simtime_t);
        @signal[propagationTime](type=simtime_t);
        @statistic[lifeTime](title="Life time"; source=demux(lifeTime); record=vector,histogram; interpolationmode=none);
        @statistic[elapsedTime](title="Elapsed time"; source=demux(elapsedTime); record=vector,histogram; interpolationmode=none);
        @statistic[delayingTime](title="Delaying time"; source=demux(delayingTime); record=vector,histogram; interpolationmode=none);
        @statistic[queueingTime](title="Queueing time"; source=demux(queueingTime); record=vector,histogram; interpolationmode=none);
        @statistic[processingTime](title="Processing time"; source=demux(processingTime); record=vector,histogram; interpolationmode=none);
        @statistic[transmissionTime](title="Transmission time"; source=demux(transmissionTime); record=vector,histogram; interpolationmode=none);
        @statistic[propagationTime](title="Propagation time"; source=demux(propagationTime); record=vector,histogram; interpolationmode=none);
}
