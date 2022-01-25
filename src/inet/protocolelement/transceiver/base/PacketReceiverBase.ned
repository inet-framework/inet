//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

simple PacketReceiverBase extends PacketProcessorBase
{
    parameters:
        double datarate @unit(bps);
        @display("i=block/wrx");
        @signal[receptionStarted](type=inet::physicallayer::Signal);
        @signal[receptionEnded](type=inet::physicallayer::Signal);
// TODO        @statistic[receiving](title="transmitting"; type=int; source=count(receptionStarted) - count(receptionEnded); record=vector; interpolationmode=sample-hold);
        @statistic[throughput](title="throughput"; source=throughput(receptionEnded); record=vector,histogram; interpolationmode=linear);
// TODO        @statistic[utilization](title="utilization"; source=utilization(count(receptionStarted) - count(receptionEnded)); record=vector,histogram; interpolationmode=linear);
        @statistic[propagationTime](title="propagation time"; unit=s);
        @statistic[receptionTime](title="reception time"; unit=s; source=packetDuration(receptionEnded); record=vector,histogram; interpolationmode=none);
        @statistic[flowReceptionTime](title="flow reception time"; unit=s; source=packetDuration(demuxFlow(receptionEnded)); record=vector,histogram; interpolationmode=none);
    gates:
        input in;
        output out;
}

