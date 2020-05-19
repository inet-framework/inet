//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

package inet.queueing.timing;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

simple TimingMeasurementStarter extends PacketFlowBase like IPacketFlow
{
    parameters:
        string packetFilter = default("*"); // which packets are considered, matches all packets by default
        string packetDataFilter = default("*"); // which packets are considered based on the data they contain, matches all packets by default
        int offset @unit(b) = default(0b); // designates the packet part to be measured
        int length @unit(b) = default(-1b); // designates the packet part to be measured
        string flowName = default(""); // use packet flow to separate measurement from other measurements
        string measure = default("*"); // match expression for a combination of: elapsedTime, delayingTime, queueingTime, processingTime, transmissionTime, propagationTime
        @class(TimingMeasurementStarter);
        @display("i=block/timer");
        @signal[packetFlowStarted](type=inet::Packet);
}
