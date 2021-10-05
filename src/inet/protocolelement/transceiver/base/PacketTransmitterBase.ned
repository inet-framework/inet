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

package inet.protocolelement.transceiver.base;

import inet.queueing.base.PacketProcessorBase;

simple PacketTransmitterBase extends PacketProcessorBase
{
    parameters:
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        volatile double datarate @unit(bps);
        @class(PacketTransmitterBase);
        @display("i=block/wtx");
        @signal[transmissionStarted](type=inet::physicallayer::Signal);
        @signal[transmissionEnded](type=inet::physicallayer::Signal);
        @statistic[transmitting](title="transmitting"; type=int; source=count(transmissionStarted) - count(transmissionEnded); record=vector; interpolationmode=sample-hold);
        @statistic[throughput](title="throughput"; source=throughput(transmissionEnded); record=vector,histogram; interpolationmode=linear);
        @statistic[utilization](title="utilization"; source=utilization(count(transmissionStarted) - count(transmissionEnded)); record=vector,histogram; interpolationmode=linear);
        @statistic[propagationTime](title="propagation time"; unit=s);
        @statistic[transmissionTime](title="transmission time"; unit=s; source=packetDuration(transmissionEnded); record=vector,histogram; interpolationmode=none);
        @statistic[flowTransmissionTime](title="flow transmission time"; unit=s; source=packetDuration(demuxFlow(transmissionEnded)); record=vector,histogram; interpolationmode=none);
    gates:
        input in;
        output out;
}
